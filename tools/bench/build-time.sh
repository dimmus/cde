#!/bin/bash
# CDE benchmarks: build times.
#
#   build-time.sh [-t TREE] [-j JOBS] [-c] [-T] [-R] [-k] [-o OUT.jsonl]
#                 [-- CONFIGURE_ARGS...]
#
# Times, in a tree of its own (by default a "git clone --shared" of this
# repository's HEAD in a temporary directory: committed work only, and
# nothing in this checkout is touched; -t TREE uses an existing one,
# which it builds from scratch, so never point it at a tree in use):
#   configure      ./autogen.sh CONFIGURE_ARGS (autoreconf and configure;
#                  default --disable-docs)
#   clean          make -jJOBS from nothing (JOBS default: nproc)
#   noop           make -jJOBS again, nothing to do
#   config_status  ./config.status && make -jJOBS, with the objects it
#                  rebuilds (objects_rebuilt)
# and, on request:
#   -T  the clean build through cc-trace.sh: per-TU times as Chrome trace
#       JSON (trace.json next to the output, or in the tree) and the
#       parallelism summary of cc-trace2json.py (tu, cpu_s, parallelism)
#   -R  the config_status rebuild under "remake --profile" (callgrind
#       files of make's own work), when remake is installed
#   -c  ccache across two trees: the clean build of a second clone in
#       another directory with a ccache filled by the first (CC="ccache
#       cc", CCACHE_BASEDIR so that paths do not spoil the hits), and its
#       hit rate (ccache -s): ccache_second_s, ccache_hits, ccache_misses
#   -k  keep the tree(s)
# One JSON object per step on stdout (and appended to OUT).

set -u
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(git -C "$HERE" rev-parse --show-toplevel)
COMMIT=$(git -C "$HERE" rev-parse HEAD)

TREE= JOBS=$(nproc) CCACHE= TRACE= REMAKE= KEEP= OUT=
while getopts t:j:cTRko: opt; do
	case $opt in
	t) TREE=$OPTARG ;;
	j) JOBS=$OPTARG ;;
	c) CCACHE=1 ;;
	T) TRACE=1 ;;
	R) REMAKE=1 ;;
	k) KEEP=1 ;;
	o) OUT=$OPTARG ;;
	*) sed -n '2,34p' "$0" >&2; exit 2 ;;
	esac
done
shift $((OPTIND - 1))
[ "${1:-}" = -- ] && shift
[ $# -gt 0 ] || set -- --disable-docs

WORK=$(mktemp -d "${TMPDIR:-/tmp}/cdebuild.XXXXXX")
cleanup() { [ -n "$KEEP" ] || rm -rf "$WORK"; }
trap cleanup EXIT

emit() {	# name seconds [extra JSON members]
	printf '{"bench": "build", "name": "%s", "seconds": %s%s}\n' \
		"$1" "$2" "${3:+, $3}" | tee -a "${OUT:-/dev/null}"
}

now() { date +%s.%N; }
since() { awk -v a="$1" -v b="$(now)" 'BEGIN { printf "%.2f", b - a }'; }

clone() {	# dir
	git clone -q --shared --no-checkout "$REPO" "$1" &&
	git -C "$1" checkout -q --detach "$COMMIT"
}

if [ -z "$TREE" ]; then
	TREE=$WORK/tree
	clone "$TREE" || exit 1
fi
cd "$TREE" || exit 1

cc=cc cxx=c++
if [ -n "$CCACHE" ]; then
	command -v ccache >/dev/null || { echo "build-time: no ccache" >&2; exit 1; }
	export CCACHE_DIR=$WORK/ccache CCACHE_BASEDIR=$WORK
	ccache -z >/dev/null
	cc="ccache cc" cxx="ccache c++"
fi
if [ -n "$TRACE" ]; then
	export CC_TRACE_LOG=$WORK/cc-trace.log
	: > "$CC_TRACE_LOG"
	cc="$HERE/cc-trace.sh $cc" cxx="$HERE/cc-trace.sh $cxx"
fi

t=$(now)
./autogen.sh "$@" CC="$cc" CXX="$cxx" > "$WORK/configure.log" 2>&1 || {
	echo "build-time: configure failed, see $WORK/configure.log" >&2
	KEEP=1; exit 1; }
emit configure "$(since "$t")"

t=$(now)
make -j"$JOBS" > "$WORK/make.log" 2>&1 || {
	echo "build-time: make failed, see $WORK/make.log" >&2; KEEP=1; exit 1; }
extra="\"jobs\": $JOBS"
if [ -n "$TRACE" ]; then
	trace=${OUT:+$(dirname "$OUT")/}trace.json
	"$HERE/cc-trace2json.py" "$CC_TRACE_LOG" > "$trace" 2> "$WORK/trace.txt"
	extra="$extra, \"trace\": \"$trace\", \"trace_summary\": \"$(tr '\n"' "; " < "$WORK/trace.txt")\""
fi
emit clean "$(since "$t")" "$extra"

t=$(now)
make -j"$JOBS" > "$WORK/noop.log" 2>&1
emit noop "$(since "$t")"

touch "$WORK/marker"
sleep 1
t=$(now)
if [ -n "$REMAKE" ] && command -v remake >/dev/null; then
	./config.status > /dev/null 2>&1 &&
	remake --profile -j"$JOBS" > "$WORK/remake.log" 2>&1
	prof=$(ls callgrind.out.* 2>/dev/null | wc -l)
	extra="\"remake_profiles\": $prof"
else
	[ -n "$REMAKE" ] && echo "build-time: remake is not installed" >&2
	./config.status > /dev/null 2>&1 && make -j"$JOBS" > "$WORK/cs.log" 2>&1
	extra=
fi
n=$(find . -name '*.o' -newer "$WORK/marker" | wc -l)
emit config_status "$(since "$t")" "\"objects_rebuilt\": $n${extra:+, $extra}"

if [ -n "$CCACHE" ]; then
	second=$WORK/tree2
	clone "$second" || exit 1
	ccache -z >/dev/null
	cd "$second" || exit 1
	t=$(now)
	{ ./autogen.sh "$@" CC="ccache cc" CXX="ccache c++" &&
	  make -j"$JOBS"; } > "$WORK/second.log" 2>&1 || {
		echo "build-time: the second build failed, see $WORK/second.log" >&2
		KEEP=1; exit 1; }
	s=$(since "$t")
	hits=$(ccache --print-stats 2>/dev/null | awk '$1 ~ /direct_cache_hit|preprocessed_cache_hit/ { h += $2 } END { print h + 0 }')
	miss=$(ccache --print-stats 2>/dev/null | awk '$1 == "cache_miss" { print $2 + 0 }')
	emit ccache_second "$s" "\"ccache_hits\": $hits, \"ccache_misses\": ${miss:-0}"
fi
