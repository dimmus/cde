configdir = $(CDE_INSTALLATION_TOP)/config/C

SUBDIRS = dtcopy

bin_PROGRAMS = dtfile

BUILT_SOURCES = dtfile.config dtfile_error
CLEANFILES = dtfile.config dtfile_error

dist_config_DATA = dtfile.config

dist_bin_SCRIPTS = dtfile_error dterror.ds

dtfile_CPPFLAGS = -I./dtcopy -DSHAPE -D_ILS_MACROS -DSUN_PERF \
		-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
		-DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
		-DKORNSHELL=\"$(KSH)\"

dtfile_LDADD = dtcopy/sharedFuncs.o \
	dtcopy/fsrtns.o \
	$(DTCLIENTLIBS) $(TIRPCLIB) $(XTOOLLIB) 

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
	-DCDE_CONFIGURATION_TOP="$(CDE_CONFIGURATION_TOP)" \
	-DKORNSHELL="$(KSH)"

dtfile.config: dtfile.config.cpp
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $< > $@

dtfile_error: dtfile_error.cpp
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $<  > $@

dterror.ds: dterror.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $<  > $@
