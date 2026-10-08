MAINTAINERCLEANFILES = Makefile.in

# This is only available for the C locale, it will be blank for other
# languages
APPBUILDER = AppBuilder.sdl

# this sets the LANG variables
include $(top_srcdir)/programs/localized/templates/English.am
# this does all the work
include $(top_srcdir)/doc/common/help/sdl-docs.am
# install all of the extra assets
include $(top_srcdir)/doc/common/help/sdl-data.am

# need to setup a symlink for the C locale to point toward en_US
install-data-hook:
	$(RM) -rf $(DESTDIR)$(CDE_INSTALLATION_TOP)/appconfig/help/C
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/appconfig/help/ && $(LN_S) en_US.UTF-8 C
