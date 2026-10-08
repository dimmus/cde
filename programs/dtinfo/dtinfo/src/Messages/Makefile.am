MAINTAINERCLEANFILES = Makefile.in

noinst_DATA = Messages.msg

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

BUILT_SOURCES = Messages.msg
CLEANFILES = $(BUILT_SOURCES)

Messages.msg:
	cat $(Messages_MSGS) > $@
