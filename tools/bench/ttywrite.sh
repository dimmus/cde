#!/bin/bash
# CDE benchmarks: run a command inside a terminal emulator, timed from
# the moment the terminal is idle to the moment it has drawn all of the
# command's output.
#
#   dtterm -e tools/bench/ttywrite.sh RESULT COMMAND [ARG...]
#
# Both ends are a cursor position report (DSR, ESC [ 6 n): the terminal
# answers once it has processed everything written before.  With
# CDEBENCH_REPORT_SIGNAL set, the terminal (our parent) is sent that
# signal at both ends, so that libcdebench_preload.so appends its counts
# then (event=signal lines): their difference is the output's own cost,
# without the terminal's startup.  Appends "T0 T1" (ns, CLOCK_REALTIME)
# to RESULT.

res=$1
shift
sync_tty() {
	printf '\033[6n' > /dev/tty
	IFS= read -r -s -d R _ < /dev/tty
}
snap() {
	if [ -n "${CDEBENCH_REPORT_SIGNAL:-}" ]; then
		kill -"$CDEBENCH_REPORT_SIGNAL" "$PPID" 2>/dev/null
		sleep 0.2
	fi
}
saved=$(stty -g < /dev/tty)
stty -icanon -echo < /dev/tty
sync_tty
snap
t0=$(date +%s%N)
"$@"
sync_tty
t1=$(date +%s%N)
snap
stty "$saved" < /dev/tty
echo "$t0 $t1" > "$res"
