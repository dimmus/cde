# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/Messages.msg: $(Messages_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/Messages.msg
