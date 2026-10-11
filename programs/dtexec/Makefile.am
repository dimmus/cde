MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtexec

dtexec_CFLAGS = $(CDE_CFLAGS) -DMSGLOG_CLIENT_ONLY	\
	-I../../lib/DtSvc/include

dtexec_LDADD = $(LIBSVC) $(LIBTT)

dtexec_SOURCES = Main.c
