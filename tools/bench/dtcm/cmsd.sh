#!/bin/bash
#
# CDE benchmarks: a private rpc.cmsd for the dtcm benchmarks.
#
# Licensed under the LGPL 2.1 license.
#
#   cmsd.sh start WORKDIR    start the tree's rpc.cmsd -d, its calendars in
#                            WORKDIR/spool, its log in WORKDIR/cmsd.log,
#                            its pid in WORKDIR/cmsd.pid
#   cmsd.sh stop WORKDIR     stop it, unregister it from rpcbind and
#                            remove WORKDIR/spool
#
# rpc.cmsd insists on running as root, keeps its calendars in the fixed
# /var/spool/calendar and registers program 100068 with rpcbind.  As an
# ordinary user it is run in a user namespace: root inside is the user
# outside, uids 1-65536 are the user's subordinate ids (/etc/subuid, for
# the "daemon" uid rpc.cmsd switches to), and in a private mount
# namespace WORKDIR/spool is bind-mounted over /var/spool/calendar.
# rpcbind (which must be running: systemctl start rpcbind) accepts the
# registration from the user over its local socket, and clients talk to
# it as to any rpc.cmsd.
#
# Without user namespaces or subordinate ids, run it as root instead:
#   sudo mkdir -p /var/spool/calendar
#   sudo env LD_LIBRARY_PATH=$CDE_TOP/lib/csa/.libs \
#        $CDE_TOP/programs/dtcm/server/.libs/rpc.cmsd -d 2>cmsd.log &
# and point DTCMBENCH_CMSD_LOG at cmsd.log (run.sh -x skips cmsd.sh).

set -e
. "$(dirname "$0")/../cdeenv.sh"

CMSD_PROG=100068

die() { echo "cmsd.sh: $*" >&2; exit 1; }

registered() { rpcinfo -p 2>/dev/null | awk '$1 == 100068' | grep -q .; }

subid() {
	# first subordinate range of the user: "start count"
	awk -F: -v u="$(id -un)" -v n="$(id -u)" \
		'($1 == u || $1 == n) { print $2, $3; exit }' "$1" 2>/dev/null
}

start() {
	local w=$1 bin uid gid
	command -v rpcinfo >/dev/null || die "rpcinfo not found"
	rpcinfo -p >/dev/null 2>&1 || die "rpcbind is not running"
	if registered; then
		die "an rpc.cmsd is already registered with rpcbind" \
		    "(rpcinfo -p | grep 100068); stop it first"
	fi
	bin=$(cde_bin rpc.cmsd) || die "rpc.cmsd not built"
	mkdir -p "$w"
	w=$(cd "$w" && pwd)
	mkdir -p "$w/spool"
	chmod 1777 "$w/spool"
	: > "$w/cmsd.log"

	if [ "$(id -u)" = 0 ]; then
		unshare -m sh -c "mount --bind '$w/spool' /var/spool/calendar \
			&& exec '$bin' -d" </dev/null >>"$w/cmsd.log" 2>&1 &
	else
		read -r uid ucnt <<<"$(subid /etc/subuid)"
		read -r gid gcnt <<<"$(subid /etc/subgid)"
		[ -n "$uid" ] && [ -n "$gid" ] ||
			die "no subordinate ids for $(id -un) in /etc/subuid" \
			    "and /etc/subgid; see the comment at the top"
		[ "$ucnt" -gt 65536 ] && ucnt=65536
		[ "$gcnt" -gt 65536 ] && gcnt=65536
		unshare --map-users="1:$uid:$ucnt" \
			--map-groups="1:$gid:$gcnt" -r -m \
			sh -c "mount --bind '$w/spool' /var/spool/calendar \
			&& exec '$bin' -d" </dev/null >>"$w/cmsd.log" 2>&1 &
	fi
	echo $! > "$w/cmsd.pid"
	for _ in $(seq 100); do
		if rpcinfo -t localhost $CMSD_PROG 5 >/dev/null 2>&1; then
			return 0
		fi
		kill -0 "$(cat "$w/cmsd.pid")" 2>/dev/null || break
		sleep 0.05
	done
	cat "$w/cmsd.log" >&2
	stop "$w"
	die "rpc.cmsd did not start"
}

stop() {
	local w=$1 pid uid gid
	if [ -f "$w/cmsd.pid" ]; then
		pid=$(cat "$w/cmsd.pid")
		kill "$pid" 2>/dev/null || true
		for _ in $(seq 40); do
			kill -0 "$pid" 2>/dev/null || break
			sleep 0.05
		done
		kill -9 "$pid" 2>/dev/null || true
		rm -f "$w/cmsd.pid"
	fi
	for v in 2 3 4 5; do
		rpcinfo -d $CMSD_PROG $v 2>/dev/null || true
	done
	if [ -d "$w/spool" ]; then
		# the calendars belong to subordinate ids: remove them from
		# inside the namespace
		read -r uid ucnt <<<"$(subid /etc/subuid)"
		read -r gid gcnt <<<"$(subid /etc/subgid)"
		if [ "$(id -u)" != 0 ] && [ -n "$uid" ] && [ -n "$gid" ]; then
			[ "$ucnt" -gt 65536 ] && ucnt=65536
			[ "$gcnt" -gt 65536 ] && gcnt=65536
			unshare --map-users="1:$uid:$ucnt" \
				--map-groups="1:$gid:$gcnt" -r \
				rm -rf "$w/spool" 2>/dev/null || true
		fi
		rm -rf "$w/spool" 2>/dev/null || true
	fi
}

case $1 in
start) [ -n "$2" ] || die "usage: cmsd.sh start WORKDIR"; start "$2" ;;
stop)  [ -n "$2" ] || die "usage: cmsd.sh stop WORKDIR"; stop "$2" ;;
*)     die "usage: cmsd.sh start|stop WORKDIR" ;;
esac
