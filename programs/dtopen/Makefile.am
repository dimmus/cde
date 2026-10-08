MAINTAINERCLEANFILES = Makefile.in

dist_bin_SCRIPTS = dtopen

LOCAL_CPP_DEFINES = -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
		-DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP) \
		-DKORNSHELL=$(KSH)

BUILT_SOURCES = dtopen
CLEANFILES = $(BUILT_SOURCES)

dtopen: dtopen.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@

install-data-hook:
	$(RM) $(DESTDIR)$(bindir)/dtopen_*
	cd $(DESTDIR)$(bindir) && $(LN_S) dtopen dtopen_video
	cd $(DESTDIR)$(bindir) && $(LN_S) dtopen dtopen_image
	cd $(DESTDIR)$(bindir) && $(LN_S) dtopen dtopen_ps
	cd $(DESTDIR)$(bindir) && $(LN_S) dtopen dtopen_pdf
