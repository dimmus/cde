MAINTAINERCLEANFILES = Makefile.in

TT_VERSION = "CDE Version @CDE_VERSION_MAJOR@.@CDE_VERSION_MINOR@.@CDE_VERSION_MICRO@@CDE_VERSION_DEV@"
TT_VERSION_DEFINE = -DTT_VERSION_STRING=\"$(TT_VERSION)\"

noinst_LTLIBRARIES = libtttk.la

libtttk_la_CXXFLAGS = $(TT_VERSION_DEFINE) -I../ -I../../lib

libtttk_la_SOURCES = ttdesktop.C     ttdtfile.C      ttdtprocid.C \
	             ttmedia.C       tttk.C          tttk2free.C \
		     tttkmessage.C   tttkpattern.C   tttkutils.C
