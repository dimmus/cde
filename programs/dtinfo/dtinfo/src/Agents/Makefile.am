MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libAgents.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libAgents_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)
libAgents_a_SOURCES = ../Prelude.h $(Agents_SRCS)

RANLIB = echo
libAgents_a_AR = $(LD) -r -o

BUILT_SOURCES = Agents.msg
CLEANFILES = $(BUILT_SOURCES)

Agents.msg:
	cat $(Agents_MSGS) > $@
