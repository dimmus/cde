MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

# The library is made of six convenience libraries, one per directory.
# They are all built from this one Makefile (each directory's
# Makefile.inc, included below) rather than by a make per directory, so
# that make sees the whole library at once: the directories no longer
# wait for one another, and a no-op build reads one Makefile instead of
# eight.  Paths in the Makefile.inc files are relative to this directory
# (%reldir%); sources include their headers as "dir/file.h" from here
# (-I. -I$(srcdir)), or from their own directory.

noinst_LTLIBRARIES =

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

include api/c/Makefile.inc
include api/dnd/Makefile.inc
include db/Makefile.inc
include mp/Makefile.inc
include util/Makefile.inc
include tttk/Makefile.inc

lib_LTLIBRARIES = libtt.la

libtt_la_SOURCES =

# fake things so libtool creates this as a C++ library
nodist_EXTRA_libtt_la_SOURCES = dummy.cxx

dummy.cxx:
	: > $@

BUILT_SOURCES = dummy.cxx
CLEANFILES = dummy.cxx
libtt_la_LIBADD = api/c/libapi.la api/dnd/libdnd.la db/libdb.la mp/libmp.la \
		  util/libutil.la tttk/libtttk.la $(XTLIB) $(X11LIB)

if SOLARIS
# This stuff should be figured out by configure
libtt_la_LIBADD += -lnsl -lsocket -lintl -lw
endif

if LINUX
libtt_la_LIBADD += $(TIRPCLIB)
endif

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libtt_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0
else
libtt_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 3:0:1
endif

# export only the symbols in build-aux/symbols/libtt.sym
if EXPORT_MAPS
libtt_la_LDFLAGS += -Wl,--version-script=$(VSCRIPT_DIR)/libtt-exports.ver
EXTRA_libtt_la_DEPENDENCIES = $(VSCRIPT_DIR)/libtt-exports.ver
endif
$(VSCRIPT_DIR)/libtt-exports.ver: $(EXPORTS_DIR)/libtt.sym
	$(AM_V_GEN)$(MAKE_VERSION_SCRIPT)

