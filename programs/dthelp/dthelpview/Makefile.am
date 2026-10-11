MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dthelpview

dthelpview_CFLAGS = $(CDE_CFLAGS) -D_BMS

dthelpview_LDADD = $(LIBHELP) $(LIBSVC) $(XMLIB) $(XTLIB) $(X11LIB)

dthelpview_SOURCES = Main.c ManPage.c Util.c
