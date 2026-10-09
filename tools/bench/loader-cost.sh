#!/bin/bash
# CDE benchmarks: dynamic loader cost per program and library.
#
#   loader-cost.sh [-o OUT.jsonl] [PROGRAM...]
#
# For each uninstalled binary of this tree (default: every
# programs/*/.libs/* and programs/*/*/.libs/* ELF executable, plus
# lib/*/.libs/*.so) prints one JSON object with
#   dsos         shared objects loaded (ldd)
#   unused       direct dependencies nothing uses (ldd -u -r)
#   exported     dynamic symbols defined (nm -D --defined-only)
#   relocations  relocations processed at startup and
#   lookups      symbol lookups, from LD_DEBUG=statistics (programs only;
#                the program runs with --help-like arguments that make
#                most CDE programs exit, under a 5 s timeout, without X)
# so that CI can track them.

set -u
. "$(dirname "$0")/cdeenv.sh"

OUT=
while getopts o: opt; do
	case $opt in
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-o OUT.jsonl] [PROGRAM...]" >&2; exit 2 ;;
	esac
done
shift $((OPTIND - 1))

if [ $# -eq 0 ]; then
	set -- $(for f in "$CDE_TOP"/programs/*/.libs/* "$CDE_TOP"/programs/*/*/.libs/* \
			  "$CDE_TOP"/lib/*/.libs/*.so "$CDE_TOP"/lib/tt/lib/.libs/*.so; do
		[ -f "$f" ] && [ ! -L "$f" ] && head -c 4 "$f" 2>/dev/null | grep -q ELF && echo "$f"
	done)
fi

for f in "$@"; do
	dsos=$(ldd "$f" 2>/dev/null | grep -c '=>')
	unused=$(ldd -u -r "$f" 2>/dev/null | grep -c '^\s*/')
	exported=$(nm -D --defined-only "$f" 2>/dev/null | wc -l)
	rel= look=
	case $f in
	*.so) ;;
	*)
		stats=$(timeout 5 env -u DISPLAY LD_DEBUG=statistics "$f" \
			-cdebench-no-such-option </dev/null 2>&1 >/dev/null)
		rel=$(echo "$stats" | awk '$2 == "number" && $4 == "relocations:" { print $NF; exit }')
		look=$(echo "$stats" | awk '$2 == "number" && $5 == "from" { print $NF; exit }')
		;;
	esac
	printf '{"bench": "loader", "name": "%s", "dsos": %s, "unused": %s, "exported": %s%s}\n' \
		"${f#"$CDE_TOP"/}" "$dsos" "$unused" "$exported" \
		"${rel:+, \"relocations\": $rel, \"relocations_from_cache\": $look}" |
		tee -a "${OUT:-/dev/null}"
done
