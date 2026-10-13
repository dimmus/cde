include $(top_srcdir)/build-aux/cde-libs.am

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

SUBDIRS = dtcopy

bin_PROGRAMS = dtfile

BUILT_SOURCES = dtfile.config dtfile_error
CLEANFILES = dtfile.config dtfile_error

# Built here as the template that programs/localized merges with each
# language's messages; programs/localized/C installs the result into
# config/C.  Installing the template from here as well put its
# unexpanded %|nls-...| placeholders over the merged file whenever this
# directory was installed after programs/localized (make -C ... install).
dist_noinst_DATA = dtfile.config

dist_bin_SCRIPTS = dtfile_error dterror.ds

dtfile_CPPFLAGS = $(CDE_CPPFLAGS) -I$(srcdir)/dtcopy -DSHAPE -D_ILS_MACROS -DSUN_PERF \
		-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
		-DCDE_DATA_TOP=\"$(CDE_DATA_TOP)\" \
		-DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\"

# (made here by the .c.o rule, which leaves their dependency files, unused,
# in dtcopy/$(DEPDIR))
DISTCLEANFILES = dtcopy/$(DEPDIR)/sharedFuncs.Po dtcopy/$(DEPDIR)/fsrtns.Po

dtfile_LDADD = dtcopy/sharedFuncs.o \
	dtcopy/fsrtns.o \
	$(LIBHELP) $(LIBWIDGET) $(LIBSVC) $(LIBTT) $(LIBXIN) $(TIRPCLIB) \
	$(XMLIB) $(XTLIB) $(XEXTLIB) $(X11LIB)

if OPENBSD
dtfile_CPPFLAGS += -DFILE_MAP_OPTIMIZE
endif

if LINUX
dtfile_CPPFLAGS += -DFILE_MAP_OPTIMIZE
endif

if SOLARIS
dtfile_LDADD += -lintl -lresolv
endif

dtfile_SOURCES = ChangeDir.c     ChangeDirP.c    Command.c       Common.c \
		 Desktop.c       Directory.c     Encaps.c        File.c \
		 FileDialog.c    FileManip.c     FileMgr.c       FileOp.c \
		 Filter.c        FilterP.c       Find.c          FindP.c \
		 Help.c          HelpCB.c        HelpP.c         IconWindow.c \
		 IconicPath.c    Main.c          Menu.c          MkDir.c \
		 ModAttr.c       ModAttrP.c      MultiView.c     OverWrite.c \
		 Prefs.c         PrefsP.c        SharedMsgs.c    SharedProcs.c \
		 ToolTalk.c      Trash.c         Utils.c         fsDialog.c

# Mind the quoting here...
SCRIPTFLAGS = -DSHAPE -D_ILS_MACROS -DSUN_PERF \
	-DCDE_INSTALLATION_TOP="$(CDE_INSTALLATION_TOP)" \
	-DCDE_DATA_TOP="$(CDE_DATA_TOP)" \
	-DCDE_CONFIGURATION_TOP="$(CDE_CONFIGURATION_TOP)"

dtfile.config: dtfile.config.cpp
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $< > $@

dtfile_error: dtfile_error.cpp
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $<  > $@

dterror.ds: dterror.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $<  > $@
