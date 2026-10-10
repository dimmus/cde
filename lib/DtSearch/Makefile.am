MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

SUBDIRS = raima

lib_LTLIBRARIES = libDtSearch.la

BUILT_SOURCES = boolyac.h boolyac.c
CLEANFILES = boolyac.h boolyac.c
AM_YFLAGS = -d

libDtSearch_la_CFLAGS = $(CDE_CFLAGS) -DI18N_MSG -DMULTIBYTE -I./raima \
			$(DT_INCDIR)


# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtSearch_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0
else
libDtSearch_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 3:0:1
endif

# export only the symbols in build-aux/symbols/libDtSearch.sym
if EXPORT_MAPS
libDtSearch_la_LDFLAGS += -Wl,--version-script=$(VSCRIPT_DIR)/libDtSearch-exports.ver
EXTRA_libDtSearch_la_DEPENDENCIES = $(VSCRIPT_DIR)/libDtSearch-exports.ver
endif
$(VSCRIPT_DIR)/libDtSearch-exports.ver: $(EXPORTS_DIR)/libDtSearch.sym
	$(AM_V_GEN)$(MAKE_VERSION_SCRIPT)

libDtSearch_la_LIBADD = raima/libraima.la $(LIBSVC)

libDtSearch_la_SOURCES = apndext.c	ausdopen.c	ausexit.c \
			 bmstrstr.c	boolpars.c	boolsrch.c \
			 boolyac.y	cuslang.c	dbchange.c \
			 dberr.c	delspace.c	dtoe.c \
			 dtoeinit.c	dtsrapi.c	dtsrdbrec.c \
			 dtsrjoint.c	dtsrswab.c	dtsrutil.c  \
			 dtsrve.c	endslash.c	fileman.c \
			 globals.c	hdecode.c	hencode.c \
			 hilite.c	iscompat.c	isduprec.c \
			 jpn.c		lang.c		langmap.c \
			 msgs.c		msgutil.c	objdate.c \
			 ocf.c		opendblk.c	ophuf.c \
			 readchar.c	strupr.c	userint.c \
			 vedelete.c	vestatis.c	vstfunct.c
