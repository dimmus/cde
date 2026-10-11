#!/bin/bash
# CDE benchmarks: dtmail on big mailboxes: open, idle, sort, delete all.
#
#   dtmail-time.sh [-n "1000 10000"] [-w IDLE_S] [-A] [-S] [-o OUT.jsonl]
#
# For each size N (default "1000 10000"; 50000 works, slowly) it generates
# an mbox with mbox-gen.py (no Content-Length, 30% multipart), opens it
# with the uninstalled dtmail of this tree ($DTMAIL to override) on a
# private Xvfb with a private ttsession, and prints apptime.py's JSON:
#
#   dtmail-N          map_s (main window up), idle_s (dtmail stopped using
#                     CPU: the message list is filled), cpu_s, the
#                     preload's counts until then, and what dtmail does
#                     in the IDLE_S seconds (default 60; the TODO's 10
#                     minutes is -w 600) after: idle_wakeups, idle_cpu_s
#                     (and idle_syscalls with -S)
#   dtmail-N-sort-sender, -sort-size, -sort-date
#                     View > By Sender / By Size / By Date/Time
#   dtmail-N-select-all, dtmail-N-delete
#                     Message > Select All, then Message > Delete
#                     each: seconds from the last key to idle, cpu_s,
#                     mallocs, requests and round trips
#
# -A leaves the actions out, -S counts system calls (strace -f: the times
# of such a run mean nothing).  dtmail is stopped with SIGTERM; the mbox
# is checked afterwards: a size change means dtmail rewrote it.

set -u
. "$(dirname "$0")/cdeenv.sh"

SIZES="1000 10000" OUT= IDLE=60 ACTIONS=1 STRACE=
while getopts n:w:ASo: opt; do
	case $opt in
	n) SIZES=$OPTARG ;;
	w) IDLE=$OPTARG ;;
	A) ACTIONS= ;;
	S) STRACE=--strace ;;
	o) OUT=$OPTARG ;;
	*) echo "usage: $0 [-n SIZES] [-w IDLE_S] [-A] [-S] [-o OUT.jsonl]" >&2; exit 2 ;;
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

# Menu mnemonics of the C app-defaults (Dtmail*menubar*View.By_Sender...).
set --
if [ -n "$ACTIONS" ]; then
	set -- --action "sort-sender=alt+v s" --action "sort-size=alt+v z" \
	       --action "sort-date=alt+v d" --action "select-all=alt+m s" \
	       --action "delete=alt+m d"
fi

for n in $SIZES; do
	mbox=$WORK/mbox$n
	"$CDE_BENCH/mbox-gen.py" -n "$n" "$mbox"
	before=$(cksum < "$mbox")
	HOME=$WORK/home MAILRC=$WORK/home/.mailrc MAIL=$mbox "$CDE_BENCH/apptime.py" --xvfb --ttsession \
		--label "dtmail-$n" --name "Mailer - .*mbox$n" --idle-ms 2000 \
		--timeout 900 --idle-window "$IDLE" $STRACE "$@" -- \
		"$DTMAIL" -f "$mbox" | tee -a "${OUT:-/dev/null}"
	[ "$before" = "$(cksum < "$mbox")" ] ||
		echo "dtmail-time: dtmail rewrote $mbox" >&2
done
