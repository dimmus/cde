MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ttauth

ttauth_CFLAGS = -I../../slib -I../../lib

ttauth_LDADD = @LIBTT@ $(X_LIBS)

if LINUX
ttauth_LDADD += $(TIRPCLIB) $(XTOOLLIB)
endif

if SOLARIS
ttauth_LDADD += $(XTOOLLIB) -lintl -lsocket -lnsl
endif

if BSD
ttauth_LDADD += $(XTOOLLIB)
endif

ttauth_SOURCES = ttauth.c process.c
