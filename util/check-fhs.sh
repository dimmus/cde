#!/bin/sh
#
# check-fhs.sh - verify a CDE install tree is free of hardcoded paths.
#
# Building CDE with packager-supplied paths should produce an install tree where no file
# contains a literal "/usr/dt", "/etc/dt", or "/var/dt" string -- all
# such references should expand to whatever the packager passed via
# --prefix, --with-cde-config-dir, --with-cde-state-dir, etc.
#
# Usage:
#   make install DESTDIR=$PWD/staging
#   ./util/check-fhs.sh staging
#
# Or, more usefully, build with non-default flags and run the check:
#
#   ./configure --prefix=/usr \
#     --bindir=/usr/bin --libdir=/usr/lib64 --includedir=/usr/include \
#     --sysconfdir=/etc \
#     --with-cde-data-dir=/usr/share/cde \
#     --with-cde-config-dir=/etc/cde \
#     --with-cde-state-dir=/var/lib/cde \
#     --with-cde-libexec-dir=/usr/libexec/cde
#   make
#   make install DESTDIR=$PWD/fhs-staging
#   ./util/check-fhs.sh fhs-staging
#
# Exits 0 if the tree is clean; non-zero (with a list of offending
# files and line numbers) if any "/usr/dt", "/etc/dt", or "/var/dt"
# literal survived into the install tree.

set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $0 <DESTDIR>" >&2
    exit 2
fi

DESTDIR=$1
if [ ! -d "$DESTDIR" ]; then
    echo "Error: $DESTDIR is not a directory" >&2
    exit 2
fi

# Skip these path families:
#  - The CDE_INSTALLATION_TOP value itself (the packager's chosen prefix)
#    will of course appear in installed files. We only flag the LEGACY
#    literals that survive when the prefix is something other than /usr/dt.
#  - Binary files (we restrict to text files via grep -I).
#  - Top-level docs (COPYING, HISTORY, README.md) which legitimately
#    contain "/usr/dt" in their text.
#  - Manual pages and help volumes whose text describes the historical
#    /usr/dt layout to users.
#
# A non-empty result means at least one piece of code or config didn't
# honor the packager's --with-cde-* override.

echo "==> Scanning $DESTDIR for surviving /usr/dt, /etc/dt, /var/dt literals"

found=$(grep -rIl --binary-files=without-match \
    --exclude='COPYING' --exclude='HISTORY' --exclude='README.md' \
    --exclude='copyright' --exclude='CONTRIBUTORS' \
    --exclude='*.1' --exclude='*.1m' --exclude='*.3' \
    --exclude='*.4' --exclude='*.5' --exclude='*.6' \
    --exclude-dir='man*' --exclude-dir='help' --exclude-dir='infolib' \
    -E '/(usr|etc|var)/dt(/|$|"|:)' "$DESTDIR" 2>/dev/null || true)

if [ -z "$found" ]; then
    echo "==> Clean: no surviving /usr/dt, /etc/dt, /var/dt literals."
    exit 0
fi

echo "==> Found legacy paths in:"
echo "$found" | sed 's/^/    /'
echo
echo "==> Sample offending lines:"
echo "$found" | head -20 | while IFS= read -r f; do
    grep -nE '/(usr|etc|var)/dt(/|$|"|:)' "$f" 2>/dev/null | head -3 | sed "s|^|  $f:|"
done
exit 1
