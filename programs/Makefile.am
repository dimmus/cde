MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = backdrops icons dsdm dtmail dtpad dtfile dtwm dtlogin		\
        dtsession dthello dtstyle dtexec dtdbcache dticon dtterm	\
        dtcalc dtaction dtspcd dtscreen dtksh dtcm dtsearchpath		\
        dtappbuilder palettes dtappintegrate dtprintegrate dtconfig	\
        dtcreate dtprintinfo fontaliases dtdspmsg ttsnoop dtimsstart	\
        dtpdm dtpdmd dtinfo types tttypes util dtopen localized	\
        dthelp

# The documentation tools (dthelp/parser is the other part of them)
if BUILD_DOC_TOOLS
SUBDIRS += dtsr dtdocbook
endif

install-data-hook:
	-if [ $(CDE_INSTALLATION_TOP)/bin != $(bindir) ];then \
		cd $(DESTDIR)$(CDE_INSTALLATION_TOP) && $(LN_S) $(bindir) bin ; fi
