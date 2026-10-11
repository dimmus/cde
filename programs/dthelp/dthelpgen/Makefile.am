MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dthelpgen

bin_SCRIPTS = dthelpgen.dtsh

dthelpgen_CPPFLAGS = $(CDE_CPPFLAGS) -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
		   -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
		   -I ../../../lib/DtHelp

dthelpgen_LDADD = $(LIBHELP) $(LIBSVC) $(X11LIB)

dthelpgen_SOURCES = helpgen.c
