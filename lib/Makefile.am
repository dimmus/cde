MAINTAINERCLEANFILES = Makefile.in

# The libraries are built by a make in each directory, but not one
# directory after the other through SUBDIRS: "all", "install", "uninstall"
# and "clean" run a make in each of them as separate jobs of this one,
# which share its jobserver, ordered only by the libraries each one links
# (the _LIBADD of its Makefile.am).  With -jN, libDtMmdb (which needs no
# other) builds while the chain libtt, libDtSvc, libDtHelp, ... does, and
# a directory's last files no longer hold up the next directory.
# (doc/Makefile.am does the same.)

LIB_DIRS = DtXinerama tt DtSvc DtSearch DtWidget DtHelp DtPrint DtTerm \
	   DtMrm csa DtMmdb

# No recursion through SUBDIRS; distclean, maintainer-clean and distdir
# still visit every directory.
SUBDIRS =
DIST_SUBDIRS = $(LIB_DIRS)

# One job per directory and goal: <goal>-in-<dir>.
LIB_ALL = $(LIB_DIRS:%=all-in-%)
LIB_INSTALL = $(LIB_DIRS:%=install-in-%)
LIB_UNINSTALL = $(LIB_DIRS:%=uninstall-in-%)
LIB_CLEAN = $(LIB_DIRS:%=clean-in-%)
LIB_MOSTLYCLEAN = $(LIB_DIRS:%=mostlyclean-in-%)

.PHONY: $(LIB_ALL) $(LIB_INSTALL) $(LIB_UNINSTALL) $(LIB_CLEAN) \
	$(LIB_MOSTLYCLEAN)

all-local: $(LIB_ALL)
install-exec-local: $(LIB_INSTALL)
uninstall-local: $(LIB_UNINSTALL)
clean-local: $(LIB_CLEAN)
mostlyclean-local: $(LIB_MOSTLYCLEAN)

$(LIB_ALL): all-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $* all
$(LIB_INSTALL): install-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $* install
$(LIB_UNINSTALL): uninstall-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $* uninstall
$(LIB_CLEAN): clean-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $* clean
$(LIB_MOSTLYCLEAN): mostlyclean-in-%:
	@$(MAKE) $(AM_MAKEFLAGS) -C $* mostlyclean

# Which library links which (and so must be built, and installed, first:
# libtool relinks a library against the installed ones it uses).
LIB_DEPS_DtSvc = tt DtXinerama
LIB_DEPS_DtSearch = DtSvc
LIB_DEPS_DtWidget = DtSvc
LIB_DEPS_DtHelp = DtSvc
LIB_DEPS_DtPrint = DtHelp DtSvc
LIB_DEPS_DtTerm = DtHelp DtSvc
LIB_DEPS_DtMrm = DtHelp DtPrint DtTerm DtWidget
LIB_DEPS_csa = DtSvc

all-in-DtSvc: $(LIB_DEPS_DtSvc:%=all-in-%)
all-in-DtSearch: $(LIB_DEPS_DtSearch:%=all-in-%)
all-in-DtWidget: $(LIB_DEPS_DtWidget:%=all-in-%)
all-in-DtHelp: $(LIB_DEPS_DtHelp:%=all-in-%)
all-in-DtPrint: $(LIB_DEPS_DtPrint:%=all-in-%)
all-in-DtTerm: $(LIB_DEPS_DtTerm:%=all-in-%)
all-in-DtMrm: $(LIB_DEPS_DtMrm:%=all-in-%)
all-in-csa: $(LIB_DEPS_csa:%=all-in-%)

install-in-DtSvc: $(LIB_DEPS_DtSvc:%=install-in-%)
install-in-DtSearch: $(LIB_DEPS_DtSearch:%=install-in-%)
install-in-DtWidget: $(LIB_DEPS_DtWidget:%=install-in-%)
install-in-DtHelp: $(LIB_DEPS_DtHelp:%=install-in-%)
install-in-DtPrint: $(LIB_DEPS_DtPrint:%=install-in-%)
install-in-DtTerm: $(LIB_DEPS_DtTerm:%=install-in-%)
install-in-DtMrm: $(LIB_DEPS_DtMrm:%=install-in-%)
install-in-csa: $(LIB_DEPS_csa:%=install-in-%)
