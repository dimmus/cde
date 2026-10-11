MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dttypes

dttypes_LDADD = $(LIBSVC) $(XTLIB)

if SOLARIS
dttypes_LDADD += -lgen
endif

