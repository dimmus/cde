MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

CLEANFILES = init.c DtFuncs.dtsh libshell.a libshell.a.tmp ksh93.stamp

SUBDIRS = examples

kshdir = ${libdir}/dtksh

dist_ksh_SCRIPTS = DtFuncs.dtsh

KSH93SRC = ksh93

KSH93LIBSHELL = $(KSH93SRC)/lib/libshell.a
KSH93LIBS = $(KSH93SRC)/lib/libcmd.a $(KSH93SRC)/lib/libast.a \
	    $(KSH93SRC)/lib/libdll.a
KSH93PMAIN = $(KSH93SRC)/src/cmd/ksh93/pmain.o

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

dtksh_CPPFLAGS = $(CDE_CPPFLAGS) -I$(KSH93SRC)/include/ast -I$(KSH93SRC)/src/cmd/ksh93/include \
		 -I$(KSH93SRC)/src/cmd/ksh93 -I${srcdir}/lib/DtPrint \
		 -I${srcdir}/lib/DtHelp $(KSH93_SHOPTS) \
		 -D_PACKAGE_ast -DKSHELL \
		 '-DERROR_CONTEXT_T=Error_context_t' \
		 -D_API_ast=20100309 -DBUILD_DTKSH

dtksh_LDADD = $(KSH93PMAIN) libshell.a $(KSH93LIBS) \
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

dtksh_SOURCES = init.c \
                widget.c \
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
# over in that case.
#
# ksh93's build asks git for the commit and branch it is built from, to
# put the commit into the version string and to choose between a release
# and a development build; here it would find the CDE repository and
# record the CDE commit, so that every commit recompiled part of ksh93
# and dtksh (and ccache could not help).  GIT_CEILING_DIRECTORIES keeps
# git out of the CDE repository: ksh93 is then built as from a release
# tarball, the same in a git checkout as in a CDE tarball.
KSH93_CC = $(CC)
KSH93_CCFLAGS = $(strip $(KSH93_SHOPTS) $(HARDEN_CFLAGS) $(CFLAGS))
KSH93_JOBS = $(or $(patsubst -j%,%,$(filter -j%,$(MAKEFLAGS))),1)
KSH93_SRCFILES := $(shell find $(KSH93SRC)/src $(KSH93SRC)/bin/package -type f ! -name "*.o" 2>/dev/null)

ksh93.stamp: $(KSH93_SRCFILES) Makefile
	@cc='$(KSH93_CC)'; ccflags='$(KSH93_CCFLAGS)'; \
	gen=$(KSH93SRC)/lib/package/gen; \
	if test -f $$gen/CC && { test "`cat $$gen/CC`" != "$$cc" || \
	   test "`cat $$gen/CCFLAGS 2>/dev/null`" != "$$ccflags"; }; then \
		echo "ksh93: CC or CFLAGS changed, rebuilding it from scratch"; \
		$(KSH93SRC)/bin/package clean || exit 1; \
	fi; \
	echo "$(KSH93SRC)/bin/package flat make -j $(KSH93_JOBS) CC='$$cc' CCFLAGS='$$ccflags'"; \
	GIT_CEILING_DIRECTORIES='$(abs_srcdir)' \
	$(KSH93SRC)/bin/package flat make -j $(KSH93_JOBS) CC="$$cc" CCFLAGS="$$ccflags"
	@touch $@

$(KSH93SRC)/bin/ksh $(KSH93LIBSHELL) $(KSH93LIBS) $(KSH93PMAIN): ksh93.stamp
	@:

# Everything here includes headers that the ksh93 build generates.
init.c: $(KSH93SRC)/src/cmd/ksh93/sh/init.c | ksh93.stamp
	$(CP) $(KSH93SRC)/src/cmd/ksh93/sh/init.c $@

libshell.a: $(KSH93LIBSHELL) dtksh-init.o
	$(RM) $@.tmp
	$(CP) $(KSH93LIBSHELL) $@.tmp
	$(AR) d $@.tmp init.o
	$(AR) cq $@.tmp dtksh-init.o
	mv -f $@.tmp $@

check-local:
	./dtksh -c true && ksh93/bin/shtests --posix --utf8 SHELL=.libs/dtksh

clean-local:
	ksh93/bin/package clean

DtFuncs.dtsh: DtFuncs.sh
	$(CP) DtFuncs.sh DtFuncs.dtsh
