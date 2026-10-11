MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtpad

dtpad_CFLAGS = $(CDE_CFLAGS) -DDT_LITE -DWORDWRAP

dtpad_LDADD = $(LIBPRINT) $(LIBHELP) $(LIBWIDGET) $(LIBSVC) $(LIBTT) \
	$(TIRPCLIB) $(XMLIB) $(XTLIB) $(X11LIB)

if SOLARIS
dtpad_LDADD += -lintl -lresolv
endif

dtpad_SOURCES = dtpad.c         editCB.c        fileCB.c \
		fileDlg.c       fileIo.c        formatCB.c \
		helpCB.c        helpDlg.c       main.c \
		optionsCB.c     printJob.c      printOutput.c \
		printSetup.c    session.c       ttMsgSupport.c \
		ttSaveSupport.c
