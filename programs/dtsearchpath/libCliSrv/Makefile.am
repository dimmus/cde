MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libCliSrv.a

libCliSrv_a_CXXFLAGS = $(CDE_CXXFLAGS) $(DT_INCDIR) \
	-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
	-DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
	-DCDE_LOGFILES_TOP=\"$(CDE_LOGFILES_TOP)\"

if SOLARIS
libCliSrv_a_CXXFLAGS += -DIOSTREAMSWORKS
libCliSrv_a_LIBADD = -lintl -lc -ladm
endif

libCliSrv_a_SOURCES = DirIterator.C Environ.C TTFile.C UnixEnv.C	\
	cstring.C cstring.h DirIterator.h Environ.h TTFile.h
