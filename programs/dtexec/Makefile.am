MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtexec

dtexec_CFLAGS = $(CDE_CFLAGS) -DMSGLOG_CLIENT_ONLY	\
	-I$(srcdir)/../../lib/DtSvc/include

# libDtSvc is loaded only to log a message (LogStderr in Main.c).
dtexec_LDADD = $(LIBTT)

dtexec_SOURCES = Main.c
