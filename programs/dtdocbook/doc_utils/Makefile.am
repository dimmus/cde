MAINTAINERCLEANFILES = Makefile.in

include ../dtdocbook.am

infolib = ../infolib

# SUBDIRS = $(infolib) ../dtsr ../instant

noinst_PROGRAMS = \
		  dtdocbook2infolib \
		  MixedGen \
		  NCFGen \
		  NodeParser \
		  StyleUpdate \
		  dbdrv \
		  valBase \
		  validator

noinst_SCRIPTS = dtdocbook2man dtdocbook2sdl

AM_CXXFLAGS = $(CDE_CXXFLAGS) -DCDE_NEXT -I$(top_builddir)/lib/DtMmdb
AM_LDFLAGS = $(CDE_LDFLAGS) $(DTCLIENTLIBS) $(XTOOLLIB)

dtdcbk_datarootdir = $(abs_top_srcdir)/programs/dtdocbook

AM_CPPFLAGS = $(CDE_CPPFLAGS) \
	      -DDTDCBK_DATAROOTDIR=\"$(dtdcbk_datarootdir)\" \
	      -DINFOLIB_LIBEXECDIR=\"$(PWD)\"

validator_SOURCES = validator.C
validator_LDFLAGS = $(CDE_LDFLAGS) $(LIBMMDB) $(LMDBLIB)

CLEANFILES = $(noinst_SCRIPTS) $(validator_SOURCES) dtdocbook2infolib.c

dtdocbook2infolib.c: ../dtdocbook2infolib.c
	$(RM) $@ && $(LN_S) $<

validator.C: $(infolib)/validator.C
	$(RM) $@ && $(LN_S) $<

MixedGen$(EXEEXT): $(infolib)/MixedGen$(EXEEXT)
	$(RM) $@ && $(LN_S) $<

NCFGen$(EXEEXT): $(infolib)/NCFGen$(EXEEXT)
	$(RM) $@ && $(LN_S) $<

NodeParser$(EXEEXT): $(infolib)/NodeParser$(EXEEXT)
	$(RM) $@ && $(LN_S) $<

StyleUpdate$(EXEEXT): $(infolib)/StyleUpdate$(EXEEXT)
	$(RM) $@ && $(LN_S) $<

dbdrv$(EXEEXT): $(infolib)/dbdrv$(EXEEXT)
	$(RM) $@ && $(LN_S) $<

valBase$(EXEEXT): $(infolib)/valBase$(EXEEXT)
	$(RM) $@ && $(LN_S) $<

dtdocbook2man: ../dtdocbook2man
	echo "#!/bin/sh" > $@ && chmod +x $@
	echo "dtdcbk_libexecdir=$(dtdcbk_datarootdir) \
		dtdcbk_datarootdir=$(dtdcbk_datarootdir) \
		sh $(PWD)/$< \$$@" >> $@

dtdocbook2sdl: ../dtdocbook2sdl
	echo "#!$(KSH)" > $@ && chmod +x $@
	echo "dtdcbk_libdir=$(dtdcbk_datarootdir) \
		dtdcbk_libexecdir=$(dtdcbk_datarootdir) \
		dtdcbk_datarootdir=$(dtdcbk_datarootdir) \
		$(KSH) $(PWD)/$< \$$@" >> $@

install:

distclean:
