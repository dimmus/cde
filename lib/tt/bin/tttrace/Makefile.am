MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = tttrace

tttrace_CXXFLAGS = -I../../lib

tttrace_LDADD = $(LIBTT) $(TIRPCLIB) $(XTOOLLIB)

if SOLARIS
tttrace_LDADD += -lintl -lsocket -lnsl
endif

tttrace_SOURCES = tttrace.C tttrace_objs.C
