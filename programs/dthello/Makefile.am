MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

bin_PROGRAMS = dthello

dthello_LDADD = $(LIBSVC) $(XTLIB) $(X11LIB)

if SOLARIS
dthello_LDADD += -lgen
endif

if LINUX
dthello_LDADD += $(TIRPCLIB)
endif

dthello_SOURCES = dthello.c

CPP_TARGETS = Dthello
BUILT_SOURCES = $(CPP_TARGETS)
CLEANFILES = $(CPP_TARGETS)

Dthello: Dthello.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=#  $< > $@
