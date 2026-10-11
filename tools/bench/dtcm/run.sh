#!/bin/bash
#
# CDE benchmarks: dtcm, libcsa and rpc.cmsd, end to end.
#
# Licensed under the LGPL 2.1 license.
#
#   run.sh [-o OUT.json] [-w WORKDIR] [-n ONETIME] [-R REPEATING]
#          [-r REPEAT] [-x] [-G] [-S]
#
#   -o  the JSON result (default WORKDIR/dtcm.json)
#   -w  work directory: calendars, logs, strace output
#       (default: a new mktemp -d directory)
#   -n, -R  the calendar: one-time and repeating appointments
#       (default 5000 and 200)
#   -r  runs per case, median (default 5)
#   -x  use an already running rpc.cmsd -d (started as root, see
#       cmsd.sh): $DTCMBENCH_CMSD_LOG is its stderr, $DTCMBENCH_CMSD_PID
#       its pid (optional); the calendar must not exist yet
#   -G  no dtcm (GUI) part
#   -S  no strace part
#
# What it does:
#   1. builds dtcmbench (and ../libcdebench_preload.so);
#   2. starts a private rpc.cmsd (cmsd.sh) unless -x;
#   3. populates the calendar (dtcmbench populate) and times it;
#   4. runs the lookup cases (dtcmbench bench): month, week, year, Find
#      "all" the current and the old way; time, client CPU, mallocs,
#      RPCs (from rpc.cmsd's -d log), rpc.cmsd CPU, all per operation;
#   5. counts, with strace, the syscalls and the /etc/localtime accesses
#      (open and stat) of each case, client and rpc.cmsd, minus the
#      logon baseline (the "noop" case);
#   6. starts dtcm on Xvfb with the calendar: time to the first window
#      and to idle (../apptime.py), RPCs and /etc/localtime accesses at
#      startup, then 10 appointments added and deleted on the day shown
#      (20 changes), with dtcm's RPCs, X requests, mallocs and CPU per
#      change.
# The client and rpc.cmsd are pinned to one CPU when taskset exists, as
# ../bench.py does.

set -e
HERE=$(cd "$(dirname "$0")" && pwd)
. "$HERE/../cdeenv.sh"

OUT= WORK= NONCE=5000 NREP=200 REPEAT=5 EXTERNAL= GUI=1 STRACE=1
while getopts "o:w:n:R:r:xGS" opt; do
	case $opt in
	o) OUT=$OPTARG ;;
	w) WORK=$OPTARG ;;
	n) NONCE=$OPTARG ;;
	R) NREP=$OPTARG ;;
	r) REPEAT=$OPTARG ;;
	x) EXTERNAL=1 ;;
	G) GUI= ;;
	S) STRACE= ;;
	*) sed -n '3,25p' "$0" >&2; exit 2 ;;
	esac
done
[ -n "$WORK" ] || WORK=$(mktemp -d "${TMPDIR:-/tmp}/dtcmbench.XXXXXX")
mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)
: "${OUT:=$WORK/dtcm.json}"
command -v strace >/dev/null || STRACE=
log() { echo "run.sh: $*" >&2; }

make -s -C "$HERE/.." libcdebench_preload.so
make -s -C "$HERE"
BENCH=$HERE/dtcmbench
PRELOAD=$HERE/../libcdebench_preload.so
PIN=
command -v taskset >/dev/null && PIN="taskset -c 0"

XVFB_PID=
cleanup() {
	[ -n "$XVFB_PID" ] && kill "$XVFB_PID" 2>/dev/null
	[ -z "$EXTERNAL" ] && "$HERE/cmsd.sh" stop "$WORK" 2>/dev/null
	true
}
trap cleanup EXIT

if [ -z "$EXTERNAL" ]; then
	"$HERE/cmsd.sh" start "$WORK"
	export DTCMBENCH_CMSD_LOG=$WORK/cmsd.log
	export DTCMBENCH_CMSD_PID=$(cat "$WORK/cmsd.pid")
fi
[ -n "$DTCMBENCH_CMSD_PID" ] && [ -n "$PIN" ] &&
	taskset -pc 0 "$DTCMBENCH_CMSD_PID" >/dev/null
rpcs() { grep -c "_svc called" "$DTCMBENCH_CMSD_LOG" 2>/dev/null || echo 0; }

log "populating: $NONCE one-time, $NREP repeating"
$PIN "$BENCH" populate -n "$NONCE" -R "$NREP" | tee "$WORK/populate.txt"

log "lookups, median of $REPEAT"
LD_PRELOAD=$PRELOAD $PIN "$BENCH" bench -r "$REPEAT" -j "$WORK/bench.json" \
	| tee "$WORK/bench.txt"

# strace: syscalls and /etc/localtime accesses per case
: > "$WORK/strace.txt"
if [ -n "$STRACE" ]; then
	for c in noop month week year find_all find_rare find_old; do
		if [ -n "$DTCMBENCH_CMSD_PID" ]; then
			strace -f -qq -o "$WORK/st-server-$c" \
				-p "$DTCMBENCH_CMSD_PID" &
			SPID=$!
			sleep 0.3
		fi
		strace -f -qq -o "$WORK/st-client-$c" \
			"$BENCH" bench -r 1 "$c" >/dev/null 2>&1
		if [ -n "$DTCMBENCH_CMSD_PID" ]; then
			sleep 0.2
			kill -INT "$SPID"
			wait "$SPID" 2>/dev/null || true
		fi
		for side in client server; do
			f=$WORK/st-$side-$c
			[ -f "$f" ] || continue
			echo "$c $side $(grep -vc -e '^[0-9]* *+++' -e '^[0-9]* *---' "$f")" \
			     "$(grep -c '"/etc/localtime"' "$f")" \
			     "$(grep -c 'open.*"/etc/localtime"' "$f")"
		done >> "$WORK/strace.txt"
	done
fi

# dtcm on Xvfb
: > "$WORK/gui.txt"
if [ -n "$GUI" ] && command -v Xvfb >/dev/null &&
   command -v xdotool >/dev/null && DTCM=$(cde_bin dtcm); then
	# the RPCs of "dtcmbench change" itself, without a dtcm to call back
	b0=$(rpcs)
	"$BENCH" change -n 10 -i 0 >/dev/null
	b1=$(rpcs)
	D=$(cde_free_display)
	cde_xvfb "$D"
	XVFB_PID=$CDE_XVFB_PID
	export DISPLAY=:$D
	# startup time (no strace)
	r0=$(rpcs)
	python3 "$HERE/../apptime.py" --name Calendar --idle-ms 1000 \
		--label dtcm -- "$DTCM" > "$WORK/apptime.json" 2>/dev/null ||
		log "apptime.py failed"
	r1=$(rpcs)
	# startup and updates, with counters (and strace when available)
	ST=
	[ -n "$STRACE" ] && ST="strace -f -qq -o $WORK/st-dtcm"
	rm -f "$WORK/dtcm-report.txt"
	r2=$(rpcs)
	CDEBENCH_REPORT=$WORK/dtcm-report.txt CDEBENCH_REPORT_SIGTERM=1 \
		LD_PRELOAD=$PRELOAD $ST "$DTCM" 2>/dev/null &
	GPID=$!
	xdotool search --sync --onlyvisible --name Calendar >/dev/null
	sleep 1.5
	r3=$(rpcs)
	# dtcm is strace's child, or $GPID itself: not just any dtcm
	DPID=$(pgrep -n -x -P "$GPID" dtcm || echo "$GPID")
	st0=0
	[ -n "$ST" ] && st0=$(grep -c '"/etc/localtime"' "$WORK/st-dtcm")
	c0=$(awk '{print $14 + $15}' "/proc/$DPID/stat")
	"$BENCH" change -n 10 -i 300 > "$WORK/change.txt"
	sleep 1
	c1=$(awk '{print $14 + $15}' "/proc/$DPID/stat")
	r4=$(rpcs)
	st1=0
	[ -n "$ST" ] && st1=$(grep -c '"/etc/localtime"' "$WORK/st-dtcm")
	# the counters at the change start: a second report is not
	# possible, so run the same session without changes for the base
	kill "$DPID" 2>/dev/null || true
	wait "$GPID" 2>/dev/null || true
	rep_upd=$(grep "comm=dtcm" "$WORK/dtcm-report.txt" | tail -1)
	rm -f "$WORK/dtcm-report.txt"
	CDEBENCH_REPORT=$WORK/dtcm-report.txt CDEBENCH_REPORT_SIGTERM=1 \
		LD_PRELOAD=$PRELOAD "$DTCM" 2>/dev/null &
	GPID=$!
	xdotool search --sync --onlyvisible --name Calendar >/dev/null
	sleep 1.5
	kill "$GPID" 2>/dev/null || true
	wait "$GPID" 2>/dev/null || true
	rep_base=$(grep "comm=dtcm" "$WORK/dtcm-report.txt" | tail -1)
	echo "startup_rpcs $((r1 - r0))" >> "$WORK/gui.txt"
	echo "startup_localtime $st0" >> "$WORK/gui.txt"
	# the bench's own RPCs: logon/logoff and 20 changes; the rest is dtcm
	echo "change_rpcs_all $((r4 - r3))" >> "$WORK/gui.txt"
	echo "change_rpcs_bench $((b1 - b0))" >> "$WORK/gui.txt"
	echo "change_cpu_ticks $((c1 - c0))" >> "$WORK/gui.txt"
	echo "change_localtime $((st1 - st0))" >> "$WORK/gui.txt"
	echo "report_updates $rep_upd" >> "$WORK/gui.txt"
	echo "report_base $rep_base" >> "$WORK/gui.txt"
	kill "$XVFB_PID" 2>/dev/null || true
	XVFB_PID=
fi

python3 - "$WORK" "$OUT" <<'EOF'
import json, os, re, sys
work, out = sys.argv[1], sys.argv[2]
res = json.load(open(os.path.join(work, "bench.json")))
pop = open(os.path.join(work, "populate.txt")).read().strip()
res["populate"] = pop
m = re.search(r"in ([0-9.]+) s", pop)
if m:
    res["populate_s"] = float(m.group(1))
m = re.search(r"([0-9]+) RPCs", pop)
if m:
    res["populate_rpcs"] = int(m.group(1))
st = {}
for line in open(os.path.join(work, "strace.txt")):
    c, side, n, lt, lto = line.split()
    st.setdefault(c, {})[side] = (int(n), int(lt), int(lto))
if st:
    base = st.get("noop", {})
    res["strace"] = {}
    for c, sides in st.items():
        if c == "noop":
            continue
        d = {}
        for side, (n, lt, lto) in sides.items():
            b = base.get(side, (0, 0, 0))
            d[side + "_syscalls"] = n - b[0]
            d[side + "_localtime_accesses"] = lt - b[1]
            d[side + "_localtime_opens"] = lto - b[2]
        res["strace"][c] = d
    res["strace_note"] = ("per run of the case, minus the logon baseline "
                          "(noop case); accesses = open + stat")
gui = {}
for line in open(os.path.join(work, "gui.txt")):
    k, _, v = line.strip().partition(" ")
    gui[k] = v
if gui:
    g = {"startup_rpcs": int(gui["startup_rpcs"]),
         "startup_localtime_accesses": int(gui["startup_localtime"]),
         "changes": 20,
         "rpcs_per_change": (int(gui["change_rpcs_all"]) -
                             int(gui["change_rpcs_bench"])) / 20.0,
         "cpu_ms_per_change": int(gui["change_cpu_ticks"]) * 10 / 20.0,
         "localtime_accesses_per_change":
             int(gui["change_localtime"]) / 20.0}
    def rep(s):
        return {k: int(v) for k, v in re.findall(r"(\w+)=(\d+)", s)}
    u, b = rep(gui.get("report_updates", "")), rep(gui.get("report_base", ""))
    for k in ("mallocs", "requests", "rtrips"):
        if k in u and k in b:
            g[k + "_per_change"] = (u[k] - b[k]) / 20.0
    try:
        g["apptime"] = json.load(open(os.path.join(work, "apptime.json")))
    except Exception:
        pass
    res["gui"] = g
json.dump(res, open(out, "w"), indent=2)
print("run.sh: wrote " + out)
EOF
