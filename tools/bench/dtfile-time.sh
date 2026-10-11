#!/bin/bash
# CDE benchmarks: dtfile time to first paint and to idle on big folders.
#
#   dtfile-time.sh [-n "1000 10000"] [-v "by_name_and_icon by_name"]
#                  [-w IDLE_S] [-M] [-S] [-x] [-o OUT.jsonl]
#
# For each size N (default "1000 10000"; the TODO's 100000 works too, but
# takes minutes) it generates a folder with dtfile-gendir.py, then opens
# it with the uninstalled dtfile of this tree ($DTFILE to override) on a
# private Xvfb with a private ttsession and the tree's dtwm, twice:
# "cold" is the first open after the folder was written (dropping the
# page cache needs root) and "warm" the second; it prints apptime.py's
# JSON: map_s (window up), idle_s (dtfile stopped using CPU: all icons
# typed and laid out), cpu_s and dtfile's malloc/request/round trip totals.
#
# View modes are dtfile's "view" resource values (by_name_and_icon,
# by_name, by_name_and_small_icon, by_attributes).
#
#   -w IDLE_S  watch the warm run of the first view mode for IDLE_S
#              seconds once idle (idle_wakeups, idle_cpu_s, idle_forks;
#              the TODO's 10 minutes is -w 600)
#   -M         no window manager: dtfile then sleeps 2 s in its startup
#              (Desktop.c retries DtWsmGetWorkspaceList), which is what
#              the runs before this option measured
#   -S         count system calls (strace -f): syscalls, children_syscalls
#              (the forked folder readers)...; the times mean nothing then
#   -x         count requests per type through cdebench-proxy (x_ops:
#              GetGeometry, GetInputFocus, ClearArea...)

set -u
. "$(dirname "$0")/cdeenv.sh"

SIZES="1000 10000" VIEWS="by_name_and_icon" OUT= IDLE=0 WM=--wm EXTRA=
while getopts n:v:w:MSxo: opt; do
	case $opt in
	n) SIZES=$OPTARG ;;
	v) VIEWS=$OPTARG ;;
	w) IDLE=$OPTARG ;;
	M) WM= ;;
	S) EXTRA="$EXTRA --strace" ;;
	x) EXTRA="$EXTRA --xops" ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-n SIZES] [-v VIEWS] [-w IDLE_S] [-M] [-S] [-x] [-o OUT.jsonl]" >&2; exit 2 ;;
	esac
done

DTFILE=${DTFILE:-$(cde_bin dtfile)}
WORK=$(mktemp -d "${TMPDIR:-/tmp}/dtfilebench.XXXXXX")
trap 'chmod -R u+rwx "$WORK" 2>/dev/null; rm -rf "$WORK"' EXIT
mkdir -p "$WORK/home"
FIRST_VIEW=${VIEWS%% *}

for n in $SIZES; do
	dir=$WORK/folder$n
	"$CDE_BENCH/dtfile-gendir.py" -n "$n" "$dir"
	for view in $VIEWS; do
		for temp in cold warm; do
			idle=0
			[ "$view" = "$FIRST_VIEW" ] && [ $temp = warm ] && idle=$IDLE
			HOME=$WORK/home "$CDE_BENCH/apptime.py" --xvfb --ttsession $WM \
				$EXTRA --idle-window "$idle" \
				--label "dtfile-$n-$view-$temp" \
				--name "folder$n" --idle-ms 1500 --timeout 600 -- \
				"$DTFILE" -xrm "*retryLoadDesktop: 0" \
				-xrm "*view: $view" -dir "$dir" |
				tee -a "${OUT:-/dev/null}"
		done
	done
done
