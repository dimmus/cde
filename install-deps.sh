#!/usr/bin/env bash
#
# install-deps.sh - install everything needed to build (and run) CDE on Arch Linux.
#
# Derived from the checks in configure.ac plus libraries the Makefiles link
# that configure does not test for (libXss).
#
# Motif (libXm) is NOT installed from pacman: it is built from ~/motif.
# Only that build's own dependencies are installed here.
#
# Usage: ./install-deps.sh [--no-runtime] [--yes] [--dry-run]
#   --no-runtime  only install build dependencies
#   --yes         pass --noconfirm to pacman
#   --dry-run     print the pacman command without running it

set -euo pipefail

runtime=1
noconfirm=()
dry_run=0

for arg in "$@"; do
    case "$arg" in
        --no-runtime) runtime=0 ;;
        --yes|-y)     noconfirm=(--noconfirm) ;;
        --dry-run|-n) dry_run=1 ;;
        -h|--help)    sed -n '3,12p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

if ! command -v pacman >/dev/null; then
    echo "This script supports Arch Linux (pacman) only." >&2
    echo "See https://sourceforge.net/p/cdesktopenv/wiki/Home/ for other distributions." >&2
    exit 1
fi

# Toolchain and autotools (autogen.sh, configure)
build_deps=(
    base-devel autoconf automake libtool pkgconf
    bison flex m4 perl gzip ncurses
)

# Programs configure requires (MISSING_PROGS in configure.ac)
build_deps+=(
    ksh                 # ksh / ksh93
    xorg-xrdb           # xrdb
    xorg-bdftopcf       # bdftopcf
    xorg-mkfontscale    # mkfontdir
    rpcsvc-proto        # rpcgen
    opensp              # onsgmls (help/dtinfo docs)
    xorg-sessreg        # sessreg
    tcl                 # tclConfig.sh (dtksh)
)

# Build dependencies of our own Motif (~/motif, CMake)
build_deps+=(
    cmake fontconfig libxft libpng libjpeg-turbo
    ninja ccache        # 'make ... ninja ccache' targets in ~/motif/GNUmakefile
)

# Libraries and headers
build_deps+=(
    libx11 libxext libxrender libxau libxpm libxt libxmu libsm libice
    libxinerama libxss libxdmcp
    xbitmaps
    libtirpc lmdb
    pam libutempter
    libxcrypt           # crypt() in dtlogin and the dtsession screen lock
)

# Needed to actually run the desktop (ToolTalk needs rpcbind, dtsession uses
# xset/xsetroot, fonts for the default interface)
runtime_deps=(
    rpcbind
    xorg-xset xorg-xsetroot
    xorg-fonts-misc xorg-fonts-100dpi xorg-fonts-75dpi
)

pkgs=("${build_deps[@]}")
(( runtime )) && pkgs+=("${runtime_deps[@]}")

sudo_cmd=()
(( EUID != 0 )) && sudo_cmd=(sudo)

cmd=("${sudo_cmd[@]}" pacman -S --needed "${noconfirm[@]}" "${pkgs[@]}")

if (( dry_run )); then
    printf '%q ' "${cmd[@]}"; echo
    exit 0
fi

echo "==> Installing ${#pkgs[@]} packages (already installed ones are skipped)"
"${cmd[@]}"

# Verify that what configure looks for is now present
echo "==> Verifying"
fail=0
for prog in ksh xrdb perl cpp bdftopcf mkfontdir gzip m4 rpcgen gencat onsgmls sessreg; do
    if ! command -v "$prog" >/dev/null; then
        echo "  missing program: $prog"; fail=1
    fi
done
for lib in Xss Xinerama jpeg lmdb tirpc crypt; do
    if ! ldconfig -p | grep -q "lib${lib}\.so"; then
        echo "  missing library: lib${lib}"; fail=1
    fi
done
[[ -f /usr/lib/tclConfig.sh ]] || { echo "  missing: /usr/lib/tclConfig.sh"; fail=1; }

# Motif comes from ~/motif, so only warn: it may simply not be built yet.
# CDE needs libXm, libMrm (libDtMrm), libUil (dtbuilder), the Xm headers and
# the xm_* cursor bitmaps (DtSvc includes X11/bitmaps/xm_hour16 and friends).
motif_missing=()
motif_local=()
for lib in Xm Mrm Uil; do
    if ldconfig -p | grep -q "lib${lib}\.so"; then
        :
    elif compgen -G "/usr/local/lib/lib${lib}.so*" >/dev/null; then
        motif_local+=("lib${lib}")
    else
        motif_missing+=("lib${lib}")
    fi
done
if [[ ! -f /usr/include/Xm/Xm.h && ! -f /usr/local/include/Xm/Xm.h ]]; then
    motif_missing+=("Xm/Xm.h")
fi
if [[ ! -f /usr/include/X11/bitmaps/xm_hour16 && ! -f /usr/local/include/X11/bitmaps/xm_hour16 ]]; then
    motif_missing+=("X11/bitmaps/xm_*")
fi
if (( ${#motif_local[@]} )); then
    echo "  note: ${motif_local[*]} in /usr/local/lib, which gcc and ld.so do not search on Arch."
    echo "        Either reinstall Motif with -DCMAKE_INSTALL_PREFIX=/usr, or configure CDE with"
    echo "        LDFLAGS=-L/usr/local/lib and add /usr/local/lib to /etc/ld.so.conf.d/ (then ldconfig)."
fi
if (( ${#motif_missing[@]} )); then
    echo "  note: Motif not found (${motif_missing[*]}) - build and install Motif from ~/motif"
    echo "        before configuring CDE; configure stops without it."
fi

# The build generates message catalogs and docs under en_US.UTF-8
if ! locale -a | grep -qiE '^en_US\.utf-?8$'; then
    echo "  locale en_US.UTF-8 is not generated: uncomment it in /etc/locale.gen and run 'sudo locale-gen'"
    fail=1
fi

if (( fail )); then
    echo "==> Some dependencies are still missing (see above)." >&2
    exit 1
fi

echo "==> All dependencies present. Build with:"
echo "    ./autogen.sh && ./configure && make -j\$(nproc) && sudo make install"
if (( runtime )); then
    echo "==> To run CDE, also enable rpcbind:  sudo systemctl enable --now rpcbind"
fi
