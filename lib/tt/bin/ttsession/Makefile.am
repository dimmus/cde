MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

bin_PROGRAMS = ttsession

ttsession_CXXFLAGS = $(CDE_CXXFLAGS) -I../../slib -I../../lib $(TT_VERSION_DEFINE)

ttsession_LDADD = $(LIBTT) ../../slib/libstt.a

if LINUX
ttsession_LDADD += $(TIRPCLIB)
endif

if SOLARIS
ttsession_LDADD += -lintl -lsocket -lnsl
endif

ttsession_SOURCES = mp_server.C
