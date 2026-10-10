MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dthelpdemo

dthelpdemo_CPPFLAGS = $(CDE_CPPFLAGS) -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
		   -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\"

dthelpdemo_LDADD = $(LIBHELP) $(LIBSVC) $(XMLIB) $(XTLIB) $(X11LIB)

dthelpdemo_SOURCES = Main.c HourGlass.c HelpCache.c
