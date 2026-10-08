#!/bin/sh

# srcdir=`dirname $0`
# test -z "$srcdir" && srcdir=.

# cd "$srcdir"

# libtoolize --force --automake
# aclocal -I m4
# autoconf -f
# autoheader
# automake --foreign  --include-deps --add-missing

[ "${0%/*}" = "$0" ] || cd "${0%/*}" || exit

autoreconf -fiv && rm -rf autom4te.cache
./configure "$@"

exit $?

