# CDE benchmarks: CDE_STARTUP_TRACE stamps, for ksh93 scripts (Xsession,
# Xstartup, dtdbcache wrappers...).  Source it, then
#
#     cde_startup_trace Xsession start
#
# appends "<seconds.nanoseconds> <pid> <component> <event>" to the file
# named by CDE_STARTUP_TRACE, like startup-trace.h does from C, without a
# fork: ksh93's printf %(...)T formats the time itself.  Note that this is
# CLOCK_REALTIME, not CLOCK_MONOTONIC: startup-trace.py aligns the two
# clocks with a "clock realtime=<s> monotonic=<s>" line, which
# startup-trace.py --stamp-clock writes (run it once at the start, for
# example from dtlogin's Xsetup).
#
# Under a POSIX sh without %(...)T it falls back to date(1), which forks.

cde_startup_trace() {
	[ -n "${CDE_STARTUP_TRACE:-}" ] || return 0
	typeset now
	now=$(printf '%(%s.%N)T' now 2>/dev/null) || now=$(date +%s.%N)
	print -r -- "$now $$ $1 ${2:-} realtime" >> "$CDE_STARTUP_TRACE"
}
