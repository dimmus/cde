MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtsearchpath

dtsearchpath_CPPFLAGS = -I../libCliSrv $(DT_INCDIR)

if SOLARIS
dtsearchpath_CPPFLAGS += -DIOSTREAMSWORKS
endif

dtsearchpath_LDADD = ../libCliSrv/libCliSrv.a $(DTCLIENTLIBS) $(XTOOLLIB)

dtsearchpath_SOURCES = AppSearchPath.C DbSearchPath.C			\
		       HelpSearchPath.C IconSearchPath.C		\
		       InfoLibSearchPath.C ManSearchPath.C Options.C	\
		       SearchPath.C dtsearchpath.C Options.h		\
		       SearchPath.h
