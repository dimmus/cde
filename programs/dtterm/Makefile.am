MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

bin_PROGRAMS = dtterm

dtterm_LDADD = $(LIBTERM) $(LIBSVC) $(LIBTT) $(XMLIB) $(XTLIB) $(X11LIB)

if BSD
dtterm_LDADD += -lutil
endif

if HAS_UTEMPTER_LIBRARY
dtterm_LDADD += -lutempter
endif

dtterm_CFLAGS = $(CDE_CFLAGS) -I$(top_srcdir)/lib/DtTerm/TermPrim	\
	-I$(top_srcdir)/lib/DtTerm/Term			\
	-I$(top_srcdir)/lib/DtTerm/TermView	\
	$(DT_INCDIR) \
	-DSUN_TERMINAL_SERVER

dtterm_SOURCES = DtTermLogit.c DtTermMain.c DtTermFallBackResources.c	\
		 sunDtTermServer.c DtTermSyntax.c DtTermLogit.h		\
		 DtTermMain.h DtTermServer.h DtTermSyntax.h

CPP_TARGETS = dtterm.ti DtTermFallBackResources.c.src	\
	DtTermFallBackResources.c Dtterm.ad

BUILT_SOURCES = $(CPP_TARGETS)
CLEANFILES = $(CPP_TARGETS)

TRADCPP = $(top_builddir)/util/tradcpp/tradcpp

$(TRADCPP):
	$(MAKE) -C $(top_builddir)/util/tradcpp tradcpp

cfgdir = $(CDE_INSTALLATION_TOP)/config
dist_cfg_DATA = dtterm.ti

Dtterm.ad: Dtterm.ad.src $(TRADCPP)
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $< > $@

DtTermFallBackResources.c.src: Dtterm.ad.src $(TRADCPP)
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# -DFALLBACK_RESOURCES $< > $@

DtTermFallBackResources.c: DtTermFallBackResources.c.src
	$(RM) $@
	$(srcdir)/mkfallbk < DtTermFallBackResources.c.src > $@

dtterm.ti: terminfoChecklist $(TRADCPP)
	$(RM) $@
	CPP="$(GENCPP)" $(SHELL) $(srcdir)/terminfoCreate < $< > $@

install-exec-hook:
if !HAS_UTEMPTER_LIBRARY
	-chown root $(DESTDIR)$(bindir)/dtterm
	-chmod 4755 $(DESTDIR)$(bindir)/dtterm
endif
