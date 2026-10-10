MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

noinst_LIBRARIES = libQuery.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libQuery_a_CXXFLAGS = $(CDE_CXXFLAGS) $(DTINFO_DEFINES) $(DTINFO_INCLUDES)
libQuery_a_SOURCES = ../Prelude.h $(Query_SRCS)

RANLIB = echo
libQuery_a_AR = $(LD) -r -o

BUILT_SOURCES = Query.msg
CLEANFILES = $(BUILT_SOURCES)

Query.msg:
	cat $(Query_MSGS) > $@
