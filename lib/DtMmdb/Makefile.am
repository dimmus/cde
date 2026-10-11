MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

# The library is made of 19 convenience libraries, one per directory,
# each with its own flags.  They are all built from this one Makefile
# (each directory's Makefile.inc, included below) rather than by a make
# per directory, so that make sees the whole library at once: the
# directories no longer wait for one another, and a no-op build reads
# one Makefile instead of twenty.  Paths in the Makefile.inc files are
# relative to this directory (%reldir%); sources include their headers
# as "dir/file.h" from here (-I. -I$(srcdir)), or from their own
# directory.

noinst_LTLIBRARIES =
noinst_HEADERS =
EXTRA_DIST =
CLEANFILES =
BUILT_SOURCES =

AM_YFLAGS = -l -d
AM_LFLAGS = -8 -s

include HardCopy/Makefile.inc
include StyleSheet/Makefile.inc
include api/Makefile.inc
include btree/Makefile.inc
include compression/Makefile.inc
include diskhash/Makefile.inc
include dstr/Makefile.inc
include dti_cc/Makefile.inc
include dti_excs/Makefile.inc
include dynhash/Makefile.inc
include hmphf/Makefile.inc
include index/Makefile.inc
include mgrs/Makefile.inc
include misc/Makefile.inc
include object/Makefile.inc
include oliasdb/Makefile.inc
include schema/Makefile.inc
include storage/Makefile.inc
include utility/Makefile.inc

lib_LTLIBRARIES = libDtMmdb.la

libDtMmdb_la_SOURCES =

# fake things so libtool links this as a C++ library (libstdc++)
nodist_EXTRA_libDtMmdb_la_SOURCES = dummy.cxx

libDtMmdb_la_LIBADD = HardCopy/libHardCopy.la \
		      StyleSheet/libStyleSheet.la \
		      api/libapi.la \
		      btree/libbtree.la \
		      compression/libcompression.la \
		      diskhash/libdiskhash.la \
		      dstr/libdstr.la \
		      dti_cc/libdti_cc.la \
		      dti_excs/libdti_excs.la \
		      dynhash/libdynhash.la \
		      hmphf/libhmphf.la \
		      index/libindex.la \
		      mgrs/libmgrs.la \
		      misc/libmisc.la \
		      object/libobject.la \
		      oliasdb/liboliasdb.la \
		      schema/libschema.la \
		      storage/libstorage.la \
		      utility/libutility.la $(LMDBLIB)

libDtMmdb_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0

# export only the symbols in build-aux/symbols/libDtMmdb.sym
if EXPORT_MAPS
libDtMmdb_la_LDFLAGS += -Wl,--version-script=$(VSCRIPT_DIR)/libDtMmdb-exports.ver
EXTRA_libDtMmdb_la_DEPENDENCIES = $(VSCRIPT_DIR)/libDtMmdb-exports.ver
endif
$(VSCRIPT_DIR)/libDtMmdb-exports.ver: $(EXPORTS_DIR)/libDtMmdb.sym
	$(AM_V_GEN)$(MAKE_VERSION_SCRIPT)
