MAINTAINERCLEANFILES = Makefile.in

fontaliasesconfdir = $(sysconfdir)/$(PACKAGE_TARNAME)/fontaliases
fontaliasesdatadir = $(pkgdatadir)/fontaliases

BUILT_SOURCES = dtinfo.pcf.gz fixed.alias mixed.alias fonts.dir
fontaliasesdata_DATA = $(BUILT_SOURCES)
CLEANFILES = $(BUILT_SOURCES)

mk_fonts_alias_SOURCES = mk_fonts_alias.c
test_fonts_alias_SOURCES = test_fonts_alias.c
test_fonts_alias_LDADD = $(XTOOLLIB)
noinst_PROGRAMS = mk_fonts_alias test_fonts_alias

if MISC_FIXED
FONTS_ALIAS = fixed.alias
else
FONTS_ALIAS = mixed.alias
endif

dtinfo.pcf.gz: dtinfo.bdf
	$(RM) $@
	$(BDFTOPCF) -t $< |$(GZIP) > $@

fixed.alias: mk_fonts_alias
	./$< > $@

mixed.alias: mk_fonts_alias
	./$< 1 > $@

fonts.dir: dtinfo.pcf.gz
	$(MKFONTDIR) -x bdf .

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(fontaliasesconfdir)
	$(CP) $(DESTDIR)$(fontaliasesdatadir)/$(FONTS_ALIAS) $(DESTDIR)$(fontaliasesconfdir)/fonts.alias
