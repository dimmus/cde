#!/bin/bash
# CDE benchmarks: a CPU profile of a command with perf.
#
#   perf-profile.sh [-o DIR] [-F FREQ] [-g dwarf|fp|none] [-c COMM] [-n TOP]
#                   -- COMMAND [ARG...]
#
# Records COMMAND and everything it starts (perf record follows forks)
# with the cpu-clock software event at FREQ Hz (default 999), user space
# only: kernel.perf_event_paranoid=2 allows no more, and this virtual
# machine has no hardware cycle counter.  Call graphs are unwound with
# DWARF by default (-g): the tree is built without frame pointers.
# Writes to DIR (default ./perf-profile.<pid>):
#   perf.data        the samples (perf report -i DIR/perf.data)
#   report.txt       self time by process, object and symbol
#   stacks.folded    folded stacks (flamegraph.pl, speedscope, inferno)
# and prints the TOP (default 25) symbols by self time, of the processes
# named COMM only with -c (e.g. -c dtwm under dtwmbench).
#
#   tools/bench/perf-profile.sh -c dtwm -- tools/bench/dtwmbench -r 1 map
#   tools/bench/bench.py profile dtwm map      (the same, on its own Xvfb)

set -u

DIR= FREQ=999 GRAPH=dwarf COMM= TOP=25
while getopts o:F:g:c:n: opt; do
	case $opt in
	o) DIR=$OPTARG ;;
	F) FREQ=$OPTARG ;;
	g) GRAPH=$OPTARG ;;
	c) COMM=$OPTARG ;;
	n) TOP=$OPTARG ;;
	*) echo "usage: $0 [-o DIR] [-F FREQ] [-g dwarf|fp|none] [-c COMM] [-n TOP] -- COMMAND..." >&2
	   exit 2 ;;
	esac
done
shift $((OPTIND - 1))
[ "${1:-}" = -- ] && shift
[ $# -gt 0 ] || { echo "perf-profile: no command" >&2; exit 2; }
command -v perf >/dev/null || { echo "perf-profile: needs perf" >&2; exit 1; }

: "${DIR:=perf-profile.$$}"
mkdir -p "$DIR"
case $GRAPH in
dwarf) CG=(--call-graph dwarf,16384) ;;
fp) CG=(--call-graph fp) ;;
none) CG=() ;;
*) echo "perf-profile: -g dwarf, fp or none" >&2; exit 2 ;;
esac

perf record -q -e cpu-clock:u -F "$FREQ" "${CG[@]}" -o "$DIR/perf.data" -- "$@"
rc=$?

FILTER=()
[ -n "$COMM" ] && FILTER=(--comms "$COMM")
perf report -i "$DIR/perf.data" --stdio --no-children "${FILTER[@]}" \
	--sort comm,dso,sym --percent-limit 0.2 -g none \
	> "$DIR/report.txt" 2>/dev/null
if [ "$GRAPH" != none ]; then
	perf script -i "$DIR/perf.data" "${FILTER[@]}" -F comm,ip,sym 2>/dev/null |
	python3 -c '
import collections, sys
# perf script: a "comm" line, then one "ip sym" line per frame, leaf
# first, then a blank line.  Fold to "comm;root;...;leaf count".
stacks = collections.Counter()
comm, frames = None, []
def flush():
    if comm is not None and frames:
        stacks[";".join([comm] + frames[::-1])] += 1
for line in sys.stdin:
    line = line.rstrip("\n")
    if not line.strip():
        flush(); comm, frames = None, []
    elif not line[0].isspace():
        flush(); comm, frames = line.split()[0], []
    else:
        parts = line.split(None, 1)
        frames.append(parts[1].split("+")[0] if len(parts) > 1 else parts[0])
flush()
for k, v in stacks.most_common():
    print(k, v)
' > "$DIR/stacks.folded"
fi
grep -v '^#' "$DIR/report.txt" | grep -v '^$' | head -n "$TOP"
echo "perf-profile: $DIR/{perf.data,report.txt${GRAPH:+,stacks.folded}}" >&2
exit $rc
