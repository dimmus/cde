MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = dfiles pmaker msgsets

noinst_SCRIPTS = treeres

LOCAL_SED_DEFINES = -e 's|PERL|$(PERL)|g'

dfiles_SOURCES = dfiles.c

pmaker_SOURCES = pmaker.c

# JET - this one migth need to be regenned via the lex file someday...
# Maybe it should always be regenned?
msgsets_SOURCES = msgsets.C

treeres: treeres.src
	$(RM) $@
	$(SED) $(LOCAL_SED_DEFINES) $< > $@
	chmod +x $@
