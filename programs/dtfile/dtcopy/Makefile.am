MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtfile_copy

dtfile_copy_CFLAGS = $(CDE_CFLAGS) -DSHAPE

dtfile_copy_LDADD = $(LIBHELP) $(LIBSVC) $(XMLIB) $(XTLIB) $(X11LIB) $(TIRPCLIB)

dtfile_copy_SOURCES = main_dtcopy.c   copydialog.c    overwrtdialog.c dosync.c \
		      fsrtns.c  utils.c  errordialog.c   sharedFuncs.c
