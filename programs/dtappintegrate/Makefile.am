MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

bin_SCRIPTS = dtappintegrate
BUILT_SOURCES = $(bin_SCRIPTS)
CLEANFILES = $(bin_SCRIPTS)

LOCAL_CPP_DEFINES = -DKORNSHELL=$(KSH)			\
	-DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP)	\
	-DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP)

dtappintegrate: dtappintegrate.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# -DSTAR='*' $(LOCAL_CPP_DEFINES) $< > $@
	chmod 755 $@

