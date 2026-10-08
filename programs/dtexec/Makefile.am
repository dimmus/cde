MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtexec

dtexec_CFLAGS = -DMSGLOG_CLIENT_ONLY	\
	-I../../lib/DtSvc/include

dtexec_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

dtexec_SOURCES = Main.c
