MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libPreferences.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libPreferences_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

libPreferences_a_SOURCES = ../Prelude.h $(Preferences_SRCS)

RANLIB = echo
libPreferences_a_AR = $(LD) -r -o
