#!/usr/bin/env python3
#
# CDE benchmarks: cc-trace.sh log to Chrome trace JSON.
#
# Licensed under the LGPL 2.1 license.
#
"""Convert a cc-trace.sh log to Chrome trace JSON on stdout.

  cc-trace2json.py LOG > trace.json

Jobs are packed into lanes (tid) so that overlapping compilations show
side by side; the summary on stderr gives the wall span, the summed
compile time, the effective parallelism and the ten slowest outputs.
"""

import json
import sys


def main():
    jobs = []
    with open(sys.argv[1]) as f:
        for line in f:
            p = line.split(None, 3)
            if len(p) == 4:
                jobs.append((int(p[0]), int(p[1]), p[3].strip()))
    if not jobs:
        sys.exit("cc-trace2json: empty log")
    jobs.sort()
    t0 = jobs[0][0]
    lanes = []
    events = []
    for s, e, name in jobs:
        for i, free in enumerate(lanes):
            if free <= s:
                lanes[i] = e
                lane = i
                break
        else:
            lanes.append(e)
            lane = len(lanes) - 1
        events.append({"name": name, "ph": "X", "pid": 1, "tid": lane,
                       "ts": (s - t0) / 1000.0, "dur": (e - s) / 1000.0})
    json.dump({"traceEvents": events}, sys.stdout)
    span = (max(e for _, e, _ in jobs) - t0) / 1e9
    busy = sum(e - s for s, e, _ in jobs) / 1e9
    print("jobs %d  span %.1f s  compile %.1f s  parallelism %.2f  lanes %d"
          % (len(jobs), span, busy, busy / span if span else 0, len(lanes)),
          file=sys.stderr)
    for s, e, name in sorted(jobs, key=lambda j: j[0] - j[1])[:10]:
        print("  %7.2f s  %s" % ((e - s) / 1e9, name), file=sys.stderr)


if __name__ == "__main__":
    main()
