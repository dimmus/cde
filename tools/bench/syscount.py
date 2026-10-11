#!/usr/bin/env python3
#
# CDE benchmarks: system call counts from strace logs.
#
# Licensed under the LGPL 2.1 license.
#
"""Count system calls per process, per case and per time window.

  syscount.py run [-o LOG] [--json] -- COMMAND [ARG...]
  syscount.py summary [--from T0] [--to T1] [--json] LOG
  syscount.py cases [--server XLOG] LOG

"run" runs COMMAND under strace (following forks) and prints, like
"strace -f -c" but per process name, how many system calls each made.

"cases" splits the log of a benchmark that wrote markers (benchutil's
bench_marker(), dtwmbench: with CDEBENCH_MARKERS=1 every timed run is
framed by a failing write(-1, "cdebench:begin CASE") and write(-1,
"cdebench:end CASE OPS")) and prints one JSON object per case with the
system calls per operation of all the traced processes, per process name
and per system call name.  With --server, the log of the X server
(traced separately, with the same -ttt timestamps) is split by the
markers' times too.

The logs are those of
  strace -f -qq -ttt -Y -s 64 -e signal=none -o LOG
(strace_cmd() below): one line per system call, prefixed by the pid and
the process name, and the time of the call.  A call that blocks is
counted once, at its "<unfinished ...>" line.

Counting with strace slows the traced processes down several times: the
counts are exact, but the times of a traced run mean nothing.  perf
trace and the raw_syscalls tracepoints need tracefs and BPF, which an
unprivileged user does not have (kernel.perf_event_paranoid=2).
"""

import argparse
import collections
import json
import os
import re
import subprocess
import sys

LINE = re.compile(r'^(\d+)(?:<([^>]*)>)?\s+(\d+\.\d+)\s+(.*)$')
CALL = re.compile(r'^([a-z_][a-z0-9_]*)\(')
MARK = re.compile(r'^write\(-1, "cdebench:(begin|end) ([^"]*)"')


def strace_cmd(log, follow=True):
    """The strace command line (a prefix) that writes LOG."""
    cmd = ["strace"]
    if follow:
        cmd.append("-f")
    return cmd + ["-qq", "-ttt", "-Y", "-s", "64", "-e", "signal=none",
                  "-o", log]


def events(path):
    """(time, pid, comm, syscall, marker) for every system call of LOG;
    marker is (kind, text) for the benchmarks' markers, else None."""
    try:
        f = open(path, errors="replace")
    except OSError:
        return
    with f:
        for line in f:
            m = LINE.match(line)
            if not m:
                continue
            rest = m.group(4)
            if rest.startswith("<...") or rest.startswith("+++") or \
                    rest.startswith("---"):
                continue
            c = CALL.match(rest)
            if not c:
                continue
            mk = MARK.match(rest)
            yield (float(m.group(3)), int(m.group(1)), m.group(2) or "?",
                   c.group(1), (mk.group(1), mk.group(2)) if mk else None)


def summary(path, t0=None, t1=None):
    """Counters by process name and by system call, between two times."""
    by_comm = collections.Counter()
    by_name = collections.Counter()
    for ts, _pid, comm, name, mark in events(path):
        if mark or (t0 is not None and ts < t0) or \
                (t1 is not None and ts >= t1):
            continue
        by_comm[comm] += 1
        by_name[name] += 1
    return by_comm, by_name


def count_matching(path, t0, t1, names):
    return sum(1 for ts, _p, _c, n, mk in events(path)
               if not mk and t0 <= ts < t1 and n in names)


def top(counter, n=8, div=1.0):
    return {k: round(v / div, 3) for k, v in counter.most_common(n)}


def cases(path, server=None):
    """Per case: the system calls of every traced process between its
    begin and end markers (summed over the runs), and the operations."""
    acc = collections.OrderedDict()
    cur = None
    for ts, _pid, comm, name, mark in events(path):
        if mark:
            kind, text = mark
            if kind == "begin":
                cur = {"case": text, "t0": ts, "comm": collections.Counter(),
                       "name": collections.Counter()}
            elif cur is not None:
                parts = text.rsplit(" ", 1)
                case = parts[0]
                ops = int(parts[1]) if len(parts) == 2 and \
                    parts[1].isdigit() else 1
                a = acc.setdefault(case, {"ops": 0, "comm":
                                          collections.Counter(), "name":
                                          collections.Counter(),
                                          "windows": []})
                a["ops"] += ops
                a["comm"].update(cur["comm"])
                a["name"].update(cur["name"])
                a["windows"].append((cur["t0"], ts))
                cur = None
            continue
        if cur is not None:
            cur["comm"][comm] += 1
            cur["name"][name] += 1
    res = []
    for case, a in acc.items():
        ops = max(a["ops"], 1)
        r = {"name": case, "syscall_ops": a["ops"],
             "syscalls_per_op": round(sum(a["comm"].values()) / ops, 3),
             "syscalls_by_process_per_op": top(a["comm"], 8, ops),
             "top_syscalls_per_op": top(a["name"], 8, ops)}
        if server:
            sc = collections.Counter()
            for t0, t1 in a["windows"]:
                _c, n = summary(server, t0, t1)
                sc.update(n)
            r["server_syscalls_per_op"] = round(sum(sc.values()) / ops, 3)
            r["server_top_syscalls_per_op"] = top(sc, 5, ops)
        res.append(r)
    return res


def print_summary(by_comm, by_name, as_json):
    if as_json:
        print(json.dumps({"syscalls": sum(by_comm.values()),
                          "by_process": dict(by_comm.most_common()),
                          "by_syscall": dict(by_name.most_common())}))
        return
    print("%10s  %s" % ("calls", "process"))
    for k, v in by_comm.most_common():
        print("%10d  %s" % (v, k))
    print("%10d  total\n" % sum(by_comm.values()))
    print("%10s  %s" % ("calls", "syscall"))
    for k, v in by_name.most_common(25):
        print("%10d  %s" % (v, k))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("-o", "--output", help="keep the strace log here")
    r.add_argument("--json", action="store_true")
    r.add_argument("command", nargs=argparse.REMAINDER)
    s = sub.add_parser("summary")
    s.add_argument("--from", dest="t0", type=float)
    s.add_argument("--to", dest="t1", type=float)
    s.add_argument("--json", action="store_true")
    s.add_argument("log")
    c = sub.add_parser("cases")
    c.add_argument("--server")
    c.add_argument("log")
    a = ap.parse_args()
    if a.cmd == "run":
        cmd = a.command[1:] if a.command[:1] == ["--"] else a.command
        if not cmd:
            ap.error("no command")
        log = a.output or "syscount.%d.log" % os.getpid()
        rc = subprocess.call(strace_cmd(log) + ["--"] + cmd)
        print_summary(*summary(log), a.json)
        if not a.output:
            os.unlink(log)
        return rc
    if a.cmd == "summary":
        print_summary(*summary(a.log, a.t0, a.t1), a.json)
        return 0
    for r in cases(a.log, a.server):
        print(json.dumps(r))
    return 0


if __name__ == "__main__":
    sys.exit(main())
