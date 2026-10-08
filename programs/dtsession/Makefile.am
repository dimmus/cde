MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtsession

bin_SCRIPTS = dtsession_res

if HAS_PAM_LIBRARY
DTPAMSVCLIB = $(LIBPAMSVC)
else
DTPAMSVCLIB =
endif

dtsession_CFLAGS = -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
          -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
	  -Ddtsession

dtsession_SOURCES = SmMain.c SmHelp.c SmGlobals.c SmCommun.c		\
	SmError.c SmProperty.c SmProtocol.c SmSave.c SmScreen.c		\
	SmRestore.c SmUI.c SmWindow.c SmLock.c SrvPalette.c		\
	SrvFile_io.c SmStrDefs.c SmConMgmt.c SmXSMP.c SmAuth.c		\
	SmWatch.c SmProp.c SmDB.c OWsync.h SmGlobals.h SmProperty.h	\
	SmScreen.h SmXSMP.h SmAuth.h Sm.h SmProp.h SmUI.h		\
	SrvFile_io.h SmCommun.h SmHelp.h SmProtocol.h SmWatch.h Srv.h	\
	SmConMgmt.h SmHftRing.h SmResource.h SmWindow.h SrvPalette.h	\
	SmDB.h SmLock.h SmRestore.h SmXdef.h SmError.h			\
	SmMigResources.h SmSave.h SmXrm.h


dtsession_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB) $(DTPAMSVCLIB)

if SOLARIS
dtsession_SOURCES += OWsync.c
dtsession_LDADD += -lelf
endif

if LINUX
dtsession_CFLAGS += -DUSE_X11SSEXT
dtsession_LDADD += $(TIRPCLIB) -lXss
endif

if BSD
dtsession_CFLAGS += -DUSE_X11SSEXT
dtsession_LDADD +=  -lXss
endif

DTSM_CMD1 = $(CDE_INSTALLATION_TOP)/bin/dtaction OpenHomeDir
DTSM_CMD2 = $(CDE_INSTALLATION_TOP)/bin/dtfile -dir ~ -geometry +700+0
DTSM_CMD4 = $(CDE_INSTALLATION_TOP)/bin/dthelpview -helpVolume Intromgr
DTSM_CMD5 = $(CDE_INSTALLATION_TOP)/bin/dtaction Dthelpgen_delay

LOCAL_CPP_DEFINES = -DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP) \
		    -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
		    -DXRDB=$(XRDB) \
		    -DKORNSHELL=$(KSH) \
		    -DDTSM_CMD1="\"$(DTSM_CMD1)\"" \
		    -DDTSM_CMD2="\"$(DTSM_CMD2)\"" \
		    -DDTSM_CMD4="\"$(DTSM_CMD4)\"" \
		    -DDTSM_CMD5="\"$(DTSM_CMD5)\""

SUBDIRS = config

if LINUX
LOCAL_CPP_DEFINES += -Dlinux -D__linux__
endif

if FREEBSD
LOCAL_CPP_DEFINES += -DFREEBSD -D__FreeBSD__
endif

if SOLARIS
LOCAL_CPP_DEFINES += -Dsun
endif

if OPENBSD
LOCAL_CPP_DEFINES += -D__OpenBSD__
endif

if NETBSD
LOCAL_CPP_DEFINES += -D__NetBSD__
endif

if BSD
LOCAL_CPP_DEFINES += -DCSRG_BASED
endif


BUILT_SOURCES = Dtsession dtloadresources dtsession.msg sys.font	\
	sys.resources sys.resources.ive sys.session sys.session.ive
CLEANFILES = $(BUILT_SOURCES)

Dtsession: Dtsession.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtsession_res: dtloadresources
	$(RM) $@
	$(LN_S) $< $@

dtloadresources: dtloadresources.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

dtsession.msg: dtsession.msg.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

sys.font: sys.font.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

sys.resources: sys.resources.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

sys.resources.ive: sys.resources.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) -DIVE $< > $@

sys.session: sys.session.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

sys.session.ive: sys.session.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) -DIVE $< > $@


# systems without a PAM library need dtsession to be SUID root

install-exec-hook:
if !HAS_PAM_LIBRARY
	chown root $(DESTDIR)$(bindir)/dtsession
	chmod 4755 $(DESTDIR)$(bindir)/dtsession
endif

