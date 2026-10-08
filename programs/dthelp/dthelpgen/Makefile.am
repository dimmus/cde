MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dthelpgen

bin_SCRIPTS = dthelpgen.dtsh

dthelpgen_CPPFLAGS = -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
		   -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
		   -I ../../../lib/DtHelp

dthelpgen_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

dthelpgen_SOURCES = helpgen.c
