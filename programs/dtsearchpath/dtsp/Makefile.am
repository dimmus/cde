MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtsearchpath

dtsearchpath_CPPFLAGS = $(CDE_CPPFLAGS) -I../libCliSrv $(DT_INCDIR)

if SOLARIS
dtsearchpath_CPPFLAGS += -DIOSTREAMSWORKS
endif

# It needs libtt (TTFile), and libtt needs Xt and X11 (it does not
# record them itself), but nothing else of the desktop and X libraries;
# linking them all made every run (one per login) load ~50 shared
# libraries.
dtsearchpath_LDADD = ../libCliSrv/libCliSrv.a $(LIBTT) $(TIRPCLIB) \
		     $(X_LIBS) -lXt -lX11

dtsearchpath_SOURCES = AppSearchPath.C DbSearchPath.C			\
		       HelpSearchPath.C IconSearchPath.C		\
		       InfoLibSearchPath.C ManSearchPath.C Options.C	\
		       SearchPath.C dtsearchpath.C Options.h		\
		       SearchPath.h
