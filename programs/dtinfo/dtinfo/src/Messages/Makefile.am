MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

noinst_DATA = Messages.msg

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

BUILT_SOURCES = Messages.msg
CLEANFILES = $(BUILT_SOURCES)

Messages.msg:
	cat $(Messages_MSGS) > $@
