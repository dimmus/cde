#!/bin/bash
# CDE benchmarks: dtmail time to open big mailboxes.
#
#   dtmail-time.sh [-n "1000 10000"] [-o OUT.jsonl]
#
# For each size N (default "1000 10000"; 50000 works, slowly) it generates
# an mbox with mbox-gen.py (no Content-Length, 30% multipart), opens it
# with the uninstalled dtmail of this tree ($DTMAIL to override) on a
# private Xvfb with a private ttsession, and prints apptime.py's JSON:
# map_s (main window up), idle_s (dtmail stopped using CPU: the message
# list is filled) and cpu_s.  dtmail is stopped with SIGTERM, so the
# preload's counters are not reported.
# The mbox is checked afterwards: a size change means dtmail rewrote it.

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

DTMAIL=${DTMAIL:-$(cde_bin dtmail)}
WORK=$(mktemp -d "${TMPDIR:-/tmp}/dtmailbench.XXXXXX")
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/home"
# Without it dtmail stops at a dialog about the mail spool's group
# permissions.  dtmail reads $MAILRC, else the .mailrc of the passwd home
# directory (not $HOME's).
echo "set __ignore_group_permissions" > "$WORK/home/.mailrc"

for n in $SIZES; do
	mbox=$WORK/mbox$n
	"$CDE_BENCH/mbox-gen.py" -n "$n" "$mbox"
	before=$(cksum < "$mbox")
	HOME=$WORK/home MAILRC=$WORK/home/.mailrc MAIL=$mbox "$CDE_BENCH/apptime.py" --xvfb --ttsession \
		--label "dtmail-$n" --name "Mailer - .*mbox$n" --idle-ms 2000 \
		--timeout 900 -- "$DTMAIL" -f "$mbox" | tee -a "${OUT:-/dev/null}"
	[ "$before" = "$(cksum < "$mbox")" ] ||
		echo "dtmail-time: dtmail rewrote $mbox" >&2
done
