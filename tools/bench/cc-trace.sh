#!/bin/bash
# CDE benchmarks: a compiler wrapper that records per-TU build times.
#
#   make CC="tools/bench/cc-trace.sh gcc" CXX="tools/bench/cc-trace.sh g++"
#   tools/bench/cc-trace2json.py $CC_TRACE_LOG > build-trace.json
#
# Each compiler run appends "<start_ns> <end_ns> <pid> <output file>" to
# $CC_TRACE_LOG (default /tmp/cc-trace.log); cc-trace2json.py turns the
# log into Chrome trace JSON (chrome://tracing, ui.perfetto.dev), one lane
# per concurrent job.

log=${CC_TRACE_LOG:-/tmp/cc-trace.log}
out=
prev=
for a in "$@"; do
	[ "$prev" = -o ] && out=$a
	prev=$a
done
start=$(date +%s%N)
"$@"
rc=$?
echo "$start $(date +%s%N) $$ ${out:-$(basename "$1")}" >> "$log"
exit $rc
