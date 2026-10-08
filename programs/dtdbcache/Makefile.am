MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtdbcache

dtdbcache_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

if SOLARIS
dtdbcache_LDADD += -lw -lgen
endif

dtdbcache_SOURCES = Main.c
