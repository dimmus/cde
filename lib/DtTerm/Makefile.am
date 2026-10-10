MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

SUBDIRS = util TermPrim Term TermView

lib_LTLIBRARIES = libDtTerm.la

libDtTerm_la_SOURCES =

libDtTerm_la_LIBADD = Term/libTerm.la TermPrim/libTermPrim.la \
		       TermView/libTermView.la $(LIBHELP) $(LIBSVC) \
		       $(XMLIB) $(XTLIB) $(X11LIB)

if OPENBSD
libDtTerm_la_LIBADD += -lutil
endif

if FREEBSD
libDtTerm_la_LIBADD += -lutil
endif

if SOLARIS
libDtTerm_la_LIBADD += -lgen
endif

if HAS_UTEMPTER_LIBRARY
libDtTerm_la_LIBADD += -lutempter
endif

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtTerm_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0
else
libDtTerm_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 3:0:1
endif

# export only the symbols in build-aux/symbols/libDtTerm.sym
if EXPORT_MAPS
libDtTerm_la_LDFLAGS += -Wl,--version-script=$(VSCRIPT_DIR)/libDtTerm-exports.ver
EXTRA_libDtTerm_la_DEPENDENCIES = $(VSCRIPT_DIR)/libDtTerm-exports.ver
endif
$(VSCRIPT_DIR)/libDtTerm-exports.ver: $(EXPORTS_DIR)/libDtTerm.sym
	$(AM_V_GEN)$(MAKE_VERSION_SCRIPT)

