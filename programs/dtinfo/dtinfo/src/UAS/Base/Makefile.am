MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

noinst_LIBRARIES = libBase.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libBase_a_CXXFLAGS = $(CDE_CXXFLAGS) -I.. -I ../.. -I../Support $(DTINFO_DEFINES)	\
	$(DTINFO_INCLUDES)
libBase_a_SOURCES = $(Base_SRCS)

RANLIB = echo
libBase_a_AR = $(LD) -r -o

BUILT_SOURCES = Base.msg
CLEANFILES = $(BUILT_SOURCES)

Base.msg:
	cat $(Base_MSGS) > $@
