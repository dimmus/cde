#!/bin/bash
# CDE benchmarks: dtfile time to first paint and to idle on big folders.
#
#   dtfile-time.sh [-n "1000 10000"] [-v "by_name_and_icon by_name"] [-o OUT.jsonl]
#
# For each size N (default "1000 10000"; the TODO's 100000 works too, but
# takes minutes) it generates a folder with dtfile-gendir.py, then opens
# it with the uninstalled dtfile of this tree ($DTFILE to override) on a
# private Xvfb with a private ttsession, twice: "cold" is the first open
# after the folder was written (dropping the page cache needs root) and
# "warm" the second; it prints apptime.py's
# JSON: map_s (window up), idle_s (dtfile stopped using CPU: all icons
# typed and laid out), cpu_s and dtfile's malloc/request/round trip totals.
#
# View modes are dtfile's "view" resource values (by_name_and_icon,
# by_name, by_name_and_small_icon, by_attributes).

set -u
. "$(dirname "$0")/cdeenv.sh"

SIZES="1000 10000" VIEWS="by_name_and_icon" OUT=
while getopts n:v:o: opt; do
	case $opt in
	n) SIZES=$OPTARG ;;
	v) VIEWS=$OPTARG ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-n SIZES] [-v VIEWS] [-o OUT.jsonl]" >&2; exit 2 ;;
	esac
done

DTFILE=${DTFILE:-$(cde_bin dtfile)}
WORK=$(mktemp -d "${TMPDIR:-/tmp}/dtfilebench.XXXXXX")
trap 'chmod -R u+rwx "$WORK" 2>/dev/null; rm -rf "$WORK"' EXIT
mkdir -p "$WORK/home"

for n in $SIZES; do
	dir=$WORK/folder$n
	"$CDE_BENCH/dtfile-gendir.py" -n "$n" "$dir"
	for view in $VIEWS; do
		for temp in cold warm; do
			HOME=$WORK/home "$CDE_BENCH/apptime.py" --xvfb --ttsession \
				--label "dtfile-$n-$view-$temp" \
				--name "folder$n" --idle-ms 1500 --timeout 600 -- \
				"$DTFILE" -xrm "*retryLoadDesktop: 0" \
				-xrm "*view: $view" -dir "$dir" |
				tee -a "${OUT:-/dev/null}"
		done
	done
done
