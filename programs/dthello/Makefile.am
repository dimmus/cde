MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dthello

dthello_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

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
