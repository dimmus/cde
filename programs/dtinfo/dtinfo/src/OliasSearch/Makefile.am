MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libOliasSearch.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libOliasSearch_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

libOliasSearch_a_SOURCES = ../Prelude.h $(OliasSearch_SRCS)

RANLIB = echo
libOliasSearch_a_AR = $(LD) -r -o
