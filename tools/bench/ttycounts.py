#!/usr/bin/env python3
#
# CDE benchmarks: a terminal's counts from libcdebench_preload.so lines.
#
# Licensed under the LGPL 2.1 license.
#
"""Print the counts of a terminal emulator from a CDEBENCH_REPORT file.

  ttycounts.py [--comm dtterm] FILE

The terminal is the process of that name with the most X requests at
exit (a forked helper that exits normally reports too).  Prints JSON:
requests, round_trips, mallocs (its totals at exit) and, when
ttywrite.sh took two snapshots (event=signal lines), output_requests,
output_round_trips and output_mallocs: their differences, the cost of
the output alone.
"""

import argparse
import json
import sys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--comm", default="dtterm")
    ap.add_argument("file")
    a = ap.parse_args()
    final, snaps = {}, {}
    try:
        lines = open(a.file).read().splitlines()
    except OSError:
        lines = []
    for line in lines:
        kv = dict(t.split("=", 1) for t in line.split()[1:] if "=" in t)
        if kv.get("comm") != a.comm:
            continue
        try:
            pid = int(kv["pid"])
            c = {"requests": int(kv["requests"]),
                 "round_trips": int(kv["rtrips"]),
                 "mallocs": int(kv["mallocs"])}
        except (KeyError, ValueError):
            continue
        if kv.get("event") == "signal":
            snaps.setdefault(pid, []).append(c)
        else:
            final[pid] = c
    if not final:
        print(json.dumps({}))
        return 0
    pid = max(final, key=lambda p: final[p]["requests"])
    res = dict(final[pid])
    s = snaps.get(pid, [])
    if len(s) >= 2:
        for k in ("requests", "round_trips", "mallocs"):
            res["output_" + k] = s[-1][k] - s[0][k]
    print(json.dumps(res))
    return 0


if __name__ == "__main__":
    sys.exit(main())
