MAINTAINERCLEANFILES = Makefile.in

bin_SCRIPTS = dthelptag

CLEANFILES = dthelptag dthelptag.tmp

dthelptag: helptag.cpp
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp
	chmod +x $@.tmp
	mv -f $@.tmp $@
