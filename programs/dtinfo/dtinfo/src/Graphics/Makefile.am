MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libGraphics.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libGraphics_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

libGraphics_a_SOURCES = ../Prelude.h $(Graphics_SRCS)

RANLIB = echo
libGraphics_a_AR = $(LD) -r -o
