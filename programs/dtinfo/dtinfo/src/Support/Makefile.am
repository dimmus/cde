MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libSupport.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libSupport_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

libSupport_a_SOURCES = ../Prelude.h $(Support_SRCS)

RANLIB = echo
libSupport_a_AR = $(LD) -r -o
