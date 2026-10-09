#!/usr/bin/env python3
#
# CDE benchmarks: time an X application from launch to first window and
# to idle.
#
# Licensed under the LGPL 2.1 license.
#
"""Time an X application from launch to its first window and to idle.

  apptime.py [options] --name REGEX -- COMMAND [ARG...]

The command runs on $DISPLAY (or on a private Xvfb with --xvfb), with
libcdebench_preload.so preloaded so that its malloc, X request and round
trip totals are reported when it exits.  Reported, as one JSON object:

  map_s      launch until a visible window whose name matches REGEX exists
             (xdotool search --onlyvisible --name)
  idle_s     launch until the process last used CPU before staying idle
             for --idle-ms (default 1000) milliseconds: "all done"
  cpu_s      CPU time of the process (user + system) until then
  mallocs, requests, round_trips   the preload's totals at exit
                                   (when the process exits normally)

With --ttsession a private "ttsession -s -S -d $DISPLAY" runs for the
duration, so that ToolTalk clients (dtfile, dtmail) find a session
instead of auto-starting one inside the measured time.  The process is
stopped with SIGTERM (then SIGKILL) once idle, or --close-key KEYS is
sent to its window with xdotool first (for programs that only exit
cleanly from their UI).
"""

import argparse
import json
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
CLK_TCK = os.sysconf("SC_CLK_TCK")


def log(msg):
    print("apptime: " + msg, file=sys.stderr, flush=True)


def proc_cpu(pid):
    """CPU seconds (user + system) of pid and its live children."""
    total = 0.0
    try:
        pids = [pid] + [int(p) for p in os.listdir("/proc") if p.isdigit()
                        and ppid_of(int(p)) == pid]
    except OSError:
        pids = [pid]
    for p in pids:
        try:
            with open("/proc/%d/stat" % p) as f:
                fields = f.read().rsplit(")", 1)[1].split()
            total += (int(fields[11]) + int(fields[12])) / CLK_TCK
        except (OSError, IndexError, ValueError):
            pass
    return total


def ppid_of(pid):
    try:
        with open("/proc/%d/stat" % pid) as f:
            return int(f.read().rsplit(")", 1)[1].split()[1])
    except (OSError, IndexError, ValueError):
        return -1


def find_window(regex):
    try:
        out = subprocess.run(["xdotool", "search", "--onlyvisible", "--name",
                              regex], capture_output=True, text=True,
                             timeout=10).stdout.split()
    except subprocess.TimeoutExpired:
        return None
    return out[0] if out else None


def start_xvfb():
    rd, wr = os.pipe()
    proc = subprocess.Popen(["Xvfb", "-displayfd", str(wr), "-screen", "0",
                             "1920x1080x24", "+extension", "RENDER",
                             "-nolisten", "tcp", "-noreset"],
                            pass_fds=(wr,), stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    os.close(wr)
    number = b""
    while not number.endswith(b"\n"):
        chunk = os.read(rd, 16)
        if not chunk:
            break
        number += chunk
    os.close(rd)
    if not number.strip().isdigit():
        proc.kill()
        raise SystemExit("apptime: Xvfb did not start")
    os.environ["DISPLAY"] = ":" + number.decode().strip()
    return proc


def read_counters(path, comm):
    res = {}
    try:
        with open(path) as f:
            for line in f:
                kv = dict(t.split("=", 1) for t in line.split()[1:]
                          if "=" in t)
                if comm is None or kv.get("comm", "").startswith(comm[:15]):
                    res = {"mallocs": int(kv["mallocs"]),
                           "requests": int(kv["requests"]),
                           "round_trips": int(kv["rtrips"])}
    except (OSError, KeyError, ValueError):
        pass
    return res


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--name", required=True,
                    help="regex for the window name (xdotool --name)")
    ap.add_argument("--label", help="name of the result")
    ap.add_argument("--idle-ms", type=int, default=1000)
    ap.add_argument("--timeout", type=float, default=300)
    ap.add_argument("--xvfb", action="store_true",
                    help="run on a private Xvfb")
    ap.add_argument("--ttsession", action="store_true",
                    help="run a private ttsession for the duration")
    ap.add_argument("--close-key", help="xdotool key(s) to close the window")
    ap.add_argument("--no-preload", action="store_true")
    ap.add_argument("command", nargs=argparse.REMAINDER)
    args = ap.parse_args()
    cmd = args.command[1:] if args.command[:1] == ["--"] else args.command
    if not cmd:
        ap.error("no command")
    if not shutil.which("xdotool"):
        raise SystemExit("apptime: needs xdotool")

    helpers = []
    work = tempfile.mkdtemp(prefix="apptime.")
    counters = os.path.join(work, "counters")
    try:
        if args.xvfb or not os.environ.get("DISPLAY"):
            helpers.append(start_xvfb())
        env = dict(os.environ)
        if args.ttsession:
            tts = shutil.which("ttsession", path=os.path.join(
                os.environ.get("CDE_TOP", os.path.join(HERE, "..", "..")),
                "lib", "tt", "bin", "ttsession", ".libs") + os.pathsep +
                os.environ.get("PATH", ""))
            helpers.append(subprocess.Popen(
                [tts, "-s", "-S", "-d", os.environ["DISPLAY"]],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
            time.sleep(0.5)
        if not args.no_preload:
            env["LD_PRELOAD"] = os.path.join(HERE, "libcdebench_preload.so")
            env["CDEBENCH_REPORT"] = counters

        t0 = time.monotonic()
        proc = subprocess.Popen(cmd, env=env, stdin=subprocess.DEVNULL,
                                stdout=open(os.path.join(work, "log"), "w"),
                                stderr=subprocess.STDOUT)
        deadline = t0 + args.timeout
        map_s = None
        last_cpu, last_change = 0.0, t0
        while True:
            now = time.monotonic()
            if now > deadline:
                log("timed out")
                break
            if proc.poll() is not None:
                log("the command exited (status %d) before going idle"
                    % proc.returncode)
                break
            if map_s is None and find_window(args.name):
                map_s = time.monotonic() - t0
            cpu = proc_cpu(proc.pid)
            if cpu != last_cpu:
                last_cpu, last_change = cpu, now
            elif map_s is not None and \
                    now - last_change >= args.idle_ms / 1000.0:
                break
            time.sleep(0.02)
        idle_s = last_change - t0
        if map_s is not None and idle_s < map_s:
            idle_s = map_s
        win = find_window(args.name)
        if args.close_key and win and proc.poll() is None:
            subprocess.run(["xdotool", "key", "--window", win] +
                           args.close_key.split(), timeout=10)
            try:
                proc.wait(10)
            except subprocess.TimeoutExpired:
                pass
        if proc.poll() is None:
            proc.send_signal(signal.SIGTERM)
            try:
                proc.wait(10)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
        res = {"bench": "apptime",
               "name": args.label or os.path.basename(cmd[0]),
               "map_s": round(map_s, 4) if map_s is not None else None,
               "idle_s": round(idle_s, 4), "cpu_s": round(last_cpu, 3)}
        res.update(read_counters(counters, os.path.basename(cmd[0])))
        print(json.dumps(res), flush=True)
        return 0 if map_s is not None else 1
    finally:
        for h in reversed(helpers):
            h.terminate()
            try:
                h.wait(10)
            except subprocess.TimeoutExpired:
                h.kill()
        shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
