# Included by ../Makefile.am (%reldir% is helptag).

bin_SCRIPTS += %reldir%/dthelptag

CLEANFILES += %reldir%/dthelptag %reldir%/dthelptag.tmp

%reldir%/dthelptag: %reldir%/helptag.cpp
	$(AM_V_GEN)$(MKDIR_P) $(@D) && \
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && \
	chmod +x $@.tmp && mv -f $@.tmp $@
