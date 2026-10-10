#!/usr/bin/env python3
#
# CDE benchmarks: run them all, write JSON, compare two runs.
#
# Licensed under the LGPL 2.1 license (ideas from Motif's
# src/tests/bench/bench.py).
#
"""Run the CDE benchmarks reproducibly and compare runs.

  bench.py run [-o OUT.json] [-r ROUNDS] [--script-rounds N] [--quick]
               [--no-pin] [GROUP...]
  bench.py compare [-t PCT] BASE.json HEAD.json

Groups (default: all):
  dtsvc   dtsvcbench   libDtSvc/libDtWidget micro-benchmarks
  tt      ttbench      ToolTalk, under a private "ttsession -s -c"
  dtwm    dtwmbench    dtwm macro-benchmarks (XTest)
  dtterm  dtterm-throughput.sh
  dtfile  dtfile-time.sh
  dtmail  dtmail-time.sh
  loader  loader-cost.sh (dsos, relocations; counted, not timed)

Every X group runs on its own Xvfb (1920x1080x24 +RENDER, Unix socket
only), started with -displayfd.  Unless --no-pin, the Xvfb, the benchmark
and the program under test are pinned together to CPU 0 with taskset:
wake-up latency across CPUs of a virtual machine otherwise dominates the
per-operation times (see dtwmbench.c).

The C benchmarks report, per case, the median over ROUNDS runs (default
5; their -r).  The script groups run ROUNDS times and report the median
of every number.  --quick runs one round with smaller inputs.

"compare" exits with status 1 when a case of HEAD is more than PCT % (default
5) slower than in BASE (ns_per_op, seconds, map_s, idle_s) or makes more
X round trips (round_trips_per_op, round_trips), and when a case or one
of those numbers that BASE has is missing from a group HEAD ran (the
benchmark failed, timed out or skipped it), or was run at another size
(n, bytes: --quick against a full run, say).

Every Xvfb display's /tmp/dtdbcache_:N is removed before and after the
group: the CDE clients build that shared database cache when it is
missing, and one left over from an earlier run would make the next run
on the same display number skip the build.
"""

import argparse
import datetime
import json
import os
import shutil
import statistics
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.environ.get("CDE_TOP", os.path.abspath(os.path.join(HERE, "..", "..")))
GROUPS = ("dtsvc", "tt", "dtwm", "dtterm", "dtfile", "dtmail", "loader")
TIME_FIELDS = ("ns_per_op", "seconds", "map_s", "idle_s")
COUNT_FIELDS = ("round_trips_per_op", "round_trips")
SIZE_FIELDS = ("n", "bytes")


def log(msg):
    print("bench.py: " + msg, file=sys.stderr, flush=True)


def remove_dtdbcache(display):
    """Remove the shared action database cache of an X display."""
    try:
        os.unlink("/tmp/dtdbcache_" + display)
    except OSError:
        pass


class Xvfb:
    def __init__(self, pin):
        rd, wr = os.pipe()
        self.proc = subprocess.Popen(
            pin + ["Xvfb", "-displayfd", str(wr), "-screen", "0",
                   "1920x1080x24", "+extension", "RENDER", "-nolisten", "tcp",
                   "-noreset"], pass_fds=(wr,), stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        os.close(wr)
        data = b""
        while not data.endswith(b"\n"):
            chunk = os.read(rd, 16)
            if not chunk:
                break
            data += chunk
        os.close(rd)
        if not data.strip().isdigit():
            self.stop()
            raise SystemExit("bench.py: Xvfb did not start")
        self.display = ":" + data.decode().strip()
        remove_dtdbcache(self.display)

    def stop(self):
        if self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(10)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
        if hasattr(self, "display"):
            remove_dtdbcache(self.display)


def run_cmd(cmd, env, timeout=3600):
    log("running " + " ".join(cmd))
    proc = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, text=True)
    try:
        out, err = proc.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        # SIGTERM first, so that the benchmark can stop what it started
        # (dtwm, ttsession's clients, its temporary files); keep what it
        # printed, and let the other groups run.
        log("%s timed out after %d s" % (cmd[0], timeout))
        proc.terminate()
        try:
            out, err = proc.communicate(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()
            out, err = proc.communicate()
    p = subprocess.CompletedProcess(cmd, proc.returncode, out or "",
                                    err or "")
    if p.returncode not in (0, 77):
        log("%s failed (status %d):\n%s" % (cmd[0], p.returncode,
                                            p.stderr[-2000:]))
    return p


def c_bench(cmd, env, work, name):
    out = os.path.join(work, name + ".json")
    run_cmd(cmd + ["-j", out], env)
    try:
        with open(out) as f:
            return json.load(f).get("cases", [])
    except (OSError, ValueError):
        return []


def jsonl(text):
    res = []
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("{"):
            try:
                res.append(json.loads(line))
            except ValueError:
                pass
    return res


def median_rounds(rounds):
    """Per case name, the median of every numeric field over the rounds."""
    by_name = {}
    for r in rounds:
        for case in r:
            by_name.setdefault(case["name"], []).append(case)
    res = []
    for name, cases in by_name.items():
        merged = dict(cases[0])
        for k in {k for c in cases for k in c}:
            # A round that failed has null there; the others count.
            vals = [c[k] for c in cases if isinstance(c.get(k), (int, float))
                    and not isinstance(c.get(k), bool)]
            if vals:
                merged[k] = statistics.median(vals)
        merged["rounds"] = len(cases)
        res.append(merged)
    return res


def script_bench(cmd, env, rounds):
    return median_rounds([jsonl(run_cmd(cmd, env).stdout)
                          for _ in range(rounds)])


def git_describe():
    try:
        return subprocess.run(["git", "-C", TOP, "describe", "--always",
                               "--dirty"], capture_output=True, text=True,
                              timeout=30).stdout.strip()
    except (OSError, subprocess.TimeoutExpired):
        return ""


def cmd_run(a):
    groups = a.groups or list(GROUPS)
    for g in groups:
        if g not in GROUPS:
            raise SystemExit("bench.py: unknown group %s" % g)
    pin = []
    if not a.no_pin and shutil.which("taskset"):
        pin = ["taskset", "-c", "0"]
    rounds = 1 if a.quick else a.rounds
    srounds = rounds if a.script_rounds is None else a.script_rounds
    work = tempfile.mkdtemp(prefix="cdebench.")
    results = {}
    env0 = dict(os.environ, CDE_TOP=TOP)
    try:
        for g in groups:
            xvfb = Xvfb(pin) if g != "loader" else None
            env = dict(env0)
            if xvfb:
                env["DISPLAY"] = xvfb.display
            try:
                if g == "dtsvc":
                    results[g] = c_bench(pin + [os.path.join(HERE, "dtsvcbench"),
                                                "-r", str(rounds)] +
                                         (["-s", "0.2"] if a.quick else []),
                                         env, work, g)
                elif g == "tt":
                    tts = os.path.join(TOP, "lib", "tt", "bin", "ttsession",
                                       ".libs", "ttsession")
                    if not os.access(tts, os.X_OK):
                        tts = shutil.which("ttsession")
                    if not tts:
                        log("no ttsession; skipping the tt group")
                        continue
                    env["LD_LIBRARY_PATH"] = os.path.join(TOP, "lib", "tt", "lib", ".libs")
                    results[g] = c_bench(pin + [tts, "-s", "-c",
                                                os.path.join(HERE, "ttbench"),
                                                "-r", str(rounds)] +
                                         (["-s", "0.2"] if a.quick else []),
                                         env, work, g)
                elif g == "dtwm":
                    results[g] = c_bench(pin + [os.path.join(HERE, "dtwmbench"),
                                                "-r", str(rounds)] +
                                         (["-s", "0.2"] if a.quick else []),
                                         env, work, g)
                elif g == "dtterm":
                    size = ["-s", "8", "-y", "2"] if a.quick else []
                    results[g] = script_bench(
                        pin + [os.path.join(HERE, "dtterm-throughput.sh")] + size,
                        env, srounds)
                elif g == "dtfile":
                    sizes = ["-n", "1000"] if a.quick else []
                    results[g] = script_bench(
                        pin + [os.path.join(HERE, "dtfile-time.sh")] + sizes,
                        env, srounds)
                elif g == "dtmail":
                    sizes = ["-n", "1000"] if a.quick else []
                    results[g] = script_bench(
                        pin + [os.path.join(HERE, "dtmail-time.sh")] + sizes,
                        env, srounds)
                elif g == "loader":
                    results[g] = jsonl(run_cmd(
                        [os.path.join(HERE, "loader-cost.sh")], env).stdout)
            finally:
                if xvfb:
                    xvfb.stop()
    finally:
        shutil.rmtree(work, ignore_errors=True)
    doc = {"bench": "cde", "version": 1, "tree": git_describe(),
           "date": datetime.datetime.now().isoformat(timespec="seconds"),
           "rounds": rounds, "script_rounds": srounds, "pinned": bool(pin), "results": results}
    text = json.dumps(doc, indent=1)
    if a.output:
        with open(a.output, "w") as f:
            f.write(text + "\n")
    else:
        print(text)
    return 0


def cmd_compare(a):
    with open(a.base) as f:
        base = json.load(f)["results"]
    with open(a.head) as f:
        head = json.load(f)["results"]
    bad = 0
    for g, cases in head.items():
        bcases = {c["name"]: c for c in base.get(g, [])}
        hnames = {c["name"] for c in cases}
        # A group HEAD ran without some cases BASE has: the benchmark
        # failed or skipped them, which must not pass as no regression.
        for name in bcases:
            if name not in hnames:
                bad += 1
                print("%-8s %-28s MISSING" % (g, name))
        for c in cases:
            b = bcases.get(c["name"])
            if not b:
                continue
            # Per-operation numbers include fixed costs per run, so runs
            # of other sizes (--quick, -s, -y) do not compare.
            size = [k for k in SIZE_FIELDS if k in b and k in c and
                    b[k] != c[k]]
            if size:
                bad += 1
                print("%-8s %-28s %-10s %12s -> %12s  NOT COMPARABLE" %
                      (g, c["name"], size[0], b[size[0]], c[size[0]]))
                continue
            for k in TIME_FIELDS + COUNT_FIELDS:
                if isinstance(b.get(k), (int, float)) and \
                        not isinstance(c.get(k), (int, float)):
                    bad += 1
                    print("%-8s %-28s %-10s %12.4g ->      missing" %
                          (g, c["name"], k, b[k]))
            for k in TIME_FIELDS:
                if isinstance(c.get(k), (int, float)) and \
                        isinstance(b.get(k), (int, float)) and b[k] > 0:
                    pct = (c[k] - b[k]) * 100.0 / b[k]
                    flag = pct > a.threshold
                    bad += flag
                    print("%-8s %-28s %-10s %12.4g -> %12.4g %+7.1f%%%s" %
                          (g, c["name"], k, b[k], c[k], pct,
                           "  REGRESSION" if flag else ""))
            for k in COUNT_FIELDS:
                if isinstance(c.get(k), (int, float)) and \
                        isinstance(b.get(k), (int, float)) and \
                        c[k] > b[k] + 1e-9:
                    bad += 1
                    print("%-8s %-28s %-10s %12.4g -> %12.4g  MORE ROUND TRIPS"
                          % (g, c["name"], k, b[k], c[k]))
    return 1 if bad else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("-o", "--output")
    r.add_argument("-r", "--rounds", type=int, default=5)
    r.add_argument("--script-rounds", type=int,
                   help="rounds of the script groups (default ROUNDS)")
    r.add_argument("--quick", action="store_true")
    r.add_argument("--no-pin", action="store_true")
    r.add_argument("groups", nargs="*")
    c = sub.add_parser("compare")
    c.add_argument("-t", "--threshold", type=float, default=5.0)
    c.add_argument("base")
    c.add_argument("head")
    a = ap.parse_args()
    return cmd_run(a) if a.cmd == "run" else cmd_compare(a)


if __name__ == "__main__":
    sys.exit(main())
