MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtcreate

AM_CPPFLAGS = $(CDE_CPPFLAGS) \
	-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
	-DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
	-DCDE_LOGFILES_TOP=\"$(CDE_LOGFILES_TOP)\"

dtcreate_LDADD = $(LIBHELP) $(LIBWIDGET) $(LIBSVC) $(LIBTT) $(XMLIB) $(XTLIB) $(X11LIB)

dtcreate_SOURCES = main.c AddFiletype.c Confirmed.c			\
        CreateActionAppShell.c FileCharacteristics.c OpenFile.c		\
        UxXt.c ca_aux.c cmncbs.c cmnutils.c cmnrtns.c ErrorDialog.c	\
        fileio.c af_aux.c icon_selection_dialog.c parser.c		\
        AddFiletype.h Confirmed.h icon_selection_dialog.h af_aux.h	\
        CreateActionAppShell.h OpenFile.h ca_aux.h dtcreate.h		\
        parser.h cmncbs.h ErrorDialog.h UxXt.h cmnrtns.h		\
        FileCharacteristics.h cmnutils.h fileio.h
