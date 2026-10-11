#!/bin/bash
# CDE benchmarks: dtterm on vtebench-style output.
#
#   dtterm-vtebench.sh [-s SCREENS] [-c "CASE..."] [-o OUT.jsonl]
#
# For each case of vtgen.py (dense_cells, scrolling,
# scrolling_in_region_top, scrolling_in_region_bottom, unicode,
# cursor_motion) it writes SCREENS (default 200) screens of output to an
# 80x24 dtterm (the uninstalled one of this tree, or $DTTERM) in a UTF-8
# locale, one screen per write(2) (dd bs=<screen bytes>), on $DISPLAY or
# a private Xvfb, and prints one JSON object per case: seconds (from
# idle to all drawn: ttywrite.sh), MB/s, screens/s, dtterm's X requests,
# round trips and mallocs for the output alone (preload snapshots,
# without its startup), requests_per_screen and round_trips_per_burst (a
# burst is one write of one screen; the target is 0).

set -u
. "$(dirname "$0")/cdeenv.sh"

SCREENS=200 OUT=
CASES="dense_cells scrolling scrolling_in_region_top scrolling_in_region_bottom unicode cursor_motion"
while getopts s:c:o: opt; do
	case $opt in
	s) SCREENS=$OPTARG ;;
	c) CASES=$OPTARG ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-s SCREENS] [-c CASES] [-o OUT.jsonl]" >&2; exit 2 ;;
	esac
done

DTTERM=${DTTERM:-$(cde_bin dtterm)}
PRELOAD=$CDE_BENCH/libcdebench_preload.so
WORK=$(mktemp -d "${TMPDIR:-/tmp}/dttermvte.XXXXXX")
trap 'rm -rf "$WORK"; [ -n "${CDE_XVFB_PID:-}" ] && kill $CDE_XVFB_PID 2>/dev/null' EXIT

if [ -z "${DISPLAY:-}" ]; then
	n=$(cde_free_display)
	cde_xvfb "$n" || exit 1
	export DISPLAY=:$n
fi

UTF8=$(locale -a 2>/dev/null | grep -i -m1 -E '^(C\.utf-?8|en_US\.utf-?8)$')
: "${UTF8:=C.UTF-8}"

for case in $CASES; do
	meta=$("$CDE_BENCH/vtgen.py" -s "$SCREENS" "$case" "$WORK/$case.out") || exit 1
	bs=$(echo "$meta" | python3 -c 'import json,sys; print(json.load(sys.stdin)["screen_bytes"])')
	res=$WORK/res cnt=$WORK/cnt
	rm -f "$res" "$cnt"
	HOME=$WORK LANG=$UTF8 LC_ALL=$UTF8 CDEBENCH_REPORT=$cnt \
	CDEBENCH_REPORT_SIGNAL=40 LD_PRELOAD=$PRELOAD \
	timeout 1800 "$DTTERM" -geometry 80x24 -sl 4s \
		-e "$CDE_BENCH/ttywrite.sh" "$res" \
		dd if="$WORK/$case.out" bs="$bs" status=none >"$WORK/log" 2>&1
	if [ ! -s "$res" ]; then
		echo "dtterm-vtebench: $case failed:" >&2
		cat "$WORK/log" >&2
		continue
	fi
	python3 - "$meta" "$res" "$("$CDE_BENCH/ttycounts.py" "$cnt")" <<'PY' | tee -a "${OUT:-/dev/null}"
import json, sys
meta = json.loads(sys.argv[1])
t0, t1 = map(int, open(sys.argv[2]).read().split())
c = json.loads(sys.argv[3])
s = (t1 - t0) / 1e9
# Without both snapshots (dtterm died, or never took the signal) the
# output's counts are unknown: null, which compare reports as missing,
# not 0 round trips.
req = c.get("output_requests")
rt = c.get("output_round_trips")
bursts = -(-meta["bytes"] // meta["screen_bytes"])
print(json.dumps({"bench": "dtterm-vte", "name": "vte-" + meta["case"],
                  "bytes": meta["bytes"], "screens": meta["screens"],
                  "seconds": round(s, 3),
                  "mb_per_s": round(meta["bytes"] / 1048576 / s, 2),
                  "screens_per_s": round(meta["screens"] / s, 1),
                  "requests": req, "round_trips": rt,
                  "mallocs": c.get("output_mallocs"),
                  "requests_per_screen": None if req is None else
                  round(req / meta["screens"], 2),
                  "bursts": bursts,
                  "round_trips_per_burst": None if rt is None else
                  round(rt / bursts, 4)}))
PY
done
