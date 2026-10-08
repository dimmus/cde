MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = en_US.UTF-8

if GERMAN
SUBDIRS += de_DE.UTF-8
endif

if FRENCH
SUBDIRS += fr_FR.UTF-8
endif

if SPANISH
SUBDIRS += es_ES.UTF-8
endif

if ITALIAN
SUBDIRS += it_IT.UTF-8
endif

if JAPANESE
SUBDIRS += ja_JP.UTF-8
endif

install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/man
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/ && $(LN_S) $(mandir) man
