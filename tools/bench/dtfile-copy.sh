#!/bin/bash
# CDE benchmarks: dtfile_copy (dtfile's folder copy) against cp.
#
#   dtfile-copy.sh [-s BIG_MB] [-f FILES] [-d DIR] [-S] [-o OUT.jsonl]
#
# Copies two trees with "cp -r" and with the uninstalled dtfile_copy of
# this tree ($DTFILE_COPY to override; it shows its progress dialog on
# $DISPLAY, or on a private Xvfb, and pops down 1 ms after the copy):
#   big    one file of BIG_MB MiB (default 1024: the TODO's 1 GB)
#   small  FILES files of 4 KiB in 100 folders (default 10000)
# under DIR (default $TMPDIR or /tmp; a tmpfs measures the copy loop, a
# disk the I/O too), and checks each copy.  One JSON object per copy:
# seconds, MB/s, user and system CPU seconds; dtfile_copy's rows have
# its startup (an empty folder copied the same way) subtracted, in
# startup_s.  -S adds the system calls (syscount.py: syscalls and
# top_syscalls, e.g. read/write against copy_file_range); the times of
# that run are not comparable.

set -u
. "$(dirname "$0")/cdeenv.sh"

BIG_MB=1024 FILES=10000 BASE=${TMPDIR:-/tmp} OUT= STRACE=
while getopts s:f:d:So: opt; do
	case $opt in
	s) BIG_MB=$OPTARG ;;
	f) FILES=$OPTARG ;;
	d) BASE=$OPTARG ;;
	S) STRACE=1 ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-s BIG_MB] [-f FILES] [-d DIR] [-S] [-o OUT.jsonl]" >&2; exit 2 ;;
	esac
done

COPY=${DTFILE_COPY:-$(cde_bin dtfile_copy)}
WORK=$(mktemp -d "$BASE/dtfilecopy.XXXXXX")
trap 'rm -rf "$WORK"; [ -n "${CDE_XVFB_PID:-}" ] && kill $CDE_XVFB_PID 2>/dev/null' EXIT
mkdir -p "$WORK/home" "$WORK/empty"

if [ -z "${DISPLAY:-}" ]; then
	n=$(cde_free_display)
	cde_xvfb "$n" || exit 1
	export DISPLAY=:$n
fi

mkdir -p "$WORK/big" "$WORK/small"
head -c $((BIG_MB * 1024 * 1024)) /dev/urandom > "$WORK/big/file"
python3 - "$WORK/small" "$FILES" <<'PY'
import os, sys
root, n = sys.argv[1], int(sys.argv[2])
blob = os.urandom(4096)
for i in range(n):
    d = os.path.join(root, "d%02d" % (i % 100))
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, "f%05d" % i), "wb") as f:
        f.write(blob)
PY

# seconds user system of a command, in $T
timed() {
	local TIMEFORMAT='%R %U %S'
	T=$( { time "$@" >/dev/null 2>&1; } 2>&1 | tail -1)
}

count() {	# command... -> syscall JSON
	"$CDE_BENCH/syscount.py" run --json -- "$@" 2>/dev/null | tail -1
}

dtcopy() {	# src dst
	HOME=$WORK/home "$COPY" -xrm "*popDown: True" -delay 1 "$1" "$2"
}

timed dtcopy "$WORK/empty" "$WORK/empty.copy"
read -r startup _ _ <<< "$T"

for tree in big small; do
	bytes=$(du -sb "$WORK/$tree" | cut -f1)
	for tool in cp dtfile_copy; do
		dst=$WORK/$tree.$tool
		rm -rf "$dst"
		sync
		if [ $tool = cp ]; then
			timed cp -r "$WORK/$tree" "$dst"
		else
			timed dtcopy "$WORK/$tree" "$dst"
		fi
		read -r s u y <<< "$T"
		diff -r "$WORK/$tree" "$dst" >/dev/null 2>&1 || {
			echo "dtfile-copy: the $tool copy of $tree differs" >&2
			continue
		}
		sys=
		if [ -n "$STRACE" ]; then
			rm -rf "$dst"
			if [ $tool = cp ]; then
				sys=$(count cp -r "$WORK/$tree" "$dst")
			else
				sys=$(count env HOME="$WORK/home" "$COPY" \
				      -xrm "*popDown: True" -delay 1 "$WORK/$tree" "$dst")
			fi
		fi
		python3 - "$tree" "$tool" "$bytes" "$s" "$u" "$y" "$startup" "$sys" <<'PY' | tee -a "${OUT:-/dev/null}"
import json, sys
tree, tool, b, s, u, y, startup, sysj = sys.argv[1:9]
b, s, u, y, startup = int(b), float(s), float(u), float(y), float(startup)
r = {"bench": "dtfile-copy", "name": "copy-%s-%s" % (tree, tool),
     "bytes": b, "user_s": u, "sys_s": y}
if tool == "dtfile_copy":
    r["startup_s"] = startup
    s = max(s - startup, 0.0)
r["seconds"] = round(s, 3)
r["mb_per_s"] = round(b / 1048576 / s, 1) if s > 0 else None
if sysj:
    j = json.loads(sysj)
    r["syscalls"] = j["syscalls"]
    r["top_syscalls"] = dict(list(j["by_syscall"].items())[:8])
print(json.dumps(r))
PY
		rm -rf "$dst"
	done
done
