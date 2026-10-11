# Helpers for the dtrandr tests (sourced).
#
#   DTRANDR          the program (default: ./dtrandr)
#   DTRANDR_WRAPPER  a command to run it under, e.g.
#                    "valgrind -q --error-exitcode=99 --leak-check=full"
#   srcdir           the source directory of programs/dtrandr

: "${DTRANDR:=$PWD/dtrandr}"
: "${srcdir:=.}"
FIX=$srcdir/tests/fake
PROF=$srcdir/tests/profiles
WORKROOT=${TESTWORK:-$PWD/tests/work}/$(basename "$0" .sh)
rm -rf "$WORKROOT"
mkdir -p "$WORKROOT" || exit 99

A=00ffffffffffff000daeaaaa0000000001    # the panel, eDP-1
B=00ffffffffffff0010acbbbb0000000002    # the monitor on HDMI-1
C=00ffffffffffff004c2dcccc0000000003    # the monitor on DP-1

fails=0
ntests=0
current=

# t NAME [FIXTURE]: start a test with a fresh home, system directories
# and fake display
t() {
    current=$1
    ntests=$((ntests + 1))
    W=$WORKROOT/$ntests-$1
    mkdir -p "$W/home" "$W/sys" "$W/xdg"
    export HOME="$W/home"
    unset XDG_CONFIG_HOME DTRANDR_FAKE_OUT
    export XDG_CONFIG_DIRS="$W/xdg" DTRANDR_SYSTEM_DIR="$W/sys"
    export DTRANDR_FAKE="$W/display" DTRANDR_FAKE_LOG="$W/log"
    cp "$FIX/${2:-docked}.fake" "$W/display"
    : > "$W/log"
    U=$HOME/.config/dtrandr
    # the X tests use the real display
    [ -z "$REAL_X" ] || unset DTRANDR_FAKE DTRANDR_FAKE_LOG
}

fail() {
    echo "FAIL [$current]: $*" >&2
    fails=$((fails + 1))
}

# run ARG...: run dtrandr, keeping its output in $W/out and $W/err
run() {
    $DTRANDR_WRAPPER "$DTRANDR" "$@" > "$W/out" 2> "$W/err"
    status=$?
    if [ "$status" = 99 ] && [ -n "$DTRANDR_WRAPPER" ]; then
        fail "$DTRANDR_WRAPPER found errors in: dtrandr $*"
        cat "$W/err" >&2
    fi
}

status_is() {
    [ "$status" = "$1" ] \
        || fail "exit status $status, expected $1: $(cat "$W/err")"
}

out_has() {
    grep -q -e "$1" "$W/out" || { fail "output lacks \"$1\":"; cat "$W/out" >&2; }
}

out_lacks() {
    ! grep -q -e "$1" "$W/out" || fail "output has \"$1\""
}

err_has() {
    grep -q -e "$1" "$W/err" || { fail "stderr lacks \"$1\":"; cat "$W/err" >&2; }
}

err_lacks() {
    ! grep -q -e "$1" "$W/err" || fail "stderr has \"$1\""
}

out_is() {
    printf '%s\n' "$1" | cmp -s - "$W/out" || {
        fail "output differs:"
        printf '%s\n' "$1" | diff - "$W/out" >&2
    }
}

display_has() {
    grep -q -e "$1" "$DTRANDR_FAKE" \
        || { fail "display lacks \"$1\""; cat "$DTRANDR_FAKE" >&2; }
}

display_lacks() {
    ! grep -q -e "$1" "$DTRANDR_FAKE" || fail "display has \"$1\""
}

log_has() {
    grep -q -e "$1" "$W/log" || { fail "log lacks \"$1\":"; cat "$W/log" >&2; }
}

log_empty() {
    [ ! -s "$W/log" ] || { fail "something was applied:"; cat "$W/log" >&2; }
}

# crtc_is N "ACTIVE LINE" (or "off")
crtc_is() {
    got=$(awk -v n="$1" '/^(crtc|output) /{c = ($1 == "crtc") ? $2 : -1}
        /^  active / && c == n {sub(/^  active /, ""); print}' "$DTRANDR_FAKE")
    [ "$got" = "${2#off}" ] || fail "crtc $1 is \"$got\", expected \"$2\""
}

# edit TYPE NAME KEY [VALUE]: in the block "output NAME" or "crtc NAME"
# of the fake display, set the line KEY (connected and disconnected are
# the same key) to KEY VALUE, or delete it if VALUE is "-"
edit() {
    awk -v type="$1" -v name="$2" -v key="$3" -v val="$4" '
        function line() { return "  " key (val != "" ? " " val : "") }
        function same(k) {
            return k == key || (key ~ /^(dis)?connected$/ && k ~ /^(dis)?connected$/)
        }
        function flush() {
            if (inblk && !done && val != "-")
                print line()
            inblk = 0
            done = 0
        }
        /^(output|crtc) / { flush(); if ($1 == type && $2 == name) inblk = 1 }
        inblk && same($1) { if (!done && val != "-") print line(); done = 1; next }
        { print }
        END { flush() }' "$DTRANDR_FAKE" > "$W/tmp" && mv "$W/tmp" "$DTRANDR_FAKE"
}

# profile NAME [DIR]: install a test profile (default: the user directory)
profile() {
    mkdir -p "${2:-$U}"
    cp -R "$PROF/$1" "${2:-$U}/"
}

# newprofile NAME SETUP CONFIG [DIR]
newprofile() {
    mkdir -p "${4:-$U}/$1"
    printf '%s\n' "$2" > "${4:-$U}/$1/setup"
    printf '%s\n' "$3" > "${4:-$U}/$1/config"
}

finish() {
    echo "$ntests tests, $fails failures"
    [ "$fails" = 0 ]
}
