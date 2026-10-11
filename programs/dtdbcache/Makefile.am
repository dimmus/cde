MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtdbcache

dtdbcache_LDADD = $(LIBSVC) $(LIBTT) $(XMLIB) $(XTLIB) $(X11LIB)

if SOLARIS
dtdbcache_LDADD += -lw -lgen
endif

dtdbcache_SOURCES = Main.c
