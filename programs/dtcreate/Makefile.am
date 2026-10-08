MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtcreate

dtcreate_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

dtcreate_SOURCES = main.c AddFiletype.c Confirmed.c			\
        CreateActionAppShell.c FileCharacteristics.c OpenFile.c		\
        UxXt.c ca_aux.c cmncbs.c cmnutils.c cmnrtns.c ErrorDialog.c	\
        fileio.c af_aux.c icon_selection_dialog.c parser.c		\
        AddFiletype.h Confirmed.h icon_selection_dialog.h af_aux.h	\
        CreateActionAppShell.h OpenFile.h ca_aux.h dtcreate.h		\
        parser.h cmncbs.h ErrorDialog.h UxXt.h cmnrtns.h		\
        FileCharacteristics.h cmnutils.h fileio.h
