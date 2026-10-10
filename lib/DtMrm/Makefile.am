MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

lib_LTLIBRARIES = libDtMrm.la

libDtMrm_la_CFLAGS = $(CDE_CFLAGS) -I../DtTerm/Term -I../DtTerm/TermPrim -I../DtHelp \
		     @DT_INCDIR@ @DTI_INCDIR@

libDtMrm_la_SOURCES = DtMrm.c

libDtMrm_la_LIBADD = $(LIBTERM) $(LIBPRINT) $(LIBHELP) $(LIBWIDGET) \
		     $(MRESOURCELIB) $(XTLIB)

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtMrm_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0
else
libDtMrm_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 3:0:1
endif

# export only the symbols in build-aux/exports/libDtMrm.sym
if EXPORT_MAPS
libDtMrm_la_LDFLAGS += -Wl,--version-script=libDtMrm.ver
EXTRA_libDtMrm_la_DEPENDENCIES = libDtMrm.ver
endif
libDtMrm.ver: $(EXPORTS_DIR)/libDtMrm.sym
	$(AM_V_GEN)$(MAKE_VERSION_SCRIPT)
MOSTLYCLEANFILES = libDtMrm.ver

