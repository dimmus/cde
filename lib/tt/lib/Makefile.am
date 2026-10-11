MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

SUBDIRS = api db mp util tttk

lib_LTLIBRARIES = libtt.la

libtt_la_SOURCES =

# fake things so libtool creates this as a C++ library
nodist_EXTRA_libtt_la_SOURCES = dummy.cxx
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

