MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

lib_LTLIBRARIES = libDtPrint.la

libDtPrint_la_CFLAGS = $(CDE_CFLAGS) -DMULTIBYTE -DI18N_MSG @DT_INCDIR@

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtPrint_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 2:1:0
else
libDtPrint_la_LDFLAGS = $(CDE_LDFLAGS) $(CDE_LIB_LDFLAGS) -version-info 3:0:1
endif


if SOLARIS
libDtPrint_la_CFLAGS += -DNO_REGCOMP
endif

libDtPrint_la_LIBADD = $(LIBHELP) $(LIBSVC) $(XMLIB) $(XTLIB) $(X11LIB)

libDtPrint_la_SOURCES = PrintMsgs.c PrintDlgMgr.c PrintOptions.c PrintSetupB.c \
			PsubDefProc.c PsubUtil.c
