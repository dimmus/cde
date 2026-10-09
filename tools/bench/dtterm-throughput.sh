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
# and prints one JSON object per run: the time the shell inside dtterm
# took to write the input (cat/yes return once dtterm has read all but the
# last pty buffer), MB/s, and dtterm's totals of X requests and round
# trips from libcdebench_preload.so.

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
YES_BYTES=$((YES_MB * 1024 * 1024))

run() {	# name locale saveLines command bytes
	local name=$1 loc=$2 sl=$3 cmd=$4 bytes=$5 res="$WORK/res" cnt="$WORK/cnt"
	local t0 t1 s req rt
	rm -f "$res" "$cnt"
	HOME=$WORK LANG=$loc LC_ALL=$loc CDEBENCH_REPORT=$cnt \
	LD_PRELOAD=$PRELOAD timeout 1800 "$DTTERM" -geometry 80x24 -sl "$sl" \
		-e sh -c "t0=\$(date +%s%N); $cmd; t1=\$(date +%s%N); echo \$t0 \$t1 > $res" \
		>"$WORK/log" 2>&1
	if [ ! -s "$res" ]; then
		echo "dtterm-throughput: $name failed:" >&2
		cat "$WORK/log" >&2
		return 1
	fi
	read -r t0 t1 < "$res"
	s=$(awk -v a="$t0" -v b="$t1" 'BEGIN { printf "%.3f", (b - a) / 1e9 }')
	req=$(awk '/comm=dtterm/ { for (i = 1; i <= NF; i++) if ($i ~ /^requests=/) { sub("requests=", "", $i); r = $i } } END { print r + 0 }' "$cnt" 2>/dev/null)
	rt=$(awk '/comm=dtterm/ { for (i = 1; i <= NF; i++) if ($i ~ /^rtrips=/) { sub("rtrips=", "", $i); r = $i } } END { print r + 0 }' "$cnt" 2>/dev/null)
	printf '{"bench": "dtterm", "name": "%s", "locale": "%s", "save_lines": "%s", "bytes": %s, "seconds": %s, "mb_per_s": %s, "requests": %s, "round_trips": %s}\n' \
		"$name" "$loc" "$sl" "$bytes" "$s" \
		"$(awk -v b="$bytes" -v s="$s" 'BEGIN { printf "%.2f", (s > 0 ? b / 1048576 / s : 0) }')" \
		"${req:-0}" "${rt:-0}" | tee -a "${OUT:-/dev/null}"
}

for loc in C "$UTF8"; do
	tag=$( [ "$loc" = C ] && echo C || echo utf8 )
	for sl in 4s 10000; do
		run "cat-base64-$tag-sl$sl" "$loc" "$sl" "cat $WORK/b64.txt" "$B64_BYTES"
		run "yes-$tag-sl$sl" "$loc" "$sl" "yes | head -c $YES_BYTES" "$YES_BYTES"
	done
done
