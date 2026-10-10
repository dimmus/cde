MAINTAINERCLEANFILES = Makefile.in

# The documentation directories do not depend on each other, except that
# en_US.UTF-8/man and en_US.UTF-8/guides read a file that
# en_US.UTF-8/m-guides makes.  So instead of visiting them one after the
# other through SUBDIRS, "all", "install", "uninstall" and "clean" run a
# make in each of them as separate jobs of this one, which share its
# jobserver: with -jN the man pages, the help volumes and the information
# libraries of every language are built at the same time.

DOC_DIRS =

if BUILD_MAN_PAGES
DOC_DIRS += en_US.UTF-8/m-guides en_US.UTF-8/man
endif

if BUILD_INFOLIB
DOC_DIRS += en_US.UTF-8/m-guides en_US.UTF-8/guides
endif

if BUILD_HELP_DOCS
DOC_DIRS += en_US.UTF-8/help
endif

if GERMAN
if BUILD_HELP_DOCS
DOC_DIRS += de_DE.UTF-8/help
endif
if BUILD_INFOLIB
DOC_DIRS += de_DE.UTF-8/guides
endif
endif

if FRENCH
if BUILD_HELP_DOCS
DOC_DIRS += fr_FR.UTF-8/help
endif
if BUILD_INFOLIB
DOC_DIRS += fr_FR.UTF-8/guides
endif
endif

if SPANISH
if BUILD_HELP_DOCS
DOC_DIRS += es_ES.UTF-8/help
endif
if BUILD_INFOLIB
DOC_DIRS += es_ES.UTF-8/guides
endif
endif

if ITALIAN
if BUILD_HELP_DOCS
DOC_DIRS += it_IT.UTF-8/help
endif
if BUILD_INFOLIB
DOC_DIRS += it_IT.UTF-8/guides
endif
endif

if JAPANESE
if BUILD_HELP_DOCS
DOC_DIRS += ja_JP.UTF-8/help
endif
if BUILD_INFOLIB
DOC_DIRS += ja_JP.UTF-8/guides
endif
endif

# No recursion through SUBDIRS (see above); distclean, maintainer-clean
# and distdir still visit every directory.
SUBDIRS =
DIST_SUBDIRS = en_US.UTF-8/m-guides en_US.UTF-8/man en_US.UTF-8/guides	\
	en_US.UTF-8/help de_DE.UTF-8/help de_DE.UTF-8/guides		\
	fr_FR.UTF-8/help fr_FR.UTF-8/guides es_ES.UTF-8/help		\
	es_ES.UTF-8/guides it_IT.UTF-8/help it_IT.UTF-8/guides		\
	ja_JP.UTF-8/help ja_JP.UTF-8/guides

# One job per directory and goal: <goal>-in-<dir>, with the slashes of
# <dir> spelt "+" (a pattern without a slash matches only file names).
doc_jobs = $(subst /,+,$(sort $(DOC_DIRS)))
doc_dir = $(subst +,/,$*)

DOC_ALL = $(doc_jobs:%=all-in-%)
DOC_INSTALL = $(doc_jobs:%=install-in-%)
DOC_UNINSTALL = $(doc_jobs:%=uninstall-in-%)
DOC_CLEAN = $(doc_jobs:%=clean-in-%)
DOC_MOSTLYCLEAN = $(doc_jobs:%=mostlyclean-in-%)

.PHONY: $(DOC_ALL) $(DOC_INSTALL) $(DOC_UNINSTALL) $(DOC_CLEAN) \
	$(DOC_MOSTLYCLEAN)

all-local: $(DOC_ALL)
install-data-local: $(DOC_INSTALL)
uninstall-local: $(DOC_UNINSTALL)
clean-local: $(DOC_CLEAN)
mostlyclean-local: $(DOC_MOSTLYCLEAN)

$(DOC_ALL): all-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(doc_dir) all
$(DOC_INSTALL): install-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(doc_dir) install
$(DOC_UNINSTALL): uninstall-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(doc_dir) uninstall
$(DOC_CLEAN): clean-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(doc_dir) clean
$(DOC_MOSTLYCLEAN): mostlyclean-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $(doc_dir) mostlyclean

# en_US.UTF-8/man and en_US.UTF-8/guides read m-guides/common/ManLinks.sgm.
all-in-en_US.UTF-8+man all-in-en_US.UTF-8+guides: all-in-en_US.UTF-8+m-guides

install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/man
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/ && $(LN_S) $(mandir) man
