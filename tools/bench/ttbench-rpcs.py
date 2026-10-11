#!/usr/bin/env python3
#
# CDE benchmarks: ToolTalk RPCs per operation of each ttbench case.
#
# Licensed under the LGPL 2.1 license.
#
"""Count libtt's RPCs per operation of the ttbench cases (TT_STATS).

  ttbench-rpcs.py [-s S1,S2] [CASE...]

libtt counts its RPCs (to ttsession and to rpc.ttdbserver) with
TT_STATS=FILE and appends a table per process at exit.  Each case runs
twice in a fresh "ttsession -s -c ttbench -r 1 -s S CASE", at the two
scales S1 and S2 (default 0.1 and 0.3): the RPCs of the client
processes (ttbench and the children it forks, not ttsession) at the
smaller scale, subtracted from those at the larger, divided by the
difference in operations, leave the RPCs per operation without what
starting, joining and stopping cost.  One JSON object per case:
rpcs_per_op, rpcs_sync_per_op, rpcs_oneway_per_op and the procedures
(rpcs_top_per_op, "ttsession DISPATCH sync": 1.0...).
"""

import argparse
import collections
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.environ.get("CDE_TOP", os.path.abspath(os.path.join(HERE, "..", "..")))
ROW = re.compile(r'^tt_stats: (\S+)\s+(\S+)\s+(sync|1way)\s+(\d+) calls')


def run(case, scale, work, tts, env):
    stats = os.path.join(work, "stats")
    out = os.path.join(work, "out.json")
    for p in (stats, out):
        if os.path.exists(p):
            os.unlink(p)
    e = dict(env, TT_STATS=stats)
    subprocess.run([tts, "-s", "-c", os.path.join(HERE, "ttbench"), "-r", "1",
                    "-s", str(scale), "-j", out, case], env=e,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                   timeout=600)
    try:
        cases = json.load(open(out)).get("cases", [])
    except (OSError, ValueError):
        return None, None
    if not cases:
        return None, None
    calls = collections.Counter()
    comm = None
    try:
        lines = open(stats).read().splitlines()
    except OSError:
        lines = []
    for line in lines:
        if line.startswith("tt_stats: pid "):
            comm = line.split(None, 3)[3] if len(line.split()) > 3 else ""
            continue
        m = ROW.match(line)
        if m and comm != "ttsession":
            calls[(m.group(1), m.group(2), m.group(3))] += int(m.group(4))
    return cases[0]["n"], calls


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-s", "--scales", default="0.1,0.3")
    ap.add_argument("cases", nargs="*")
    a = ap.parse_args()
    s1, s2 = (float(x) for x in a.scales.split(","))
    tts = os.path.join(TOP, "lib", "tt", "bin", "ttsession", ".libs",
                       "ttsession")
    if not os.access(tts, os.X_OK):
        tts = shutil.which("ttsession")
    if not tts:
        raise SystemExit("ttbench-rpcs: no ttsession")
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = os.path.join(TOP, "lib", "tt", "lib", ".libs") + \
        (":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else "")
    cases = a.cases
    if not cases:
        out = subprocess.run([os.path.join(HERE, "ttbench"), "-l"],
                             capture_output=True, text=True, env=env).stdout
        cases = [l.split()[0] for l in out.splitlines() if l.strip()]
    work = tempfile.mkdtemp(prefix="ttrpcs.")
    try:
        for case in cases:
            n1, c1 = run(case, s1, work, tts, env)
            n2, c2 = run(case, s2, work, tts, env)
            if n1 is None or n2 is None or n2 <= n1:
                continue
            d = n2 - n1
            diff = collections.Counter({k: c2[k] - c1.get(k, 0) for k in c2})
            tot = sum(diff.values())
            sync = sum(v for k, v in diff.items() if k[2] == "sync")
            top = {" ".join(k): round(v / d, 3)
                   for k, v in diff.most_common(6) if v}
            print(json.dumps({"bench": "ttbench-rpcs", "name": case,
                              "rpcs_per_op": round(tot / d, 3),
                              "rpcs_sync_per_op": round(sync / d, 3),
                              "rpcs_oneway_per_op": round((tot - sync) / d, 3),
                              "rpcs_top_per_op": top}), flush=True)
    finally:
        shutil.rmtree(work, ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
