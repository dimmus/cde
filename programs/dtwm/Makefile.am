MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtwm dtfplist

configdir = $(CDE_INSTALLATION_TOP)/config/C
dist_config_DATA = sys.dtwmrc

BUILT_SOURCES = sys.dtwmrc Dtwm.defs
CLEANFILES = sys.dtwmrc Dtwm.defs

AM_CPPFLAGS = -DLARGECURSORS -DR2_COMPAT -DOPAQUE -DSHAPE -DUNMAP_ON_RESTART \
	      -DBATCH_DRAG_REQUESTS -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
	      -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\"

dtwm_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)
dtfplist_LDADD = $(DTCLIENTLIBS) $(TIRPCLIB) $(XTOOLLIB)

if SOLARIS
dtwm_LDADD += -lintl -lresolv
dtfplist_LDADD += -lintl -lresolv
endif

dtwm_SOURCES = WmCDInfo.c      WmCDecor.c      WmCEvent.c \
               WmCPlace.c      WmColormap.c    WmError.c \
               WmEvent.c       WmEwmh.c        WmFeedback.c \
               WmFunction.c    WmGraphics.c    WmIDecor.c \
               WmIPlace.c      WmIconBox.c     WmImage.c \
               WmInitWs.c      WmKeyFocus.c    WmMain.c \
               WmManage.c      WmMenu.c        WmProperty.c \
               WmProtocol.c    WmResCvt.c      WmResParse.c \
               WmResource.c    WmSignal.c      WmWinConf.c \
               WmWinInfo.c     WmWinList.c     WmWinState.c \
               WmMultiHead.c   Button.c        Callback.c \
               Clock.c         DataBaseLoad.c  PanelS.c \
               Parse.c         PopupMenu.c     Session.c \
               UI.c            WmBackdrop.c    WmFP.c \
               WmHelp.c        WmIPC.c         WmInstantTitle.c \
               WmOL.c          WmParse.c       WmParseP.c \
               WmPresence.c    WmWrkspace.c    WmXSMP.c

dtfplist_SOURCES = Print.c Parse.c DataBaseLoad.c WmParse.c Session.c

SCRIPTFLAGS = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
              -DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP)

Dtwm.defs: Dtwm.defs.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $< > $@

sys.dtwmrc: sys.dtwmrc.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(SCRIPTFLAGS) $< > $@

