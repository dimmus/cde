MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dticon

dticon_CFLAGS = $(CDE_CFLAGS) -D__TOOLTALK

dticon_LDADD = $(LIBHELP) $(LIBWIDGET) $(LIBSVC) $(LIBTT) $(XMLIB) $(XTLIB) $(XPMLIB) $(X11LIB)

dticon_SOURCES = dtIconShell.c event.c event.h fileIO.c fileIO.h	\
                 fileIODialog.c globals.c graphics.c graphics.h		\
                 help.c image.c image.h main.c newIconDialog.c		\
                 process.c queryDialog.c stdErrDialog.c utils.c		\
                 utils.h constants.h externals.h help.h main.h
