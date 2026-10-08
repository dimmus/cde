MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dticon

dticon_CFLAGS = -D__TOOLTALK

dticon_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

dticon_SOURCES = dtIconShell.c event.c event.h fileIO.c fileIO.h	\
                 fileIODialog.c globals.c graphics.c graphics.h		\
                 help.c image.c image.h main.c newIconDialog.c		\
                 process.c queryDialog.c stdErrDialog.c utils.c		\
                 utils.h constants.h externals.h help.h main.h
