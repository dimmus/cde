MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libOther.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libOther_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)
libOther_a_SOURCES = ../Prelude.h $(Other_SRCS)

RANLIB = echo
libOther_a_AR = $(LD) -r -o

BUILT_SOURCES = Other.msg
CLEANFILES = $(BUILT_SOURCES)

Other.msg:
	cat $(Other_MSGS) > $@
