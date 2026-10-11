#!/bin/sh
#
# dtrandr against real X servers: Xvfb, and Xephyr nested in that Xvfb.
# Xvfb's RandR has one output with one mode and changes nothing but the
# primary output, but it keeps an EDID property a client sets, which
# makes it look as if another monitor was plugged in (for --daemon).
# Xephyr has many modes.  Both run with -noreset (else they reset, and
# forget all that, whenever the last client goes), on free displays
# between :970 and :979, and are killed at the end.  Skipped (77)
# without Xvfb or xrandr.

. "${srcdir:-.}/tests/lib.sh"

: "${XSETEDID:=$PWD/tests/xsetedid}"
[ -x "$DTRANDR" ] || { echo "no $DTRANDR"; exit 99; }
for p in Xvfb xrandr; do
    command -v $p > /dev/null 2>&1 || { echo "no $p, skipped"; exit 77; }
done
[ -x "$XSETEDID" ] || { echo "no $XSETEDID, skipped"; exit 77; }

pids=
daemon=
cleanup() {
    for p in $daemon $pids; do
        kill "$p" 2> /dev/null
    done
    wait 2> /dev/null
}
trap cleanup EXIT
trap 'exit 1' INT TERM

# start_server VAR COMMAND...: start an X server (":N" is put after the
# command name) on a free display in 970..979; VAR gets the display
start_server() {
    var=$1
    shift
    cmd=$1
    shift
    for n in 970 971 972 973 974 975 976 977 978 979; do
        [ -e /tmp/.X$n-lock ] || [ -e /tmp/.X11-unix/X$n ] && continue
        "$cmd" ":$n" "$@" -nolisten tcp > "$WORKROOT/server-$n.log" 2>&1 &
        pid=$!
        i=0
        while [ $i -lt 50 ]; do
            if DISPLAY=:$n xrandr -q > /dev/null 2>&1; then
                pids="$pids $pid"
                eval "$var=:$n"
                return 0
            fi
            kill -0 $pid 2> /dev/null || break
            sleep 0.1
            i=$((i + 1))
        done
        kill $pid 2> /dev/null
        wait $pid 2> /dev/null
    done
    return 1
}

# The CPU time of a process, in clock ticks
cputime() {
    [ -r /proc/$1/stat ] || { echo 0; return; }
    sed 's/^.*) //' /proc/$1/stat | awk '{print $12 + $13}'
}

# wait_for COUNT FILE PATTERN: wait up to COUNT/10 s for PATTERN in FILE
wait_for() {
    i=0
    while [ $i -lt "$1" ]; do
        grep -q -e "$3" "$2" 2> /dev/null && return 0
        sleep 0.1
        i=$((i + 1))
    done
    return 1
}

start_server VFB Xvfb -noreset -screen 0 4096x4096x24 \
    || { echo "cannot start Xvfb, skipped"; exit 77; }
export DISPLAY=$VFB
REAL_X=1

# ---------------------------------------------------------------- Xvfb

t xvfb-show
run --fingerprint
status_is 0
out_is "screen --CONNECTED-BUT-EDID-UNAVAILABLE--screen"
run --config
status_is 0
out_is "output screen
crtc 0
gamma 1.0:1.0:1.0
mode 4096x4096
pos 0x0
rate 0.00
rotate normal"
run --detected
status_is 0
[ ! -s "$W/out" ] || fail "detected: $(cat "$W/out")"
# auto: nothing to change but the primary output, which there is not
run --change
status_is 0
out_has "applied profile auto (no profile matches)"
xrandr | grep -q "^screen connected primary" || fail "no primary output"
run --change
out_has "profile auto (no profile matches) is current"
run --save vfb
status_is 0
run --current
out_is "vfb"
run --list
out_has "^vfb .* (detected) (current)$"

t xvfb-edid
"$XSETEDID" screen $A || fail "xsetedid"
run --fingerprint
out_is "screen $A"
"$XSETEDID" -d screen

t xvfb-daemon
newprofile edid1 "screen $A" "output screen
mode 4096x4096
pos 0x0"
"$DTRANDR" --daemon -v > "$W/dout" 2> "$W/derr" &
daemon=$!
wait_for 30 "$W/derr" "waiting for monitors" || fail "the daemon did not start"
kill -0 $daemon 2> /dev/null || fail "the daemon exited"
grep -q "profile auto (no profile matches) is current" "$W/dout" \
    || fail "daemon: no first change: $(cat "$W/dout" "$W/derr")"
# only one per display
run --daemon -v
status_is 0
err_has "already"
# a "new monitor"
"$XSETEDID" screen $A
wait_for 30 "$W/dout" "profile edid1 (detected)" \
    || fail "daemon: no change after the EDID changed: $(cat "$W/derr")"
grep -q "the connected monitors changed" "$W/derr" || fail "no message"
# the same EDID again: a property event, but no change
lines=$(wc -l < "$W/dout")
"$XSETEDID" screen $A
sleep 1
[ "$(wc -l < "$W/dout")" = "$lines" ] \
    || fail "daemon reacted to the same monitors: $(cat "$W/dout")"
# idle: no CPU time while nothing happens
t0=$(cputime $daemon)
sleep 3
t1=$(cputime $daemon)
[ $((t1 - t0)) -le 5 ] || fail "the daemon is busy: $((t1 - t0)) ticks in 3 s"
echo "daemon CPU time while idle: $((t1 - t0)) ticks in 3 s"
# the server goes away: the daemon exits
for p in $pids; do kill $p; done
pids=
i=0
while kill -0 $daemon 2> /dev/null && [ $i -lt 50 ]; do
    sleep 0.1
    i=$((i + 1))
done
if kill -0 $daemon 2> /dev/null; then
    fail "the daemon outlived the X server"
else
    wait $daemon
    [ $? = 0 ] || fail "the daemon failed when the server went away"
fi
daemon=

# ---------------------------------------------------------------- Xephyr

if ! command -v Xephyr > /dev/null 2>&1; then
    echo "no Xephyr, skipped the Xephyr tests"
    finish
    exit
fi
start_server VFB Xvfb -noreset -screen 0 4096x4096x24 \
    || { fail "cannot start Xvfb again"; finish; exit; }
DISPLAY=$VFB start_server EPH Xephyr -noreset -screen 1920x1080 \
    || { echo "cannot start Xephyr, skipped"; finish; exit; }
export DISPLAY=$EPH

# current_mode: the output's geometry and rotation, from xrandr
current_mode() {
    xrandr 2> /dev/null | awk '$2 == "connected" {
        if ($3 == "primary") print $4, $5; else print $3, $4; exit }'
}

t xephyr-restore
# (Xephyr lists the size it started with only while it is current)
xrandr --output default --mode 1280x1024 2> /dev/null || fail "xrandr --mode"
run --save base
status_is 0
base=$U/base
xrandr --output default --mode 1024x768 2> /dev/null || fail "xrandr --mode"
[ "$(current_mode)" = "1024x768+0+0 (normal" ] || fail "xrandr: $(current_mode)"
run --current
[ ! -s "$W/out" ] || fail "base is still current"
# --dry-run changes nothing
run --change --dry-run
status_is 0
out_has "would apply profile base (detected)"
out_has "^xrandr --fb 1280x1024 --output default --mode 1280x1024 "
[ "$(current_mode)" = "1024x768+0+0 (normal" ] || fail "--dry-run changed it"
# the xrandr command line it prints does the same
grep '^xrandr ' "$W/out" > "$W/cmd"
sh "$W/cmd" 2> /dev/null || fail "xrandr $(cat "$W/cmd")"
run --current
out_is "base"
xrandr --output default --mode 1024x768 2> /dev/null
run --change
status_is 0
out_has "applied profile base (detected)"
[ "$(current_mode)" = "1280x1024+0+0 (normal" ] \
    || fail "not restored: $(current_mode)"
run --change
out_has "is current"
# (Xephyr crashes when it is rotated, even by xrandr: no rotation here)

t xephyr-mode-by-size
newprofile sz "" "output default
mode 1280x960_other"
run --load sz
status_is 0
[ "$(current_mode)" = "1280x960+0+0 (normal" ] || fail "mode: $(current_mode)"
run --config
out_has "^mode 1280x960$"

t xephyr-daemon
mkdir -p "$U"
cp -R "$base" "$U/"
xrandr --output default --mode 1024x768 2> /dev/null
"$DTRANDR" --daemon -v > "$W/dout" 2> "$W/derr" &
daemon=$!
wait_for 30 "$W/derr" "waiting for monitors" || fail "the daemon did not start"
grep -q "applied profile base" "$W/dout" || fail "no first change"
# its own change does not wake it up, nor does a change of mode
sleep 1
xrandr --output default --mode 1024x768 2> /dev/null
sleep 1.5
[ "$(current_mode)" = "1024x768+0+0 (normal" ] \
    || fail "the daemon reverted a mode change: $(current_mode)"
[ "$(grep -c "profile" "$W/dout")" = 1 ] \
    || fail "the daemon applied again: $(cat "$W/dout")"
t0=$(cputime $daemon)
sleep 2
t1=$(cputime $daemon)
[ $((t1 - t0)) -le 5 ] || fail "the daemon is busy: $((t1 - t0)) ticks"
kill -TERM $daemon
i=0
while kill -0 $daemon 2> /dev/null && [ $i -lt 30 ]; do
    sleep 0.1
    i=$((i + 1))
done
if kill -0 $daemon 2> /dev/null; then
    fail "the daemon ignored SIGTERM"
else
    wait $daemon
    [ $? = 0 ] || fail "the daemon failed on SIGTERM"
fi
grep -q "exiting on signal" "$W/derr" || fail "no exit message"
daemon=

finish
