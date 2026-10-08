MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libBase.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libBase_a_CXXFLAGS = -I.. -I ../.. -I../Support $(DTINFO_DEFINES)	\
	$(DTINFO_INCLUDES)
libBase_a_SOURCES = $(Base_SRCS)

RANLIB = echo
libBase_a_AR = $(LD) -r -o

BUILT_SOURCES = Base.msg
CLEANFILES = $(BUILT_SOURCES)

Base.msg:
	cat $(Base_MSGS) > $@
