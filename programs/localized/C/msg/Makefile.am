MAINTAINERCLEANFILES = Makefile.in

include ../../templates/C.am
include ../../templates/msg.C.am

# Setup a symlink from C -> en_US.UTF-8
install-data-hook:
	$(RM) -rf $(DESTDIR)$(CDE_INSTALLATION_TOP)/lib/nls/msg/en_US.UTF-8
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/lib/nls/msg/ && $(LN_S) C en_US.UTF-8
