MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = ttauth

ttauth_CFLAGS = $(CDE_CFLAGS) -I../../slib -I../../lib

ttauth_LDADD = $(LIBTT)

if LINUX
ttauth_LDADD += $(TIRPCLIB)
endif

if SOLARIS
ttauth_LDADD += -lintl -lsocket -lnsl
endif

ttauth_SOURCES = ttauth.c process.c
