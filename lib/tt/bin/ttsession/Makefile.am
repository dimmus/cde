MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

bin_PROGRAMS = ttsession

ttsession_CXXFLAGS = -I../../slib -I../../lib $(TT_VERSION_DEFINE)

ttsession_LDADD = $(LIBTT) $(X_LIBS) ../../slib/libstt.a

if LINUX
ttsession_LDADD += $(TIRPCLIB) $(XTOOLLIB)
endif

if SOLARIS
ttsession_LDADD += $(XTOOLLIB) -lintl -lsocket -lnsl
endif

if BSD
ttsession_LDADD += $(XTOOLLIB)
endif

ttsession_SOURCES = mp_server.C
