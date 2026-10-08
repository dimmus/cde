MAINTAINERCLEANFILES = Makefile.in

LOCAL_CPP_DEFINES = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
	-DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP) -DKORNSHELL=$(KSH) \
	-DDTKORNSHELL=$(CDE_INSTALLATION_TOP)/bin/dtksh

if SOLARIS
LOCAL_CPP_DEFINES += -D__sun
endif

BUILT_SOURCES = autoStart.dt compat.dt datatypes.dt develop.dt		\
	dtappman.dt dt.dt dtfile.dt dthelp.dt dthelpprint.dt		\
	dthelptag.dt dtims.dt dtinfo.dt dtpad.dt dtscreen.dt		\
	dt.softinit dtwm.fp error.ds hp.dt ibm.dt IconBrowse.dt		\
	laserrom.dt miscActions.dt miscImages.dt print.dt		\
	printerNN.dt sunOW.dt unsupported.dt user-prefs.dt uxstd.dt	\
	xclients.dt

CLEANFILES = $(BUILT_SOURCES)

# Use -C to GENCPP to preserve /* and */ sequences

autoStart.dt: autoStart.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

compat.dt: compat.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

datatypes.dt: datatypes.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

develop.dt: develop.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtappman.dt: dtappman.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dt.dt: dt.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtfile.dt: dtfile.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dthelp.dt: dthelp.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dthelpprint.dt: dthelpprint.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dthelptag.dt: dthelptag.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtims.dt: dtims.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtinfo.dt: dtinfo.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtpad.dt: dtpad.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtscreen.dt: dtscreen.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dt.softinit: dt.softinit.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

dtwm.fp: dtwm.fp.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

error.ds: error.ds.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

hp.dt: hp.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

ibm.dt: ibm.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

IconBrowse.dt: IconBrowse.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

laserrom.dt: laserrom.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

miscActions.dt: miscActions.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

miscImages.dt: miscImages.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

print.dt: print.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

printerNN.dt: printerNN.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

sunOW.dt: sunOW.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

unsupported.dt: unsupported.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

user-prefs.dt: user-prefs.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

uxstd.dt: uxstd.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

xclients.dt: xclients.dt.src
	$(RM) $@
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

