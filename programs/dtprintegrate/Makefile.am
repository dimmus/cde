MAINTAINERCLEANFILES = Makefile.in

bin_SCRIPTS = dtprintegrate dtlp dtlpsetup

LOCAL_CPP_DEFINES = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
		-DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP)   \
		-DDTKORNSHELL="$(bindir)/dtksh"  \
		-DKORNSHELL=$(KSH)

BUILT_SOURCES = dtprintegrate dtlp dtlpsetup
CLEANFILES = $(BUILT_SOURCES)

dtprintegrate: dtprintegrate.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

dtlp: dtlp.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

dtlpsetup: dtlpsetup.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

