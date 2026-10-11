#!/bin/bash
# CDE benchmarks: dtterm output throughput.
#
#   dtterm-throughput.sh [-s SIZE_MB] [-y YES_MB] [-o OUT.jsonl]
#
# Runs dtterm (the uninstalled one of this tree, or $DTTERM) on $DISPLAY,
# or on a private Xvfb when DISPLAY is unset, once per combination of
#   input    base64 text (SIZE_MB, default 100) | "yes | head -c YES_MB"
#            (default 16: dtterm takes 1.5-4 s per MB of "y\n" lines today;
#            the TODO's 200 MB takes 5-15 minutes per run)
#   locale   C | a UTF-8 locale (C.UTF-8 or en_US.UTF-8)
#   saveLines  -sl 4s | -sl 10000
# and prints one JSON object per run: the time from dtterm idle to the
# input all drawn (ttywrite.sh: cursor position reports before and
# after), MB/s, dtterm's totals of X requests and round trips from
# libcdebench_preload.so (with its startup), and output_requests,
# output_round_trips (the input's own, from snapshots), lines, and
# requests_per_screen (per 24 lines of input).

set -u
. "$(dirname "$0")/cdeenv.sh"

SIZE_MB=100 YES_MB=16 OUT=
while getopts s:y:o: opt; do
	case $opt in
	s) SIZE_MB=$OPTARG ;;
	y) YES_MB=$OPTARG ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-s SIZE_MB] [-y YES_MB] [-o OUT.jsonl]" >&2; exit 2 ;;
	esac
done

DTTERM=${DTTERM:-$(cde_bin dtterm)}
PRELOAD=$CDE_BENCH/libcdebench_preload.so
WORK=$(mktemp -d "${TMPDIR:-/tmp}/dttermbench.XXXXXX")
trap 'rm -rf "$WORK"; [ -n "${CDE_XVFB_PID:-}" ] && kill $CDE_XVFB_PID 2>/dev/null' EXIT

if [ -z "${DISPLAY:-}" ]; then
	n=$(cde_free_display)
	cde_xvfb "$n" || exit 1
	export DISPLAY=:$n
fi

UTF8=$(locale -a 2>/dev/null | grep -i -m1 -E '^(C\.utf-?8|en_US\.utf-?8)$')
: "${UTF8:=C.UTF-8}"

# base64 of random bytes: 76-column lines of printable ASCII.
head -c $((SIZE_MB * 1024 * 1024 * 3 / 4)) /dev/urandom | base64 > "$WORK/b64.txt"
B64_BYTES=$(stat -c %s "$WORK/b64.txt")
B64_LINES=$(wc -l < "$WORK/b64.txt")
YES_BYTES=$((YES_MB * 1024 * 1024))
# "yes | head -c", written beforehand: what dtterm is sent is the same.
yes | head -c "$YES_BYTES" > "$WORK/yes.txt"

run() {	# name locale saveLines bytes lines command...
	local name=$1 loc=$2 sl=$3 bytes=$4 lines=$5 res="$WORK/res" cnt="$WORK/cnt"
	shift 5
	rm -f "$res" "$cnt"
	HOME=$WORK LANG=$loc LC_ALL=$loc CDEBENCH_REPORT=$cnt \
	CDEBENCH_REPORT_SIGNAL=40 LD_PRELOAD=$PRELOAD \
	timeout 1800 "$DTTERM" -geometry 80x24 -sl "$sl" \
		-e "$CDE_BENCH/ttywrite.sh" "$res" "$@" >"$WORK/log" 2>&1
	if [ ! -s "$res" ]; then
		echo "dtterm-throughput: $name failed:" >&2
		cat "$WORK/log" >&2
		return 1
	fi
	python3 - "$name" "$loc" "$sl" "$bytes" "$lines" "$res" \
		"$("$CDE_BENCH/ttycounts.py" "$cnt")" <<'PY' | tee -a "${OUT:-/dev/null}"
import json, sys
name, loc, sl, b, lines, res, c = sys.argv[1:8]
b, lines, c = int(b), int(lines), json.loads(c)
t0, t1 = map(int, open(res).read().split())
s = (t1 - t0) / 1e9
r = {"bench": "dtterm", "name": name, "locale": loc, "save_lines": sl,
     "bytes": b, "seconds": round(s, 3),
     "mb_per_s": round(b / 1048576 / s, 2) if s > 0 else 0,
     "requests": c.get("requests", 0), "round_trips": c.get("round_trips", 0),
     "lines": lines}
if "output_requests" in c:
    r["output_requests"] = c["output_requests"]
    r["output_round_trips"] = c["output_round_trips"]
    r["requests_per_screen"] = round(c["output_requests"] * 24.0 / lines, 3)
print(json.dumps(r))
PY
}

for loc in C "$UTF8"; do
	tag=$( [ "$loc" = C ] && echo C || echo utf8 )
	for sl in 4s 10000; do
		run "cat-base64-$tag-sl$sl" "$loc" "$sl" "$B64_BYTES" "$B64_LINES" \
			cat "$WORK/b64.txt"
		run "yes-$tag-sl$sl" "$loc" "$sl" "$YES_BYTES" $((YES_BYTES / 2)) \
			head -c "$YES_BYTES" "$WORK/yes.txt"
	done
done
