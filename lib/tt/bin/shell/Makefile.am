MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

bin_PROGRAMS = ttmv ttrm ttcp ttrmdir

AM_CXXFLAGS = -I../../lib $(TT_VERSION_DEFINE)

LDADD = $(LIBTT) $(TIRPCLIB) $(XTOOLLIB)

if SOLARIS
LDADD += -lintl -lsocket -lnsl
endif

ttmv_SOURCES = ttmv.C mover.C

ttrm_SOURCES = ttrm.C remover.C

ttcp_SOURCES = ttcp.C copier.C

ttrmdir_SOURCES = ttrm.C remover.C
