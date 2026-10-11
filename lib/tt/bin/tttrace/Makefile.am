MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = tttrace

tttrace_CXXFLAGS = $(CDE_CXXFLAGS) -I../../lib

tttrace_LDADD = $(LIBTT) $(TIRPCLIB)

if SOLARIS
tttrace_LDADD += -lintl -lsocket -lnsl
endif

tttrace_SOURCES = tttrace.C tttrace_objs.C
