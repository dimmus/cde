MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libBasic.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libBasic_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

libBasic_a_SOURCES = ../Prelude.h $(Basic_SRCS)

RANLIB = echo
libBasic_a_AR = $(LD) -r -o
