MAINTAINERCLEANFILES = Makefile.in

dist_bin_SCRIPTS = dtconfig

configdir = $(CDE_INSTALLATION_TOP)/config
dist_config_DATA = dtlogin.rc dtlogin.xml

SCRIPTFLAGS = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
              -DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP)

BUILT_SOURCES = dtconfig dtlogin.rc
CLEANFILES = $(BUILT_SOURCES)

dtconfig: dtconfig.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

dtlogin.rc: dtlogin.rc.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

