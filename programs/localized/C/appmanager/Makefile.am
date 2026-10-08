MAINTAINERCLEANFILES = Makefile.in

include ../../templates/C.am
include ../../templates/appmgr.am

# Setup a symlink from C -> en_US.UTF-8
install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/appconfig/appmanager/en_US.UTF-8
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/appconfig/appmanager/ && $(LN_S) C en_US.UTF-8
