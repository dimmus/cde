MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

LOCAL_CPP_DEFINES = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
-DCDE_DATA_TOP=$(CDE_DATA_TOP) \
	-DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP) \
	-DCDE_LOGFILES_TOP=$(CDE_LOGFILES_TOP) -DKORNSHELL=$(KSH) \
	-DDTKORNSHELL=$(CDE_INSTALLATION_TOP)/bin/dtksh

if SOLARIS
LOCAL_CPP_DEFINES += -D__sun
endif

# GENCPP expands no macro inside '...' or "...", and joins the lines of a
# continued field into one, so a path in an EXEC_STRING like sh -c '...'
# was installed as a literal CDE_INSTALLATION_TOP.  Substitute those
# afterwards.
QUOTED_DEFINES = -e 's|CDE_INSTALLATION_TOP|$(CDE_INSTALLATION_TOP)|g' \
	-e 's|CDE_DATA_TOP|$(CDE_DATA_TOP)|g' \
	-e 's|CDE_CONFIGURATION_TOP|$(CDE_CONFIGURATION_TOP)|g' \
	-e 's|CDE_LOGFILES_TOP|$(CDE_LOGFILES_TOP)|g' \
	-e 's|DTKORNSHELL|$(CDE_INSTALLATION_TOP)/bin/dtksh|g' \
	-e 's|KORNSHELL|$(KSH)|g'

BUILT_SOURCES = autoStart.dt compat.dt datatypes.dt develop.dt		\
	dtappman.dt dt.dt dtcm.dt dtfile.dt dthelp.dt dthelpprint.dt	\
	dthelptag.dt dtims.dt dtinfo.dt dtmail.dt dtpad.dt dtscreen.dt	\
	dt.softinit dtwm.fp error.ds hp.dt ibm.dt IconBrowse.dt		\
	laserrom.dt miscActions.dt miscImages.dt print.dt		\
	printerNN.dt sunOW.dt unsupported.dt user-prefs.dt uxstd.dt	\
	xclients.dt

CLEANFILES = $(BUILT_SOURCES) $(BUILT_SOURCES:=.tmp)

# Use -C to GENCPP to preserve /* and */ sequences

autoStart.dt: autoStart.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

compat.dt: compat.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

datatypes.dt: datatypes.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp
dtcm.dt: dtcm.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtmail.dt: dtmail.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp


develop.dt: develop.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtappman.dt: dtappman.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dt.dt: dt.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtfile.dt: dtfile.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dthelp.dt: dthelp.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dthelpprint.dt: dthelpprint.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dthelptag.dt: dthelptag.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtims.dt: dtims.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtinfo.dt: dtinfo.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtpad.dt: dtpad.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtscreen.dt: dtscreen.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dt.softinit: dt.softinit.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

dtwm.fp: dtwm.fp.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

error.ds: error.ds.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

hp.dt: hp.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

ibm.dt: ibm.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

IconBrowse.dt: IconBrowse.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

laserrom.dt: laserrom.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

miscActions.dt: miscActions.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

miscImages.dt: miscImages.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

print.dt: print.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

printerNN.dt: printerNN.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

sunOW.dt: sunOW.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

unsupported.dt: unsupported.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

user-prefs.dt: user-prefs.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

uxstd.dt: uxstd.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

xclients.dt: xclients.dt.src
	$(RM) $@ $@.tmp
	$(GENCPP) -P -C -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	$(SED) $(QUOTED_DEFINES) $@.tmp > $@ && $(RM) $@.tmp

