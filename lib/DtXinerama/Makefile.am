MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

lib_LTLIBRARIES = libDtXinerama.la

libDtXinerama_la_SOURCES = DtXinerama.c

libDtXinerama_la_LIBADD = $(XINERAMALIB) $(X11LIB)

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtXinerama_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0
else
libDtXinerama_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 3:0:1
endif

# export only the symbols in build-aux/exports/libDtXinerama.sym
if EXPORT_MAPS
libDtXinerama_la_LDFLAGS += -Wl,--version-script=libDtXinerama.ver
EXTRA_libDtXinerama_la_DEPENDENCIES = libDtXinerama.ver
endif
libDtXinerama.ver: $(EXPORTS_DIR)/libDtXinerama.sym
	$(AM_V_GEN)$(MAKE_VERSION_SCRIPT)
MOSTLYCLEANFILES = libDtXinerama.ver


