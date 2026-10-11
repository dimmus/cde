#!/usr/bin/env python3
#
# CDE benchmarks: run them all, write JSON, compare two runs.
#
# Licensed under the LGPL 2.1 license (ideas from Motif's
# src/tests/bench/bench.py).
#
"""Run the CDE benchmarks reproducibly and compare runs.

  bench.py run [-o OUT.json] [-r ROUNDS] [--script-rounds N] [--quick]
               [--no-pin] [--syscalls] [GROUP...]
  bench.py compare [-t PCT] [--syscall-threshold PCT] BASE.json HEAD.json
  bench.py profile [-o DIR] GROUP [CASE...]
  bench.py report RUN.json       (Markdown tables, as in README.md)

Groups (default: all but dtfile-perf):
  dtsvc       dtsvcbench   libDtSvc/libDtWidget micro-benchmarks
  tt          ttbench      ToolTalk, under a private "ttsession -s -c"
  dtwm        dtwmbench    dtwm macro-benchmarks (XTest)
  dtterm      dtterm-throughput.sh
  dtterm-vte  dtterm-vtebench.sh   vtebench-style streams, per screen
  dtterm-less dtterm-less.py       less page-down, 2 ms added delay
  dtfile      dtfile-time.sh
  dtfile-copy dtfile-copy.sh       dtfile_copy against cp
  dtfile-perf dtfile-perf.sh       dtfile's DT_PERFORMANCE phase timers
                                   (builds a dtfile; not in the default)
  dtmail      dtmail-time.sh       open, sort, delete all, idle
  loader      loader-cost.sh (dsos, relocations; counted, not timed)

--syscalls adds a second pass that counts system calls with strace
(syscount.py), whose counts are merged into the rows of the timed pass:
the C benchmarks (5 rounds, with CDEBENCH_MARKERS) get
syscalls_per_op (of the median round), per process and per call, and
the X server's
(server_syscalls_per_op, its Xvfb traced too); dtfile, dtmail and
dtfile-copy run with -S (syscalls to idle, the forked children's, the
idle window's).  strace slows everything down: the times of that pass
are thrown away.

"profile" runs one group (the C benchmarks: the given cases, one round)
under perf-profile.sh on its own Xvfb and leaves perf.data, report.txt
and stacks.folded in DIR (default ./profile-GROUP).

Every X group runs on its own Xvfb (1920x1080x24 +RENDER, Unix socket
only), started with -displayfd.  Unless --no-pin, the Xvfb, the benchmark
and the program under test are pinned together to CPU 0 with taskset:
wake-up latency across CPUs of a virtual machine otherwise dominates the
per-operation times (see dtwmbench.c).

The C benchmarks report, per case, the median over ROUNDS runs (default
5; their -r).  The script groups run ROUNDS times and report the median
of every number.  --quick runs one round with smaller inputs.

The tt group also gets libtt's RPCs per operation (ttbench-rpcs.py,
with TT_STATS) when the tree's libtt counts them: rpcs_per_op,
rpcs_sync_per_op, rpcs_oneway_per_op, rpcs_top_per_op.

"compare" exits with status 1 when a case of HEAD is more than PCT % (default
5) slower than in BASE (ns_per_op, seconds, map_s, idle_s) or makes more
X round trips (round_trips_per_op, round_trips, output_round_trips,
round_trips_per_page, round_trips_per_burst), when it makes more than
--syscall-threshold % (default 10) more system calls or ToolTalk RPCs
(syscalls_per_op, syscalls, rpcs_per_op; both runs must have counted
them), when a program loads more
shared objects or has more unused direct dependencies (loader: dsos,
unused) or more than PCT % more startup relocations, and when a case or
one of those numbers that BASE has is missing from a group HEAD ran (the
benchmark failed, timed out or skipped it), or was run at another size
(n, bytes, screens, pages: --quick against a full run, say).

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
import signal
import statistics
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.environ.get("CDE_TOP", os.path.abspath(os.path.join(HERE, "..", "..")))
sys.path.insert(0, HERE)
import syscount  # noqa: E402

GROUPS = ("dtsvc", "tt", "dtwm", "dtterm", "dtterm-vte", "dtterm-less",
          "dtfile", "dtfile-copy", "dtfile-perf", "dtmail", "loader")
DEFAULT_GROUPS = tuple(g for g in GROUPS if g != "dtfile-perf")
C_GROUPS = ("dtsvc", "tt", "dtwm")
TIME_FIELDS = ("ns_per_op", "seconds", "map_s", "idle_s")
COUNT_FIELDS = ("round_trips_per_op", "round_trips", "output_round_trips",
                "round_trips_per_page", "round_trips_per_burst")
SYSCALL_FIELDS = ("syscalls_per_op", "syscalls", "rpcs_per_op")
LOADER_STRICT = ("dsos", "unused")
LOADER_PCT = ("relocations",)
SIZE_FIELDS = ("n", "bytes", "screens", "pages")


def log(msg):
    print("bench.py: " + msg, file=sys.stderr, flush=True)


def remove_dtdbcache(display):
    """Remove the shared action database cache of an X display."""
    try:
        os.unlink("/tmp/dtdbcache_" + display)
    except OSError:
        pass


def proc_children(pid):
    """The pids of the children of pid (strace's tracee)."""
    try:
        with open("/proc/%d/task/%d/children" % (pid, pid)) as f:
            return [int(p) for p in f.read().split()]
    except (OSError, ValueError):
        return []


class Xvfb:
    def __init__(self, pin, trace=None):
        rd, wr = os.pipe()
        st = syscount.strace_cmd(trace) + ["--"] if trace else []
        self.traced = bool(trace)
        self.proc = subprocess.Popen(
            pin + st + ["Xvfb", "-displayfd", str(wr), "-screen", "0",
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
            # strace -o FILE PROG blocks SIGTERM (-I4), and SIGKILL leaves
            # its Xvfb running, detached: signal the Xvfb, and strace
            # exits with it.
            kids = proc_children(self.proc.pid) if self.traced else []
            sig = signal.SIGTERM
            for _ in range(2):
                for pid in kids:
                    try:
                        os.kill(pid, sig)
                    except OSError:
                        pass
                if not kids:
                    self.proc.send_signal(sig)
                try:
                    self.proc.wait(10)
                    break
                except subprocess.TimeoutExpired:
                    sig = signal.SIGKILL
            else:
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
        if "strace" in cmd:
            # strace -o blocks SIGTERM, and its SIGKILL would leave the
            # benchmark running, detached: stop the benchmark itself.
            for pid in proc_children(proc.pid):
                try:
                    os.kill(pid, signal.SIGTERM)
                except OSError:
                    pass
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


def group_cmd(g, a, pin, env, rounds):
    """The command of group g, or None to skip it; may set env."""
    quick = a.quick
    if g == "dtsvc":
        return pin + [os.path.join(HERE, "dtsvcbench"), "-r", str(rounds)] + \
            (["-s", "0.2"] if quick else [])
    if g == "tt":
        tts = os.path.join(TOP, "lib", "tt", "bin", "ttsession", ".libs",
                           "ttsession")
        if not os.access(tts, os.X_OK):
            tts = shutil.which("ttsession")
        if not tts:
            log("no ttsession; skipping the tt group")
            return None
        env["LD_LIBRARY_PATH"] = os.path.join(TOP, "lib", "tt", "lib", ".libs")
        return pin + [tts, "-s", "-c", os.path.join(HERE, "ttbench"),
                      "-r", str(rounds)] + (["-s", "0.2"] if quick else [])
    if g == "dtwm":
        return pin + [os.path.join(HERE, "dtwmbench"), "-r", str(rounds)] + \
            (["-s", "0.2"] if quick else [])
    if g == "dtterm":
        return pin + [os.path.join(HERE, "dtterm-throughput.sh")] + \
            (["-s", "8", "-y", "2"] if quick else [])
    if g == "dtterm-vte":
        return pin + [os.path.join(HERE, "dtterm-vtebench.sh")] + \
            (["-s", "50"] if quick else [])
    if g == "dtterm-less":
        return pin + [os.path.join(HERE, "dtterm-less.py")] + \
            (["-p", "10"] if quick else [])
    if g == "dtfile":
        return pin + [os.path.join(HERE, "dtfile-time.sh")] + \
            (["-n", "1000"] if quick else ["-w", str(a.idle_window)])
    if g == "dtfile-copy":
        return pin + [os.path.join(HERE, "dtfile-copy.sh")] + \
            (["-s", "64", "-f", "1000"] if quick else [])
    if g == "dtfile-perf":
        return pin + [os.path.join(HERE, "dtfile-perf.sh")] + \
            (["-n", "1000"] if quick else [])
    if g == "dtmail":
        return pin + [os.path.join(HERE, "dtmail-time.sh")] + \
            (["-n", "1000", "-w", "5"] if quick else
             ["-w", str(a.idle_window)])
    if g == "loader":
        return [os.path.join(HERE, "loader-cost.sh")]
    return None


# Runs per case of the C benchmarks' strace pass: syscount.py takes the
# median, since one run of a case can make twice the calls of another.
SYSCALL_ROUNDS = 5

# The script groups that count system calls with -S.
STRACE_FLAG = {"dtfile": "-S", "dtmail": "-S", "dtfile-copy": "-S"}
SYSCALL_KEYS = ("syscalls", "top_syscalls", "children_syscalls",
                "children_top_syscalls", "idle_syscalls", "idle_top_syscalls")


def merge(rows, extra, keys=None):
    """Add the fields of extra's rows (all, or those in keys) to the rows
    of the same name."""
    by = {r["name"]: r for r in rows}
    for e in extra:
        r = by.get(e.get("name"))
        if r is None:
            continue
        for k, v in e.items():
            if (keys is None and k != "name") or (keys and k in keys):
                r[k] = v


def run_group(g, a, pin, env0, work, rounds, srounds, syscalls=False):
    """One pass over group g: its rows."""
    trace = os.path.join(work, g + ".strace") if syscalls else None
    xtrace = os.path.join(work, g + ".xstrace") if syscalls else None
    xvfb = Xvfb(pin, xtrace if g in C_GROUPS else None) \
        if g != "loader" else None
    env = dict(env0)
    if xvfb:
        env["DISPLAY"] = xvfb.display
    try:
        cmd = group_cmd(g, a, pin, env, SYSCALL_ROUNDS if syscalls
                        else rounds)
        if cmd is None:
            return None
        if g in C_GROUPS:
            if syscalls:
                env["CDEBENCH_MARKERS"] = "1"
                cmd = cmd[:len(pin)] + syscount.strace_cmd(trace) + \
                    ["--"] + cmd[len(pin):]
                c_bench(cmd, env, work, g + "-syscalls")
                return syscount.cases(trace, xtrace)
            return c_bench(cmd, env, work, g)
        if g == "loader":
            return jsonl(run_cmd(cmd, env).stdout)
        if syscalls:
            if g not in STRACE_FLAG:
                return None
            return jsonl(run_cmd(cmd + [STRACE_FLAG[g]], env).stdout)
        return script_bench(cmd, env, srounds)
    finally:
        if xvfb:
            xvfb.stop()


def tt_stats_supported():
    """Whether the tree's libtt counts RPCs (TT_STATS)."""
    lib = os.path.join(TOP, "lib", "tt", "lib", ".libs", "libtt.so")
    try:
        with open(lib, "rb") as f:
            return b"tt_stats: " in f.read()
    except OSError:
        return False


def tt_rpcs(pin, env0, quick):
    """libtt's RPCs per operation of each ttbench case (ttbench-rpcs.py),
    on an Xvfb of their own."""
    xvfb = Xvfb(pin)
    try:
        env = dict(env0, DISPLAY=xvfb.display)
        cmd = pin + [os.path.join(HERE, "ttbench-rpcs.py")] + \
            (["-s", "0.05,0.1"] if quick else [])
        return jsonl(run_cmd(cmd, env).stdout)
    finally:
        xvfb.stop()


def cmd_run(a):
    groups = a.groups or list(DEFAULT_GROUPS)
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
            rows = run_group(g, a, pin, env0, work, rounds, srounds)
            if rows is None:
                continue
            results[g] = rows
            if g == "tt" and tt_stats_supported():
                merge(rows, tt_rpcs(pin, env0, a.quick))
            if a.syscalls and (g in C_GROUPS or g in STRACE_FLAG):
                if not shutil.which("strace"):
                    log("no strace; no system call counts")
                    continue
                extra = run_group(g, a, pin, env0, work, rounds, srounds,
                                  syscalls=True) or []
                merge(rows, extra,
                      None if g in C_GROUPS else SYSCALL_KEYS)
    finally:
        shutil.rmtree(work, ignore_errors=True)
    doc = {"bench": "cde", "version": 1, "tree": git_describe(),
           "date": datetime.datetime.now().isoformat(timespec="seconds"),
           "rounds": rounds, "script_rounds": srounds, "pinned": bool(pin),
           "syscalls": bool(a.syscalls), "results": results}
    text = json.dumps(doc, indent=1)
    if a.output:
        with open(a.output, "w") as f:
            f.write(text + "\n")
    else:
        print(text)
    return 0


REPORT_COLUMNS = {
    "dtsvc": ("n", "ns_per_op", "cpu_ns_per_op", "mallocs_per_op",
              "requests_per_op", "round_trips_per_op", "syscalls_per_op"),
    "tt": ("n", "ns_per_op", "cpu_ns_per_op", "mallocs_per_op",
           "rpcs_per_op", "syscalls_per_op"),
    "dtwm": ("n", "ns_per_op", "cpu_ns_per_op", "server_cpu_ns_per_op",
             "mallocs_per_op", "requests_per_op", "round_trips_per_op",
             "wakeups_per_op", "syscalls_per_op", "server_syscalls_per_op"),
    "dtterm": ("bytes", "seconds", "mb_per_s", "output_requests",
               "output_round_trips", "requests_per_screen"),
    "dtterm-vte": ("screens", "seconds", "screens_per_s",
                   "requests_per_screen", "round_trips_per_burst"),
    "dtterm-less": ("pages", "seconds", "requests_per_page",
                    "round_trips_per_page"),
    "dtfile": ("map_s", "idle_s", "cpu_s", "mallocs", "requests",
               "round_trips", "syscalls", "children_syscalls",
               "idle_wakeups"),
    "dtfile-copy": ("bytes", "seconds", "mb_per_s", "user_s", "sys_s",
                    "syscalls"),
    "dtmail": ("map_s", "idle_s", "seconds", "cpu_s", "mallocs", "requests",
               "round_trips", "syscalls", "idle_wakeups"),
}


def fmt(v):
    if v is None or v == "":
        return "-"
    if isinstance(v, bool):
        return str(v)
    if isinstance(v, int):
        return "{:,}".format(v)
    if isinstance(v, float):
        if v == int(v) and abs(v) >= 1000:
            return "{:,}".format(int(v))
        if abs(v) >= 1000:
            return "{:,.0f}".format(v)
        if abs(v) >= 100:
            return "{:.1f}".format(v)
        return "{:.3g}".format(v) if abs(v) < 1 else "{:.2f}".format(v)
    return str(v)


def cmd_report(a):
    """Markdown tables of a run, one per group."""
    with open(a.file) as f:
        doc = json.load(f)
    print("Tree `%s`, %s, %s rounds (C benchmarks), %s (scripts), pinned: "
          "%s, system calls: %s.\n" % (doc.get("tree"), doc.get("date"),
                                       doc.get("rounds"),
                                       doc.get("script_rounds"),
                                       doc.get("pinned"),
                                       doc.get("syscalls", False)))
    for g, rows in doc["results"].items():
        cols = REPORT_COLUMNS.get(g)
        if g == "loader":
            rows = sorted(rows, key=lambda r: -(r.get("relocations") or 0))
            rows = rows[:a.loader_rows]
            cols = ("dsos", "unused", "exported", "relocations")
        if not cols or not rows:
            continue
        cols = [c for c in cols if any(c in r for r in rows)]
        print("### %s\n" % g)
        print("| case | " + " | ".join(cols) + " |")
        print("|---|" + "---:|" * len(cols))
        for r in rows:
            print("| %s | %s |" % (r["name"], " | ".join(fmt(r.get(c))
                                                       for c in cols)))
        print()
    return 0


def cmd_profile(a):
    if a.group not in GROUPS or a.group == "loader":
        raise SystemExit("bench.py: cannot profile %s" % a.group)
    out = os.path.abspath(a.output or "profile-" + a.group)
    a.quick = False
    xvfb = Xvfb([])
    env = dict(os.environ, CDE_TOP=TOP, DISPLAY=xvfb.display)
    try:
        cmd = group_cmd(a.group, a, [], env, 1)
        if cmd is None:
            return 1
        cmd = cmd + list(a.cases)
        comm = {"dtwm": ["-c", "dtwm"]}.get(a.group, [])
        return subprocess.call([os.path.join(HERE, "perf-profile.sh"), "-o",
                                out] + comm + ["--"] + cmd, env=env)
    finally:
        xvfb.stop()


def cmd_compare(a):
    with open(a.base) as f:
        bdoc = json.load(f)
    with open(a.head) as f:
        hdoc = json.load(f)
    base, head = bdoc["results"], hdoc["results"]
    # When both runs counted system calls, a count HEAD lost (its strace
    # pass failed) must not pass as no regression either.
    must = TIME_FIELDS + COUNT_FIELDS + \
        (("syscalls_per_op", "syscalls")
         if bdoc.get("syscalls") and hdoc.get("syscalls") else ())
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
            for k in must:
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
            for k in SYSCALL_FIELDS:
                if not isinstance(c.get(k), (int, float)) or \
                        not isinstance(b.get(k), (int, float)):
                    continue
                # RPCs per operation are differences of two runs, whose
                # fixed costs vary by an RPC or two (signal-driven
                # NEXT_MESSAGE calls): +-1/(n2 - n1), 0.2 with --quick,
                # netfile's -0.05 in a full run.  Half an RPC per
                # operation more is the least that fails; from none (or
                # such a negative) to some is no percentage.
                floor = 0.5 if k == "rpcs_per_op" else 0.0
                delta = c[k] - b[k]
                if b[k] > 0:
                    pct = delta * 100.0 / b[k]
                    flag = pct > a.syscall_threshold and delta > floor
                    rel = "%+7.1f%%" % pct
                else:
                    flag = delta > floor
                    rel = "%8s" % ("new" if delta > 0 else "+0.0%")
                bad += flag
                print("%-8s %-28s %-10s %12.4g -> %12.4g %s%s" %
                      (g, c["name"], k, b[k], c[k], rel,
                       "  MORE CALLS" if flag else ""))
            for k in LOADER_STRICT:
                if isinstance(c.get(k), int) and isinstance(b.get(k), int) \
                        and c[k] > b[k]:
                    bad += 1
                    print("%-8s %-28s %-10s %12d -> %12d  LOADER" %
                          (g, c["name"], k, b[k], c[k]))
            for k in LOADER_PCT:
                if isinstance(c.get(k), int) and isinstance(b.get(k), int) \
                        and b[k] > 0 and \
                        (c[k] - b[k]) * 100.0 / b[k] > a.threshold:
                    bad += 1
                    print("%-8s %-28s %-10s %12d -> %12d  LOADER" %
                          (g, c["name"], k, b[k], c[k]))
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
    r.add_argument("--syscalls", action="store_true",
                   help="also count system calls (strace pass)")
    r.add_argument("--idle-window", type=float, default=60,
                   help="seconds dtfile and dtmail are watched once idle")
    r.add_argument("groups", nargs="*")
    c = sub.add_parser("compare")
    c.add_argument("-t", "--threshold", type=float, default=5.0)
    c.add_argument("--syscall-threshold", type=float, default=10.0)
    c.add_argument("base")
    c.add_argument("head")
    p = sub.add_parser("profile")
    p.add_argument("-o", "--output")
    p.add_argument("--idle-window", type=float, default=0)
    p.add_argument("group")
    p.add_argument("cases", nargs="*")
    rp = sub.add_parser("report")
    rp.add_argument("--loader-rows", type=int, default=15)
    rp.add_argument("file")
    a = ap.parse_args()
    if a.cmd == "report":
        return cmd_report(a)
    if a.cmd == "run":
        return cmd_run(a)
    if a.cmd == "profile":
        return cmd_profile(a)
    return cmd_compare(a)


if __name__ == "__main__":
    sys.exit(main())
