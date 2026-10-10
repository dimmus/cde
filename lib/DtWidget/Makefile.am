# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

unixdir = $(CDE_INSTALLATION_TOP)/app-defaults/C

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libDtWidget.la

# also used to preprocess UNIXbindings, which must not see $(CDE_CPPFLAGS)
WIDGET_DEFINES = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) $(DT_INCDIR)

AM_CPPFLAGS = $(CDE_CPPFLAGS) $(WIDGET_DEFINES)

libDtWidget_la_CFLAGS = $(CDE_CFLAGS) -DI18N_MSG -DMULTIBYTE

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtWidget_la_LDFLAGS = $(CDE_LDFLAGS) -version-info 2:1:0
else
libDtWidget_la_LDFLAGS = $(CDE_LDFLAGS) -version-info 3:0:1
endif

if SOLARIS
libDtWidget_la_CFLAGS += -DNO_REGCOMP
libDtWidget_la_LIBADD = -lw -lintl
endif


libDtWidget_la_SOURCES = ComboBox.c      Control.c       DialogBox.c \
			 DtMsgs.c        DtStrDefs.c     EditAreaData.c \
			 EditCalls.c     Editor.c        Icon.c \
			 MenuButton.c    NewCvt.c        SearchCalls.c \
			 ItemExtent.c \
			 SearchDlg.c     SpinBox.c       TitleBox.c \
			 View.c

# cpp this...
BUILT_SOURCES = UNIXbindings
CLEANFILES = UNIXbindings

UNIXbindings: UNIXbindings.cpp
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(WIDGET_DEFINES) $< > $@
