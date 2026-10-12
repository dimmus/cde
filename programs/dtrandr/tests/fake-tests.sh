#!/bin/sh
#
# dtrandr against a fake display (DTRANDR_FAKE): profiles, matching,
# fallbacks, virtual profiles, CRTCs, modes, screen size, hooks.

. "${srcdir:-.}/tests/lib.sh"

[ -x "$DTRANDR" ] || { echo "no $DTRANDR"; exit 99; }

# ---------------------------------------------------------------- show

t fingerprint
run --fingerprint
status_is 0
out_is "HDMI-1 $B
eDP-1 $A"
edit output HDMI-1 edid -
run --fingerprint
out_has "^HDMI-1 --CONNECTED-BUT-EDID-UNAVAILABLE--HDMI-1$"

t config
run --config
status_is 0
out_is "output DP-1
off
output HDMI-1
off
output eDP-1
crtc 0
gamma 1.0:1.0:1.0
mode 1920x1080
pos 0x0
primary
rate 60.01
rotate normal"

# ---------------------------------------------------------------- save

t save
run --save now
status_is 0
"$DTRANDR" --config | cmp -s - "$U/now/config" || fail "config differs"
"$DTRANDR" --fingerprint | cmp -s - "$U/now/setup" || fail "setup differs"
log_empty
run --save now
status_is 1
err_has "not overwriting"
run --save now --force
status_is 0
for bad in off auto common horizontal none a/b .hidden settings; do
    run --save "$bad"
    status_is 1
done
[ ! -e "$U/off" ] || fail "saved a profile named off"

# ---------------------------------------------------------------- matching

t exact-beats-wildcard
for p in home home-wild office triple fancy; do profile $p; done
run --detected
out_is "home
office
home-wild"
run --change
status_is 0
out_has "applied profile home (detected)"
crtc_is 0 "0x41 2560 0 normal normal eDP-1"
crtc_is 1 "0x44 0 0 normal normal HDMI-1"
display_has "^screen 4480x1440 "
display_has "^primary HDMI-1$"
run
out_has "profile home (detected) is current"

t glob
for p in home home-wild office; do profile $p; done
edit output HDMI-1 edid 00ffffffffffff0010acdddd0000000005
run -c
status_is 0
out_has "applied profile office (detected)"
display_has "active 0x43 1920 0 normal normal HDMI-1"
display_has "active 0x41 0 0 normal normal eDP-1"
display_has "^screen 3200x1080 "
display_has "^primary eDP-1$"

t wildcard
for p in home home-wild office; do profile $p; done
edit output HDMI-1 edid 00ffffffffffff001e6d000000000009
run --change
out_has "applied profile home-wild (detected)"
display_has "active 0x41 0 0 normal normal HDMI-1"
display_has "active 0x41 0 1080 normal normal eDP-1"
display_has "^screen 1920x2160 "

t set-mismatch
profile home
profile home-wild
edit output DP-1 connected
edit output DP-1 edid $C
run --detected
[ ! -s "$W/out" ] || fail "detected: $(cat "$W/out")"
run --change
out_has "applied profile auto (no profile matches)"
profile triple
run --detected
out_is "triple"
run --change
out_has "applied profile triple (detected)"
display_has "active 0x47 4480 0 normal normal DP-1"
display_has "active 0x44 0 0 normal normal HDMI-1"
display_has "active 0x41 2560 0 normal normal eDP-1"
display_has "^screen 5760x1440 "

t tie-order
# equal matches: the first in name order wins
newprofile b-second "eDP-1 $A
HDMI-1 $B" "output HDMI-1
off
output eDP-1
mode 1024x768"
newprofile a-first "eDP-1 $A
HDMI-1 $B" "output HDMI-1
off
output eDP-1
mode 1920x1080
rate 59.93"
run --change
out_has "applied profile a-first (detected)"
display_has "active 0x42 0 0 normal normal eDP-1"

# ---------------------------------------------------------------- fallbacks

t auto-new-monitor
run --change
status_is 0
out_has "applied profile auto (no profile matches)"
crtc_is 0 "0x41 0 0 normal normal eDP-1"
crtc_is 1 "0x44 1920 0 normal normal HDMI-1"
display_has "^screen 4480x1440 "
display_has "^primary eDP-1$"
# the panel was not touched
log_has "^crtc 1 2560x1440"
! grep -q "^crtc 0\|disable crtc 0" "$W/log" || fail "touched crtc 0"

t auto-disconnected
edit crtc 1 active "0x44 1920 0 normal normal HDMI-1"
edit output HDMI-1 disconnected
edit output HDMI-1 edid -
sed 's/^screen 1920x1080/screen 4480x1440/' "$DTRANDR_FAKE" > "$W/tmp" \
    && mv "$W/tmp" "$DTRANDR_FAKE"
run --change
status_is 0
crtc_is 0 "0x41 0 0 normal normal eDP-1"
crtc_is 1 off
display_has "^screen 1920x1080 "
# off first, then the smaller screen
awk '/disable crtc 1/{d=NR} /^size /{s=NR} END{exit !(d && s && d < s)}' \
    "$W/log" || fail "wrong order: $(cat "$W/log")"

t auto-nothing-active
edit crtc 0 active -
edit output DP-1 connected
edit output DP-1 edid $C
run --change
status_is 0
# side by side in name order (DP-1, HDMI-1, eDP-1), the first primary
display_has "active 0x47 0 0 normal normal DP-1"
display_has "active 0x44 1280 0 normal normal HDMI-1"
display_has "active 0x41 3840 0 normal normal eDP-1"
display_has "^primary DP-1$"
display_has "^screen 5760x1440 "

t auto-primary-gone
edit crtc 1 active "0x44 1920 0 normal normal HDMI-1"
sed 's/^primary eDP-1/primary HDMI-1/' "$DTRANDR_FAKE" > "$W/tmp" \
    && mv "$W/tmp" "$DTRANDR_FAKE"
edit output HDMI-1 disconnected
run --load auto
status_is 0
display_has "^primary eDP-1$"

t default-setting
profile triple
mkdir -p "$U"
echo "default=vertical" > "$U/settings"
run --change
out_has "applied profile vertical (default setting)"
display_has "active 0x44 0 0 normal normal HDMI-1"
display_has "active 0x41 0 1440 normal normal eDP-1"
run --change --default horizontal
out_has "applied profile horizontal (--default)"
display_has "active 0x41 2560 0 normal normal eDP-1"

t default-none
printf '# no fallback\n default = none \n' > "$W/sys/settings"
run --change
status_is 0
log_empty

t default-system
echo "default=common" > "$W/sys/settings"
run --change
out_has "applied profile common (default setting)"
display_has "active 0x41 0 0 normal normal eDP-1"
display_has "active 0x41 0 0 normal normal HDMI-1"
display_has "^screen 1920x1080 "
# the user's setting wins
mkdir -p "$U"
echo "default=horizontal" > "$U/settings"
run --change
out_has "applied profile horizontal (default setting)"

t default-missing
run --change --default nosuch
status_is 1
err_has "no profile nosuch"

# ---------------------------------------------------------------- virtual

t clone-largest
run --load clone-largest
status_is 0
display_has "active 0x41 0 0 normal normal eDP-1"
display_has "active 0x44 0 0 normal normal HDMI-1"
display_has "transform 1.33333,0,0,0,1.33333,0,0,0,1"
display_has "filter bilinear"
display_has "^screen 2560x1440 "

t off
run --load off
status_is 1
err_has "force"
log_empty
run --load off --force
status_is 0
display_lacks "active"
run --load horizontal
status_is 0
display_has "active 0x44 0 0 normal normal HDMI-1"

t vertical-reverse
run --load vertical-reverse
display_has "active 0x41 0 0 normal normal eDP-1"
display_has "active 0x44 0 1080 normal normal HDMI-1"
display_has "^primary eDP-1$"

t virtual-name-arg
run horizontal
status_is 0
out_has "applied profile horizontal"

# ---------------------------------------------------------------- CRTCs

t clone-shared-crtc
edit output eDP-1 crtcs 0
edit output HDMI-1 crtcs 0
run --load common
status_is 1
err_has "not enough CRTCs"
edit output eDP-1 clones HDMI-1
edit output HDMI-1 clones eDP-1
run --load common
status_is 0
crtc_is 0 "0x41 0 0 normal normal eDP-1 HDMI-1"

t not-enough-crtcs
edit output DP-1 connected
edit output DP-1 edid $C
edit output DP-1 crtcs "0 1"
profile triple
run --change
status_is 1
err_has "not enough CRTCs to drive eDP-1, HDMI-1, DP-1"
log_empty

t crtc-request
newprofile moved "eDP-1 $A
HDMI-1 $B" "output eDP-1
crtc 1
mode 1920x1080
output HDMI-1
mode 1280x720
pos 1920x0"
run --change
status_is 0
crtc_is 0 "0x43 1920 0 normal normal HDMI-1"
crtc_is 1 "0x41 0 0 normal normal eDP-1"
# eDP-1 leaves CRTC 0 before CRTC 0 takes HDMI-1
awk '/disable crtc 0/{d=NR} /^crtc 0 /{s=NR} END{exit !(d && s && d < s)}' \
    "$W/log" || fail "wrong order: $(cat "$W/log")"

t crtc-request-impossible
newprofile p "eDP-1 $A
HDMI-1 $B" "output eDP-1
crtc 2
mode 1920x1080
output HDMI-1
off"
run --change -v
status_is 0
err_has "cannot use crtc 2"
out_has "profile p (detected) is current"

# ---------------------------------------------------------------- modes

t rate
for case in "50:0x45" "59.9:0x42" "60.01:0x41" ":0x41"; do
    rate=${case%%:*}
    mode=${case#*:}
    newprofile r "eDP-1 $A
HDMI-1 $B" "output eDP-1
off
output HDMI-1
mode 1920x1080
${rate:+rate $rate}"
    run --change
    status_is 0
    display_has "active $mode 0 0 normal normal HDMI-1"
done
newprofile r "" "output eDP-1
off
output HDMI-1
mode 1920x1080i
rate 60"
run --load r
display_has "active 0x46 0 0 normal normal HDMI-1"
# an unknown mode name: by its size
newprofile r "" "output HDMI-1
mode 1280x720_custom"
run --load r
status_is 0
display_has "active 0x43 0 0 normal normal HDMI-1"
newprofile r "" "output HDMI-1
mode 3840x2160"
run --load r
status_is 1
err_has "no mode 3840x2160"

# ---------------------------------------------------------------- geometry

t rotation-scale-panning-gamma
profile fancy
run --load fancy
status_is 0
crtc_is 1 "0x42 0 0 left x eDP-1"
crtc_is 0 "0x43 1080 0 normal normal HDMI-1"
display_has "^screen 3640x1920 mm 963x508 "
display_has "transform 2,0,0,0,2,0,0,0,1"
display_has "filter nearest"
display_has "panning 2560x1440+1080+0/0x0+0+0/0/0/0/0"
display_has "gamma 0.8:0.9:1$"
log_has "^gamma 1 0.8:0.9:1"
run --config
out_has "^reflect x$"
out_has "^rotate left$"
out_has "^gamma 0.8:0.9:1.0$"
out_has "^rate 59.93$"
out_has "^transform 2.000000,0.000000,0.000000,0.000000,2.000000,0.000000,0.000000,0.000000,1.000000$"
out_has "^panning 2560x1440+1080+0/0x0+0+0/0/0/0/0$"
run --current
out_is "fancy"
# the saved profile reads back as the same configuration
run --save again
run --load horizontal
run --load again
status_is 0
run --current
out_has "^again$"
out_has "^fancy$"

t too-large
newprofile big "" "output HDMI-1
mode 2560x1440
scale 4x4"
run --load big
status_is 1
err_has "larger than the maximum"
log_empty

# ---------------------------------------------------------------- applying

t no-op
profile home
run --change
log_has grab
: > "$W/log"
run --change
status_is 0
out_has "is current"
log_empty
run --change --force
log_has grab

t dry-run
profile home
cp "$DTRANDR_FAKE" "$W/before"
run --change --dry-run
status_is 0
out_has "would apply profile home (detected)"
out_has "^xrandr --fb 4480x1440 .*--output HDMI-1 --mode 2560x1440 --rate 59.95 --pos 0x0 --rotate normal --reflect normal --transform none --gamma 1:1:1 --crtc 1 --primary"
out_has "--output eDP-1 --mode 1920x1080 --rate 60.01 --pos 2560x0"
cmp -s "$W/before" "$DTRANDR_FAKE" || fail "the display changed"
log_empty
run -n --load horizontal
out_has "would apply profile horizontal"
log_empty
run -n --save x
[ ! -e "$U/x" ] || fail "--dry-run saved"

t restore-on-failure
profile home
echo "fail-crtc 1" >> "$DTRANDR_FAKE"
"$DTRANDR" --config > "$W/before"
run --change
status_is 1
err_has "restoring"
log_has "disable crtc"
"$DTRANDR" --config | cmp -s "$W/before" - || fail "not restored"
display_has "^screen 1920x1080 "
display_has "active 0x41 0 0 normal normal eDP-1"

t load-not-connected
profile triple
run --load triple
status_is 1
err_has "DP-1 is not connected"
log_empty

t unknown-keys
profile home
run --load home
err_lacks "x-prop"
run --load home --force -v
err_has 'ignoring "x-prop-broadcast_rgb"'

# ---------------------------------------------------------------- hooks

t hooks
profile home
mkdir -p "$U/postswitch.d"
for h in predetect preswitch postswitch postswitch.d/50-more home/postswitch \
         home/preswitch; do
    cat > "$U/$h" <<EOF
#!/bin/sh
echo "$h|\$AUTORANDR_CURRENT_PROFILE|\$AUTORANDR_PROFILE_FOLDER|\$AUTORANDR_MONITORS|\$DTRANDR_PROFILE" >> "$W/hooks"
EOF
    chmod +x "$U/$h"
done
printf '#!/bin/sh\nexit 3\n' > "$U/postswitch.d/60-fails"
chmod +x "$U/postswitch.d/60-fails"
printf 'not run\n' > "$U/postswitch.d/70-not-executable"
run --change
status_is 0
err_has "60-fails failed"
printf '%s\n' "predetect||||" \
    "preswitch|home|$U/home|eDP-1:HDMI-1|home" \
    "home/preswitch|home|$U/home|eDP-1:HDMI-1|home" \
    "postswitch|home|$U/home|eDP-1:HDMI-1|home" \
    "postswitch.d/50-more|home|$U/home|eDP-1:HDMI-1|home" \
    "home/postswitch|home|$U/home|eDP-1:HDMI-1|home" \
    | diff - "$W/hooks" >&2 || fail "hooks"
# nothing to change: only predetect
: > "$W/hooks"
run --change
printf '%s\n' "predetect||||" | diff - "$W/hooks" >&2 || fail "hooks again"
# dry run: none
: > "$W/hooks"
run --change --dry-run --force
out_has "would apply"
[ ! -s "$W/hooks" ] || fail "hooks ran in a dry run"

# ---------------------------------------------------------------- directories

t user-before-system
profile home "$W/sys"
profile home-wild
mv "$U/home-wild" "$U/home"
run --change
out_has "applied profile home (detected)"
display_has "active 0x41 0 0 normal normal HDMI-1"
run --list
out_has "^home  *$U (detected) (current)$"
rm -r "$U/home"
run --change
out_has "applied profile home (detected)"
display_has "active 0x44 0 0 normal normal HDMI-1"
run --list
out_has "^home  *$W/sys (detected) (current)$"

t system-save-remove
run --system --save sys1
status_is 0
[ -f "$W/sys/sys1/config" ] && [ -f "$W/sys/sys1/setup" ] || fail "not saved"
run --remove sys1
status_is 1
err_has "is in $W/sys/sys1"
run --system --remove sys1
status_is 0
[ ! -e "$W/sys/sys1" ] || fail "not removed"
run --remove sys1
status_is 1
err_has "no profile sys1"

t autorandr-dirs
profile home "$HOME/.config/autorandr"
profile office "$HOME/.autorandr"
profile triple "$W/xdg/autorandr"
run --list
out_has "^home  *$HOME/.config/autorandr"
out_has "^office  *$HOME/.autorandr"
out_has "^triple  *$W/xdg/autorandr"
run --change
out_has "applied profile home (detected)"
display_has "active 0x44 0 0 normal normal HDMI-1"
# ours before autorandr's
profile home
cp "$PROF/home-wild/config" "$U/home/config" && chmod u+w "$U/home/config"
run --change
display_has "active 0x41 0 0 normal normal HDMI-1"
# --save goes to ours
run --save mine
[ -f "$U/mine/config" ] || fail "not saved in $U"

t xdg-config-home
export XDG_CONFIG_HOME="$W/cfg"
profile home "$W/cfg/dtrandr"
run --change
out_has "applied profile home (detected)"
run --save x
[ -f "$W/cfg/dtrandr/x/config" ] || fail "not saved in XDG_CONFIG_HOME"
unset XDG_CONFIG_HOME

t no-home
profile home-wild
mv "$U/home-wild" "$U/home"
profile home "$W/sys"
printf '#!/bin/sh\necho sys >> "%s"\n' "$W/hooks" > "$W/sys/postswitch"
chmod +x "$W/sys/postswitch"
(unset HOME; "$DTRANDR" --change > "$W/out" 2> "$W/err")
status=$?
status_is 0
display_has "active 0x44 0 0 normal normal HDMI-1"
grep -q sys "$W/hooks" 2>/dev/null || fail "system hook not run"
(unset HOME; "$DTRANDR" --save x > "$W/out" 2> "$W/err")
status=$?
status_is 1
err_has "use --system"

t remove
profile home
mkdir -p "$U/home"
run --remove home
status_is 0
[ ! -e "$U/home" ] || fail "not removed"
profile home
touch "$U/home/postswitch"
run --remove home
status_is 0
err_has "left"

# ---------------------------------------------------------------- options

t options
run --help
status_is 0
out_has "Usage"
run --version
status_is 0
out_has "dtrandr"
run --bogus
status_is 2
run --list --config
status_is 2
run --system --change
status_is 2
run a b
status_is 2

finish
