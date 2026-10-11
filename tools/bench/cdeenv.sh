# CDE benchmarks: environment for running the uninstalled programs of
# this build tree.  Source it from the scripts of this directory:
#
#   . "$(dirname "$0")/cdeenv.sh"
#
# Sets CDE_TOP (the build tree, overridable), CDE_BENCH (this directory),
# LD_LIBRARY_PATH (the tree's libraries first), and defines
#
#   cde_bin NAME      print the path of the uninstalled binary NAME
#                     (programs/*/.libs/NAME, lib/tt/bin/*/.libs/NAME), or
#                     of the installed one when the tree has none
#   cde_free_display  print a free X display number
#   cde_xvfb N        start "Xvfb :N" in the background, wait until it
#                     answers, and set CDE_XVFB_PID
#
# The libtool wrapper scripts (programs/dtterm/dtterm...) are avoided:
# LD_PRELOAD would also load into the wrapper's shell, and the probes of
# dtwmbench do not work through it.

CDE_BENCH=$(cd "$(dirname "${BASH_SOURCE:-$0}")" && pwd)
: "${CDE_TOP:=$(cd "$CDE_BENCH/../.." && pwd)}"
export CDE_TOP CDE_BENCH

_cde_lp=
for _d in DtSvc DtWidget DtHelp DtPrint DtXinerama DtTerm DtSearch \
	  DtMmdb DtMrm csa tt/lib; do
	[ -d "$CDE_TOP/lib/$_d/.libs" ] && _cde_lp="$_cde_lp${_cde_lp:+:}$CDE_TOP/lib/$_d/.libs"
done
export LD_LIBRARY_PATH="$_cde_lp${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
unset _d _cde_lp

cde_bin() {
	for _p in "$CDE_TOP"/programs/"$1"/.libs/"$1" \
		  "$CDE_TOP"/programs/*/.libs/"$1" \
		  "$CDE_TOP"/programs/*/*/.libs/"$1" \
		  "$CDE_TOP"/lib/tt/bin/*/.libs/"$1"; do
		if [ -x "$_p" ]; then
			echo "$_p"
			return 0
		fi
	done
	command -v "$1"
}

cde_free_display() {
	_n=${1:-300}
	while [ -e /tmp/.X$_n-lock ] || [ -e /tmp/.X11-unix/X$_n ]; do
		_n=$((_n + 1))
	done
	echo "$_n"
}

cde_xvfb() {
	Xvfb ":$1" -screen 0 1920x1080x24 +extension RENDER -nolisten tcp \
		-noreset >/dev/null 2>&1 &
	CDE_XVFB_PID=$!
	for _i in $(seq 200); do
		xdpyinfo -display ":$1" >/dev/null 2>&1 && return 0
		sleep 0.05
	done
	echo "cde_xvfb: Xvfb :$1 did not start" >&2
	return 1
}
