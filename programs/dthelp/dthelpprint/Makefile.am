MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dthelpprint
bin_SCRIPTS =  dthelpprint.sh

# FORTIFY_SOURCE_1: PrintTopics.c PrintHeadFootStr() finds the end of a
# header or footer with a %n in a format string it builds at run time,
# which _FORTIFY_SOURCE 2 and 3 abort on ("%n in writable segments
# detected") every time dthelpprint prints a page.
dthelpprint_CPPFLAGS = $(CDE_CPPFLAGS) $(FORTIFY_SOURCE_1) -I.. -I ../../../lib/DtHelp \
	-DCDE_CONFIGURATION_TOP='"${prefix}"' \
	-DCDE_INSTALLATION_TOP='"$(CDE_INSTALLATION_TOP)"'

dthelpprint_LDADD = $(LIBHELP) $(LIBSVC) $(X11LIB)

if SOLARIS
dthelpprint_LDADD += -lw -lgen
endif

dthelpprint_SOURCES = Main.c Initialize.c PrintUtil.c PrintTopics.c \
		      PrintManStrFile.c
