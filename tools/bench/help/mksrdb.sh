#!/bin/bash
# CDE benchmarks, help: build a DtSearch database that stores its
# documents (AusText flavor), so that collocation ("ICE @5 CREAM")
# queries can run on it; the CDE infolibs' databases only index.
#
#   mksrdb.sh DIR [RECORDS]       (default 3000 records)
#
# Writes DIR/TESTDB.* and DIR/srbench.ocf (the configuration file to give
# to srbench or DtSearchInit).  Uses the tree's dtsrcreate, dtsrload and
# dtsrindex, the English stop list and suffixes, and a Huffman encode
# table made from the decode tree compiled into libDtSearch (ophuf.c).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
. "$HERE/../cdeenv.sh"
dir=${1:?usage: mksrdb.sh DIR [RECORDS]}
n=${2:-3000}
mkdir -p "$dir"
dir=$(cd "$dir" && pwd)
python3 -I "$HERE/mksrdb.py" "$CDE_TOP/lib/DtSearch/ophuf.c" "$dir" "$n"
cp "$CDE_TOP/programs/dtdocbook/dtsr/dtsearch.dbd" "$dir/"
cp "$CDE_TOP/programs/dtdocbook/dtsr/eng.stp" "$dir/TESTDB.stp"
cp "$CDE_TOP/programs/dtdocbook/dtsr/eng.sfx" "$dir/TESTDB.sfx"
cd "$dir"
P=$CDE_TOP/programs/dtsr
"$P/dtsrcreate" -q -o -fa -a100 -l0 TESTDB > create.log 2>&1
"$P/dtsrload" -dTESTDB -e"$dir/ophuf.huf" TESTDB > load.log 2>&1 || true
"$P/dtsrindex" -dTESTDB TESTDB > index.log 2>&1
printf 'PATH TESTDB = %s\nkeytypes TESTDB = Default Head Graphics Example Index Table\n' \
	"$dir" > srbench.ocf
rm -f TESTDB.fzk dtsearch.dbd
echo "mksrdb: $dir/srbench.ocf"
