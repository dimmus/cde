MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = init.c DtFuncs.dtsh

SUBDIRS = examples

kshdir = ${libdir}/dtksh

dist_ksh_SCRIPTS = DtFuncs.dtsh

KSH93SRC = ./ksh93

KSH93LIBSHELL = $(KSH93SRC)/lib/libshell.a

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

dtksh_LDADD = $(DTCLIENTLIBS) ksh93/src/cmd/ksh93/pmain.o $(XTOOLLIB) \
	      libshell.a $(KSH93SRC)/lib/libcmd.a $(KSH93SRC)/lib/libast.a \
	      $(KSH93SRC)/lib/libdll.a

if SOLARIS
dtksh_LDADD += -lsecdb -lsocket -lnsl
endif

if AIX
dtksh_CPPFLAGS += -D_IBMRPC_
dtksh_LDFLAGS = $(CDE_LDFLAGS) -bE:dtksh.exp
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

ksh93/bin/ksh:
	ksh93/bin/package flat make CCFLAGS='$(KSH93_SHOPTS)'

init.c: ksh93/bin/ksh
	$(CP) ksh93/src/cmd/ksh93/sh/init.c ./

libshell.a: $(KSH93SLIBSHELL) dtksh-init.o
	$(CP) $(KSH93LIBSHELL) libshell.a; \
	$(AR) d libshell.a init.o; \
	$(AR) cq libshell.a dtksh-init.o

check-local:
	./dtksh -c true && ksh93/bin/shtests --posix --utf8 SHELL=.libs/dtksh

clean-local:
	ksh93/bin/package clean

DtFuncs.dtsh:
	$(CP) DtFuncs.sh DtFuncs.dtsh
