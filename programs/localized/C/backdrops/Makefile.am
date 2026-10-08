MAINTAINERCLEANFILES = Makefile.in

include ../../templates/C.am
include ../../templates/backdrops.am

# Setup a symlink from C -> en_US.UTF-8
install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/share/backdrops/desc.en_US.UTF-8
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/share/backdrops/ && $(LN_S) desc.C desc.en_US.UTF-8


