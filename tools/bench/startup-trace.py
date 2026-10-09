#!/usr/bin/env python3
#
# CDE benchmarks: summarise a CDE_STARTUP_TRACE file.
#
# Licensed under the LGPL 2.1 license.
#
"""Summarise a CDE_STARTUP_TRACE file into the login headline numbers.

  startup-trace.py [--json] TRACE
  startup-trace.py --stamp-clock TRACE

Each line of TRACE is "<seconds> <pid> <component> <event> [realtime]"
(see startup-trace.h and startup-trace.ksh).  Lines marked "realtime"
(from ksh) are moved onto the monotonic clock with the offset of the
last "clock realtime=<s> monotonic=<s>" line, which --stamp-clock
appends.

Printed: every stamp relative to the first one, and
  T_greeter  X server ready (dtlogin server-ready)  -> greeter painted
             (dtgreet first-expose)
  T_desktop  Enter (dtgreet auth-ok)  -> front panel mapped
             (dtwm frontpanel-mapped, else dtwm ready)
  T_restore  Enter (dtgreet auth-ok)  -> last restored client mapped
             (dtsession client-mapped, the last one; else dtsession ready)
A number is left out when its stamps are missing.
"""

import argparse
import json
import sys
import time


def stamp_clock(path):
    with open(path, "a") as f:
        f.write("clock realtime=%.9f monotonic=%.9f\n" %
                (time.time(), time.monotonic()))


def parse(path):
    offset = None
    stamps = []
    with open(path) as f:
        for line in f:
            parts = line.split()
            if not parts:
                continue
            if parts[0] == "clock":
                kv = dict(p.split("=", 1) for p in parts[1:])
                offset = float(kv["monotonic"]) - float(kv["realtime"])
                continue
            if len(parts) < 4:
                continue
            t = float(parts[0])
            if parts[-1] == "realtime":
                if offset is None:
                    continue
                t += offset
            stamps.append((t, int(parts[1]), parts[2], parts[3]))
    stamps.sort()
    return stamps


def first(stamps, comp, event):
    for s in stamps:
        if s[2] == comp and s[3] == event:
            return s[0]
    return None


def last(stamps, comp, event):
    r = None
    for s in stamps:
        if s[2] == comp and s[3] == event:
            r = s[0]
    return r


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--stamp-clock", action="store_true")
    ap.add_argument("trace")
    a = ap.parse_args()
    if a.stamp_clock:
        stamp_clock(a.trace)
        return 0
    stamps = parse(a.trace)
    if not stamps:
        sys.exit("startup-trace: no stamps in %s" % a.trace)
    t0 = stamps[0][0]
    res = {}
    ready = first(stamps, "dtlogin", "server-ready")
    painted = first(stamps, "dtgreet", "first-expose")
    enter = last(stamps, "dtgreet", "auth-ok")
    panel = first(stamps, "dtwm", "frontpanel-mapped") or \
        first(stamps, "dtwm", "ready")
    restored = last(stamps, "dtsession", "client-mapped") or \
        first(stamps, "dtsession", "ready")
    if ready is not None and painted is not None:
        res["T_greeter"] = round(painted - ready, 4)
    if enter is not None and panel is not None:
        res["T_desktop"] = round(panel - enter, 4)
    if enter is not None and restored is not None:
        res["T_restore"] = round(restored - enter, 4)
    if a.json:
        print(json.dumps({"bench": "startup", "stamps": [
            {"t": round(s[0] - t0, 6), "pid": s[1], "component": s[2],
             "event": s[3]} for s in stamps], **res}))
        return 0
    for s in stamps:
        print("%10.4f  %7d  %-12s %s" % (s[0] - t0, s[1], s[2], s[3]))
    for k in ("T_greeter", "T_desktop", "T_restore"):
        if k in res:
            print("%s = %.3f s" % (k, res[k]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
