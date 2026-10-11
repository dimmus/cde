MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtaction

dtaction_SOURCES = Main.c

dtaction_LDADD = $(LIBSVC) $(XMLIB) $(XTLIB) $(X11LIB) $(CRYPTLIB)
