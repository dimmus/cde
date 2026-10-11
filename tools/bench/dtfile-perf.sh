#!/bin/bash
# CDE benchmarks: dtfile's own -DDT_PERFORMANCE timers.
#
#   dtfile-perf.sh [-n "1000 10000"] [-o OUT.jsonl]
#
# Builds a dtfile with -DDT_PERFORMANCE from the sources and flags of
# this tree into a temporary directory (perfbuild.py; the tree is not
# touched, and dtperf-shim.h makes the bit-rotted timer code compile),
# opens a generated folder of each size N with it, as dtfile-time.sh
# does, and prints one JSON object per size with the phases it times,
# in seconds (repeated phases summed): XtInitialize, DtDbLoad, Setup
# Desktop, Bring up View, the folder read, UpdateFileIcons,
# LayoutFileIcons, DisplaySomeIcons...  The timers print "%ld.%ld" of
# seconds and microseconds (2.13719 is 2.013719 s); they are decoded so.
# dtfile's output is line-buffered (stdbuf -oL): its forked children
# would otherwise print again what was still in the buffer at the fork.

set -u
. "$(dirname "$0")/cdeenv.sh"

SIZES="1000 10000" OUT=
while getopts n:o: opt; do
	case $opt in
	n) SIZES=$OPTARG ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-n SIZES] [-o OUT.jsonl]" >&2; exit 2 ;;
	esac
done

WORK=$(mktemp -d "${TMPDIR:-/tmp}/dtfileperf.XXXXXX")
trap 'chmod -R u+rwx "$WORK" 2>/dev/null; rm -rf "$WORK"' EXIT
mkdir -p "$WORK/home"
DTFILE=$("$CDE_BENCH/perfbuild.py" -D DT_PERFORMANCE \
	--include "$CDE_BENCH/dtperf-shim.h" -o "$WORK/build" \
	"$CDE_TOP/programs/dtfile" dtfile 2>"$WORK/build.log" | tail -1)
if [ ! -x "$DTFILE" ]; then
	echo "dtfile-perf: the DT_PERFORMANCE build failed:" >&2
	tail -20 "$WORK/build.log" >&2
	exit 1
fi

for n in $SIZES; do
	dir=$WORK/folder$n
	"$CDE_BENCH/dtfile-gendir.py" -n "$n" "$dir" >/dev/null
	HOME=$WORK/home "$CDE_BENCH/apptime.py" --xvfb --ttsession --no-preload \
		--label "dtfile-perf-$n" --name "folder$n" --idle-ms 1500 \
		--timeout 600 --log "$WORK/log$n" -- \
		stdbuf -oL "$DTFILE" -xrm "*retryLoadDesktop: 0" -dir "$dir" > "$WORK/app$n"
	python3 - "$WORK/app$n" "$WORK/log$n" <<'PY' | tee -a "${OUT:-/dev/null}"
import json, re, sys
res = json.load(open(sys.argv[1]))
phases = {}
for line in open(sys.argv[2], errors="replace"):
    m = re.search(r'^\s*(?:done |finished reading: )?(.*?),? time: (\d+)\.(\d+)', line)
    if not m:
        continue
    name = m.group(1).strip() or "?"
    if line.lstrip().startswith("finished reading"):
        name = "read directory"
    name = re.sub(r'^Part (\d)', r'part \1', name)
    phases[name] = round(phases.get(name, 0) + int(m.group(2)) +
                         int(m.group(3)) / 1e6, 6)
res["bench"] = "dtfile-perf"
res["phases_s"] = phases
print(json.dumps(res))
PY
done
