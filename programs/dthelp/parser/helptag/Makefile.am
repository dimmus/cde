MAINTAINERCLEANFILES = Makefile.in

bin_SCRIPTS = dthelptag

dthelptag: helptag.cpp
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@
	chmod +x $@
