MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtterm

dtterm_LDADD = $(LIBTERM) $(DTCLIENTLIBS) $(XTOOLLIB) 

if BSD
dtterm_LDADD += -lutil
endif

if HAS_UTEMPTER_LIBRARY
dtterm_LDADD += -lutempter
endif

dtterm_CFLAGS = -I$(top_builddir)/lib/DtTerm/TermPrim	\
	-I$(top_builddir)/lib/DtTerm/Term			\
	-I$(top_builddir)/lib/DtTerm/TermView	\
	$(DT_INCDIR) \
	-DSUN_TERMINAL_SERVER

dtterm_SOURCES = DtTermLogit.c DtTermMain.c DtTermFallBackResources.c	\
		 sunDtTermServer.c DtTermSyntax.c DtTermLogit.h		\
		 DtTermMain.h DtTermServer.h DtTermSyntax.h

CPP_TARGETS = dtterm.ti DtTermFallBackResources.c.src	\
	DtTermFallBackResources.c Dtterm.ad

BUILT_SOURCES = $(CPP_TARGETS)
CLEANFILES = $(CPP_TARGETS)

cfgdir = $(CDE_INSTALLATION_TOP)/config
dist_cfg_DATA = dtterm.ti

Dtterm.ad: Dtterm.ad.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $< > $@

DtTermFallBackResources.c.src: Dtterm.ad.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# -DFALLBACK_RESOURCES $< > $@

DtTermFallBackResources.c: DtTermFallBackResources.c.src
	$(RM) $@
	./mkfallbk < DtTermFallBackResources.c.src > $@

dtterm.ti: terminfoChecklist
	$(RM) $@
	CPP="$(GENCPP)" $(SHELL) terminfoCreate < terminfoChecklist > $@

install-exec-hook:
if !HAS_UTEMPTER_LIBRARY
	chown root $(DESTDIR)$(bindir)/dtterm
	chmod 4755 $(DESTDIR)$(bindir)/dtterm
endif
	@TIC@ dtterm.ti
