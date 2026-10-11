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
  mallocs, requests, round_trips   the preload's totals at exit (or at
                                   the SIGTERM that stops it) of the
                                   process started, not of its children
  children_mallocs  the mallocs of its forked children that exited
                    normally (dtfile reads folders in such children)

Optional measurements (each adds fields, or rows):

  --strace         run the command under strace -f (syscount.py) and add
                   syscalls (launch to idle, all its processes) and
                   top_syscalls; the times of such a run are not
                   comparable with those of a plain run
  --idle-window S  once idle, watch it for S seconds: idle_wakeups (the
                   context switches of all its threads), idle_cpu_s,
                   idle_forks (new child processes), and with --strace
                   idle_syscalls and idle_top_syscalls
  --action L=KEYS  after that, focus its window and type KEYS with
                   xdotool (e.g. "sort=alt+v s"), then time until it is
                   idle again; one more row, named LABEL-L, with seconds,
                   cpu_s, and the preload's mallocs, requests and
                   round_trips for that action (from snapshots taken
                   with CDEBENCH_REPORT_SIGNAL); repeatable, run in order
  --xops           connect the command through cdebench-proxy and add
                   x_ops (requests per type) and x_op_round_trips, summed
                   over its connections

With --wm the tree's dtwm (front panel off) manages the display for
the duration, started after the ttsession: without a workspace manager,
dtfile sleeps 2 s at startup (Desktop.c retries DtWsmGetWorkspaceList).

With --ttsession a private "ttsession -s -S -d $DISPLAY" runs for the
duration, so that ToolTalk clients (dtfile, dtmail) find a session
instead of auto-starting one inside the measured time.  The process is
stopped with SIGTERM (then SIGKILL) once idle, or --close-key KEYS is
sent to its window with xdotool first (for programs that only exit
cleanly from their UI).
"""

import argparse
import collections
import json
import os
import select
import shutil
import signal
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import syscount  # noqa: E402

CLK_TCK = os.sysconf("SC_CLK_TCK")
SNAP_SIGNAL = 40          # SIGRTMIN + 6: the preload's snapshot signal


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


def children(pid):
    try:
        return [int(p) for p in os.listdir("/proc") if p.isdigit()
                and ppid_of(int(p)) == pid]
    except OSError:
        return []


def ctxt_switches(pid):
    """Context switches of all the threads of pid."""
    total = 0
    try:
        tids = os.listdir("/proc/%d/task" % pid)
    except OSError:
        return 0
    for t in tids:
        try:
            with open("/proc/%d/task/%s/status" % (pid, t)) as f:
                for line in f:
                    if line.startswith(("voluntary_ctxt_switches:",
                                        "nonvoluntary_ctxt_switches:")):
                        total += int(line.split()[-1])
        except (OSError, ValueError, IndexError):
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


def remove_dtdbcache(display):
    """Remove the shared action database cache the clients of a private
    display build (it would outlive the display, and make the next run
    on the same display number skip the build)."""
    try:
        os.unlink("/tmp/dtdbcache_" + display)
    except OSError:
        pass


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
    remove_dtdbcache(os.environ["DISPLAY"])
    return proc


def start_wm():
    """The tree's dtwm, front panel off, on $DISPLAY; returns once it
    manages the screen (its "dtwm" window exists, and a little more)."""
    top = os.environ.get("CDE_TOP", os.path.join(HERE, "..", ".."))
    wm = os.path.join(top, "programs", "dtwm", ".libs", "dtwm")
    if not os.access(wm, os.X_OK):
        wm = shutil.which("dtwm")
    if not wm:
        raise SystemExit("apptime: no dtwm for --wm")
    proc = subprocess.Popen([wm, "-xrm", "Dtwm*useFrontPanel: False",
                             "-xrm", "Dtwm*showFeedback: -kill"],
                            stdin=subprocess.DEVNULL,
                            stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    deadline = time.monotonic() + 30
    while time.monotonic() < deadline and proc.poll() is None:
        try:
            out = subprocess.run(["xdotool", "search", "--name", "^dtwm$"],
                                 capture_output=True, text=True,
                                 timeout=10).stdout
        except subprocess.TimeoutExpired:
            out = ""
        if out.strip():
            time.sleep(0.5)
            return proc
        time.sleep(0.05)
    raise SystemExit("apptime: dtwm did not start")


def type_keys(keys):
    """Type KEYS ("alt+v s"): a chord's modifiers are pressed in a call
    of their own first, since Motif misses a menu mnemonic whose Alt
    comes in the same burst as the key (xdotool key alt+v)."""
    def xdo(*a):
        subprocess.run(["xdotool"] + list(a), timeout=30)
    for i, tok in enumerate(keys.split()):
        if i:
            time.sleep(0.15)
        parts = tok.split("+")
        mods, key = parts[:-1], parts[-1]
        if mods:
            xdo("keydown", *mods)
            time.sleep(0.05)
        xdo("key", key)
        if mods:
            time.sleep(0.05)
            xdo("keyup", *mods)


def free_proxy_display(first=60):
    """A display number whose TCP port (6000 + N) is free."""
    import socket
    for n in range(first, first + 200):
        sk = socket.socket()
        try:
            sk.bind(("127.0.0.1", 6000 + n))
            return n
        except OSError:
            continue
        finally:
            sk.close()
    raise SystemExit("apptime: no free port for cdebench-proxy")


def read_counters(path, pid):
    """The preload's last line of process pid, and the mallocs of the
    final lines of its children (each counts its own work: the preload
    restarts the counters in a forked child)."""
    res = {}
    kids = 0
    try:
        with open(path) as f:
            for line in f:
                kv = dict(t.split("=", 1) for t in line.split()[1:]
                          if "=" in t)
                try:
                    if int(kv.get("pid", -1)) == pid:
                        res.update({"mallocs": int(kv["mallocs"]),
                                    "requests": int(kv["requests"]),
                                    "round_trips": int(kv["rtrips"])})
                    elif int(kv.get("ppid", -1)) == pid and \
                            kv.get("event", "exit") != "signal":
                        kids += int(kv["mallocs"])
                except (KeyError, ValueError):
                    continue
    except OSError:
        pass
    if kids:
        res["children_mallocs"] = kids
    return res


def snapshot(path, pid, timeout=5.0):
    """Ask the preload in pid for its totals now, and return them."""
    try:
        before = os.path.getsize(path)
    except OSError:
        before = 0
    try:
        os.kill(pid, SNAP_SIGNAL)
    except OSError:
        return {}
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            if os.path.getsize(path) > before:
                break
        except OSError:
            pass
        time.sleep(0.01)
    c = read_counters(path, pid)
    c.pop("children_mallocs", None)
    return c


class Watch:
    """CPU-time polling of the process under test (and its children)."""

    def __init__(self, pid, idle_ms):
        self.pid = pid
        self.idle = idle_ms / 1000.0
        self.last_cpu = proc_cpu(pid)
        self.last_change = time.monotonic()

    def poll(self):
        now = time.monotonic()
        cpu = proc_cpu(self.pid)
        if cpu != self.last_cpu:
            self.last_cpu, self.last_change = cpu, now
        return now

    def wait_idle(self, timeout, started=None):
        """Wait until no CPU was used for idle_ms; returns the time of
        the last use.  With started, first wait up to that long for it
        to start using CPU at all (an action that costs nothing)."""
        t0 = time.monotonic()
        cpu0 = self.last_cpu
        if started:
            while time.monotonic() - t0 < started:
                self.poll()
                if self.last_cpu != cpu0:
                    break
                time.sleep(0.01)
        while True:
            now = self.poll()
            if now - self.last_change >= self.idle:
                return self.last_change
            if now - t0 > timeout:
                return None
            time.sleep(0.02)


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
    ap.add_argument("--wm", action="store_true",
                    help="run the tree's dtwm for the duration")
    ap.add_argument("--close-key", help="xdotool key(s) to close the window")
    ap.add_argument("--no-preload", action="store_true")
    ap.add_argument("--strace", action="store_true",
                    help="count system calls (strace -f)")
    ap.add_argument("--idle-window", type=float, default=0,
                    help="seconds to watch it once idle")
    ap.add_argument("--action", action="append", default=[],
                    help="LABEL=KEYS to type once idle (repeatable)")
    ap.add_argument("--action-timeout", type=float, default=600)
    ap.add_argument("--xops", action="store_true",
                    help="count requests per type through cdebench-proxy")
    ap.add_argument("--log", help="keep the command's output in this file")
    ap.add_argument("command", nargs=argparse.REMAINDER)
    args = ap.parse_args()
    cmd = args.command[1:] if args.command[:1] == ["--"] else args.command
    if not cmd:
        ap.error("no command")
    if not shutil.which("xdotool"):
        raise SystemExit("apptime: needs xdotool")

    helpers = []
    private_display = None
    work = tempfile.mkdtemp(prefix="apptime.")
    counters = os.path.join(work, "counters")
    try:
        if args.xvfb or not os.environ.get("DISPLAY"):
            helpers.append(start_xvfb())
            private_display = os.environ["DISPLAY"]
        env = dict(os.environ)
        if args.ttsession:
            tts = shutil.which("ttsession", path=os.path.join(
                os.environ.get("CDE_TOP", os.path.join(HERE, "..", "..")),
                "lib", "tt", "bin", "ttsession", ".libs") + os.pathsep +
                os.environ.get("PATH", ""))
            if not tts:
                raise SystemExit("apptime: no ttsession for --ttsession")
            # -p prints the session id once it is set up and published
            # on the display: a fixed wait was too short on a loaded
            # machine, and the client then spent seconds auto-starting
            # a ttsession of its own.
            tt = subprocess.Popen(
                [tts, "-s", "-S", "-p", "-d", os.environ["DISPLAY"]],
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
            helpers.append(tt)
            if not select.select([tt.stdout], [], [], 30)[0] or \
                    not tt.stdout.readline():
                raise SystemExit("apptime: ttsession did not start")
        if args.wm:
            helpers.append(start_wm())
        if not args.no_preload:
            env["LD_PRELOAD"] = os.path.join(HERE, "libcdebench_preload.so")
            env["CDEBENCH_REPORT"] = counters
            # It is stopped with SIGTERM once idle: report then too.
            env["CDEBENCH_REPORT_SIGTERM"] = "1"

        proxy = None
        proxy_stats = os.path.join(work, "proxy")
        if args.xops:
            port = free_proxy_display()
            proxy = subprocess.Popen(
                [os.path.join(HERE, "cdebench-proxy"), "-s", proxy_stats,
                 str(port), "/tmp/.X11-unix/X" +
                 os.environ["DISPLAY"].lstrip(":").split(".")[0]],
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
            proxy.stdout.readline()
            env["DISPLAY"] = "127.0.0.1:%d" % port
            helpers.append(proxy)
        if not args.no_preload:
            env["CDEBENCH_REPORT_SIGNAL"] = str(SNAP_SIGNAL)
        trace = os.path.join(work, "strace")
        run = (syscount.strace_cmd(trace) + ["--"] + cmd) if args.strace \
            else cmd

        t0 = time.monotonic()
        t0_wall = time.time()
        proc = subprocess.Popen(run, env=env, stdin=subprocess.DEVNULL,
                                stdout=open(os.path.join(work, "log"), "w"),
                                stderr=subprocess.STDOUT)
        app = proc.pid
        if args.strace:
            # The command is strace's child.
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline and proc.poll() is None:
                kids = children(proc.pid)
                if kids:
                    app = kids[0]
                    break
                time.sleep(0.005)
        deadline = t0 + args.timeout
        map_s = None
        watch = Watch(app, args.idle_ms)
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
            watch.poll()
            if map_s is not None and now - watch.last_change >= \
                    args.idle_ms / 1000.0:
                break
            time.sleep(0.02)
        idle_s = watch.last_change - t0
        if map_s is not None and idle_s < map_s:
            idle_s = map_s
        idle_wall = t0_wall + idle_s
        res = {"bench": "apptime",
               "name": args.label or os.path.basename(cmd[0]),
               "map_s": round(map_s, 4) if map_s is not None else None,
               "idle_s": round(idle_s, 4), "cpu_s": round(watch.last_cpu, 3)}
        rows = []
        alive = proc.poll() is None and map_s is not None
        # The counts of the launch: what it did until idle, without what
        # the idle window and the actions add.
        launch = snapshot(counters, app) if alive and not args.no_preload \
            and (args.action or args.idle_window > 0) else {}

        if alive and args.idle_window > 0:
            w0, c0, k0 = ctxt_switches(app), proc_cpu(app), set(children(app))
            tw0 = time.time()
            forks = set()
            end_w = time.monotonic() + args.idle_window
            while time.monotonic() < end_w and proc.poll() is None:
                forks.update(set(children(app)) - k0)
                time.sleep(0.05)
            tw1 = time.time()
            res.update({"idle_window_s": args.idle_window,
                        "idle_wakeups": ctxt_switches(app) - w0,
                        "idle_cpu_s": round(proc_cpu(app) - c0, 3),
                        "idle_forks": len(forks)})
            res["idle_wakeups_per_min"] = round(
                res["idle_wakeups"] * 60.0 / args.idle_window, 1)
            if args.strace:
                _c, n = syscount.summary(trace, tw0, tw1)
                res["idle_syscalls"] = sum(n.values())
                res["idle_top_syscalls"] = syscount.top(n, 8)

        for spec in args.action:
            if not alive or proc.poll() is not None:
                break
            label, _, keys = spec.partition("=")
            win = find_window(args.name)
            if not win:
                log("no window for action %s" % label)
                break
            before = snapshot(counters, app) if not args.no_preload else {}
            # Motif's menu mnemonics only work here with the pointer in
            # the window too (there is no window manager).
            subprocess.run(["xdotool", "mousemove", "--window", win, "200",
                            "300"],
                           timeout=10, stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)
            subprocess.run(["xdotool", "windowfocus", "--sync", win],
                           timeout=10, stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL)
            watch.poll()
            c0 = watch.last_cpu
            tw0 = time.time()
            type_keys(keys)
            # From the last key: what the menus cost while typing is in
            # cpu_s and in the counts, not in seconds.
            ta = time.monotonic()
            last = watch.wait_idle(args.action_timeout, started=2.0)
            tw1 = time.time()
            row = {"bench": "apptime", "name": "%s-%s" % (res["name"], label),
                   "keys": keys,
                   "seconds": round(max((last or time.monotonic()) - ta,
                                        0.0), 4),
                   "cpu_s": round(watch.last_cpu - c0, 3)}
            if last is None:
                row["timed_out"] = True
            after = snapshot(counters, app) if not args.no_preload else {}
            for k in ("mallocs", "requests", "round_trips"):
                if k in before and k in after:
                    row[k] = after[k] - before[k]
            if args.strace:
                _c, n = syscount.summary(trace, tw0, tw1)
                row["syscalls"] = sum(n.values())
                row["top_syscalls"] = syscount.top(n, 8)
            rows.append(row)

        win = find_window(args.name)
        if args.close_key and win and proc.poll() is None:
            subprocess.run(["xdotool", "key", "--window", win] +
                           args.close_key.split(), timeout=10)
            try:
                proc.wait(10)
            except subprocess.TimeoutExpired:
                pass
        if proc.poll() is None:
            try:
                os.kill(app, signal.SIGTERM)
            except OSError:
                pass
            try:
                proc.wait(10)
            except subprocess.TimeoutExpired:
                try:
                    os.kill(app, signal.SIGKILL)
                except OSError:
                    pass
                proc.kill()
                proc.wait()
        res.update(read_counters(counters, app))
        res.update(launch)
        if args.strace:
            _c, n = syscount.summary(trace, None, idle_wall)
            res["syscalls"] = sum(n.values())
            res["top_syscalls"] = syscount.top(n, 8)
            # Its own, and those of the processes it forked (dtfile reads
            # folders in children).
            kids = collections.Counter()
            for ts, pid, _comm, name, mk in syscount.events(trace):
                if not mk and ts < idle_wall and pid != app and \
                        pid != proc.pid:
                    kids[name] += 1
            if kids:
                res["children_syscalls"] = sum(kids.values())
                res["children_top_syscalls"] = syscount.top(kids, 8)
        if proxy:
            proxy.send_signal(signal.SIGTERM)
            try:
                proxy.wait(10)
            except subprocess.TimeoutExpired:
                proxy.kill()
            helpers.remove(proxy)
            ops, rts = {}, {}
            try:
                with open(proxy_stats) as f:
                    for line in f:
                        try:
                            c = json.loads(line)
                        except ValueError:
                            continue
                        for k, v in c.get("ops", {}).items():
                            ops[k] = ops.get(k, 0) + v
                        for k, v in c.get("op_round_trips", {}).items():
                            rts[k] = rts.get(k, 0) + v
            except OSError:
                pass
            res["x_ops"] = dict(sorted(ops.items(), key=lambda kv: -kv[1]))
            res["x_op_round_trips"] = dict(sorted(rts.items(),
                                                  key=lambda kv: -kv[1]))
        if args.log:
            shutil.copyfile(os.path.join(work, "log"), args.log)
        print(json.dumps(res), flush=True)
        for row in rows:
            print(json.dumps(row), flush=True)
        return 0 if map_s is not None else 1
    finally:
        for h in reversed(helpers):
            h.terminate()
            try:
                h.wait(10)
            except subprocess.TimeoutExpired:
                h.kill()
        if private_display:
            remove_dtdbcache(private_display)
        shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
