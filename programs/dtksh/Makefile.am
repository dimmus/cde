MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

CLEANFILES = init.c ksh93-init.$(OBJEXT) DtFuncs.dtsh libshell.a libshell.a.tmp ksh93.stamp \
	ksh93-cc ksh93-cc.tmp

SUBDIRS = examples

kshdir = ${libdir}/dtksh

dist_ksh_SCRIPTS = DtFuncs.dtsh

KSH93SRC = ksh93

# The ksh93 sources.  ksh93's bin/package builds inside the ksh93 tree
# ($(KSH93SRC) above); in a VPATH build, where the source directory may
# be read-only, the ksh93.stamp rule first copies the sources there.
KSH93SRCDIR = $(srcdir)/ksh93

# ksh93's bin/package builds in $(KSH93SRC)/arch/HOSTTYPE (ignored by
# ksh93's own .gitignore) and installs its libraries and headers there.
# It is run without "flat", which would also link every file it builds
# into bin/, lib/, include/ and the source directories of $(KSH93SRC).
#
# bin/package runs the first word of CC as the compiler, so a CC of more
# than one word (CC='ccache cc') becomes a one-word wrapper script that
# runs it: ksh93-cc below, and a temporary one for "package host type",
# which reads the size of a pointer from the compiler.
KSH93_HOSTTYPE := $(shell \
	if test -n '$(word 2,$(CC))'; then \
		t=`mktemp` && printf '\043!/bin/sh\nexec %s "$$@"\n' '$(CC)' > $$t && \
		chmod +x $$t && CC=$$t $(KSH93SRCDIR)/bin/package host type; \
		rm -f $$t; \
	else \
		CC='$(CC)' $(KSH93SRCDIR)/bin/package host type; \
	fi 2>/dev/null)
KSH93ROOT = $(KSH93SRC)/arch/$(KSH93_HOSTTYPE)

KSH93LIBSHELL = $(KSH93ROOT)/lib/libshell.a
KSH93LIBS = $(KSH93ROOT)/lib/libcmd.a $(KSH93ROOT)/lib/libast.a \
	    $(KSH93ROOT)/lib/libdll.a
KSH93PMAIN = $(KSH93ROOT)/src/cmd/ksh93/pmain.o

bin_PROGRAMS = dtksh

BUILT_SOURCES = init.c

# Put all SHOPT definitions here.  It is vital that dtksh and ksh93
# are built with the same options.  We pass this list to both the
# dtksh components and to the ksh93 build as well.  They must be
# specifically assigned a 1 or various preprocessor checks (#if SHOPT
# ...) in ksh93 will fail.
KSH93_SHOPTS = -DSHOPT_DYNAMIC=1 -DSHOPT_NAMESPACE=1 -DSHOPT_MULTIBYTE=1 \
	-DSHOPT_STATS=1 -DSHOPT_BGX=1 -DSHOPT_AUDIT=1 \
	-DSHOPT_KIA=0 -DSHOPT_HISTEXPAND=1 -DSHOPT_EDPREDICT=1 \
	-DSHOPT_ESH=1 -DSHOPT_VSH=1 -DSHOPT_FIXEDARRAY=1 -DSHOPT_SUID_EXEC=1 \
	-DSHOPT_BRACEPAT=1 -DSHOPT_ECHOPRINT=1 -D_BLD_ksh

dtksh_CPPFLAGS = $(CDE_CPPFLAGS) -I$(KSH93ROOT)/include/ast -I$(KSH93SRC)/src/cmd/ksh93/include \
		 -I$(KSH93ROOT)/src/cmd/ksh93 -I$(KSH93SRC)/src/cmd/ksh93 -I${srcdir}/lib/DtPrint \
		 -I${srcdir}/lib/DtHelp $(KSH93_SHOPTS) \
		 -D_PACKAGE_ast -DKSHELL \
		 '-DERROR_CONTEXT_T=Error_context_t' \
		 -D_API_ast=20100309 -DBUILD_DTKSH

dtksh_LDADD = ksh93-init.$(OBJEXT) $(KSH93PMAIN) libshell.a $(KSH93LIBS) \
	      $(LIBPRINT) $(LIBHELP) $(LIBSVC) $(LIBTT) \
	      $(XMLIB) $(XTLIB) $(X11LIB) $(MATHLIB)

if SOLARIS
dtksh_LDADD += -lsecdb -lsocket -lnsl
endif

dtksh_LDFLAGS = $(CDE_LDFLAGS)

if AIX
dtksh_CPPFLAGS += -D_IBMRPC_
dtksh_LDFLAGS += -bE:dtksh.exp
endif

dtksh_SOURCES = widget.c \
                dtkcvt.c \
                dtkcmds.c \
                XtCvtrs.c \
                xmcvt.c \
                xmcmds.c \
                xmwidgets.c \
                extra.c \
                msgs.c

# ksh93 has its own build system (bin/package and mamake).  Run it when a
# ksh93 source file or this Makefile (which carries CC and CFLAGS) has
# changed; mamake then rebuilds only what is out of date.  It gets the
# compiler and CFLAGS of the rest of the tree and the job count of this
# make (-jN in MAKEFLAGS; mamake does not take part in the jobserver, but
# nothing else is built while it runs).  bin/package refuses to continue
# when CC or CCFLAGS differ from those of the previous build, so start
# over in that case.  A flat view left by an older build (lib/package/gen
# in $(KSH93SRC), which makes bin/package keep updating it) is removed
# first.
#
# ksh93's build asks git for the commit and branch it is built from, to
# put the commit into the version string and to choose between a release
# and a development build; here it would find the CDE repository and
# record the CDE commit, so that every commit recompiled part of ksh93
# and dtksh (and ccache could not help).  GIT_CEILING_DIRECTORIES keeps
# git out of the CDE repository (from the sources, or from their copy in a
# VPATH build directory): ksh93 is then built as from a release
# tarball, the same in a git checkout as in a CDE tarball.
KSH93_CC = $(if $(word 2,$(CC)),$(abs_builddir)/ksh93-cc,$(CC))
# FILE_PREFIX_CFLAGS: ksh93's __FILE__ strings and debug information
# name its sources relative to the tree, as everywhere else.
KSH93_CCFLAGS = $(strip $(KSH93_SHOPTS) $(HARDEN_CFLAGS) $(FILE_PREFIX_CFLAGS) \
	$(CFLAGS))
KSH93_JOBS = $(or $(patsubst -j%,%,$(filter -j%,$(MAKEFLAGS))),1)
KSH93_SRCFILES := $(shell find $(KSH93SRCDIR)/src $(KSH93SRCDIR)/bin/package -type f ! -name "*.o" 2>/dev/null)

ksh93-cc: Makefile
	$(AM_V_GEN)printf '#!/bin/sh\nexec %s "$$@"\n' '$(CC)' > $@.tmp && \
	chmod +x $@.tmp && mv -f $@.tmp $@

ksh93.stamp: $(KSH93_SRCFILES) Makefile $(if $(word 2,$(CC)),ksh93-cc)
	@cc='$(KSH93_CC)'; ccflags='$(KSH93_CCFLAGS)'; \
	if test "$(srcdir)" != .; then \
		echo "ksh93: copying the sources into the build directory"; \
		$(MKDIR_P) $(KSH93SRC) && \
		(cd $(KSH93SRCDIR) && tar --exclude=./arch -cf - .) | \
		(cd $(KSH93SRC) && tar -xpf -) && \
		chmod -R u+w $(KSH93SRC) || exit 1; \
	fi; \
	if test -d $(KSH93SRC)/lib/package/gen; then \
		echo "ksh93: removing the flat view of an older build"; \
		$(KSH93SRC)/bin/package clean flat || exit 1; \
	fi; \
	gen=$(KSH93ROOT)/lib/package/gen; \
	if test -f $$gen/CC && { test "`cat $$gen/CC`" != "$$cc" || \
	   test "`cat $$gen/CCFLAGS 2>/dev/null`" != "$$ccflags"; }; then \
		echo "ksh93: CC or CFLAGS changed, rebuilding it from scratch"; \
		$(KSH93SRC)/bin/package clean || exit 1; \
	fi; \
	echo "$(KSH93SRC)/bin/package make -j $(KSH93_JOBS) CC='$$cc' CCFLAGS='$$ccflags'"; \
	GIT_CEILING_DIRECTORIES='$(abs_srcdir):$(abs_builddir)' \
	$(KSH93SRC)/bin/package make -j $(KSH93_JOBS) CC="$$cc" CCFLAGS="$$ccflags"
	@touch $@

$(KSH93ROOT)/bin/ksh $(KSH93LIBSHELL) $(KSH93LIBS) $(KSH93PMAIN): ksh93.stamp
	@:

# Everything here includes headers that the ksh93 build generates.
init.c: $(KSH93SRC)/src/cmd/ksh93/sh/init.c | ksh93.stamp
	$(CP) $(KSH93SRC)/src/cmd/ksh93/sh/init.c $@

# ksh93's init.c, built for dtksh (BUILD_DTKSH) with dtksh's flags but
# without C_EXTRA_WARNINGS (the curated -Wextra list and -Werror, see
# configure.ac): it is ksh93's code, like the rest of libshell, which
# ksh93's own build compiles without them.
ksh93-init.$(OBJEXT): init.c
	$(AM_V_CC)$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(dtksh_CPPFLAGS) \
		$(CPPFLAGS) $(filter-out $(C_EXTRA_WARNINGS),$(CDE_CFLAGS)) \
		$(CFLAGS) -c -o $@ init.c

libshell.a: $(KSH93LIBSHELL) ksh93-init.$(OBJEXT)
	$(RM) $@.tmp
	$(CP) $(KSH93LIBSHELL) $@.tmp
	$(AR) d $@.tmp init.o
	$(AR) cq $@.tmp ksh93-init.$(OBJEXT)
	mv -f $@.tmp $@

# "make check" (and so make distcheck) runs dtksh once.  The ksh93
# regression suite is "make shtests": run on dtksh, its builtins test
# fails by design (dtksh's ToolTalk and Xt builtins do not take --version
# as ksh93's own do), and its path and timetype tests depend on the PATH
# and the time zone of the machine.  (shtests is run by absolute name:
# it re-executes itself through bin/package, which changes directory.)
check-local:
	./dtksh -c 'print -r -- "$${.sh.version}"'

shtests: dtksh
	$(abs_builddir)/$(KSH93SRC)/bin/shtests --posix --utf8 SHELL=$(abs_builddir)/.libs/dtksh

.PHONY: shtests

clean-local:
	if test -x $(KSH93SRC)/bin/package; then $(KSH93SRC)/bin/package clean; fi

# the copy of the ksh93 sources that a VPATH build made
distclean-local: clean-local
	if test "$(srcdir)" != .; then rm -rf $(KSH93SRC); fi

DtFuncs.dtsh: DtFuncs.sh
	$(CP) $(srcdir)/DtFuncs.sh $@
