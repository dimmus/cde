MAINTAINERCLEANFILES = Makefile.in

include ../../templates/C.am
include ../../templates/palettes.am

# link the C version to en_US.UTF-8
install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/share/palettes/desc.en_US.UTF-8
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/share/palettes/ && $(LN_S) desc.C desc.en_US.UTF-8


