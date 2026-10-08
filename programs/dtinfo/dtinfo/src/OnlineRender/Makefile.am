MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libOnlineRender.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libOnlineRender_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)
libOnlineRender_a_SOURCES = ../Prelude.h CanvasRenderer.C	\
	FeatureSupport.C FontCache.C SegClientData.C

RANLIB = echo
libOnlineRender_a_AR = $(LD) -r -o

BUILT_SOURCES = OnlineRender.msg
CLEANFILES = $(BUILT_SOURCES)

OnlineRender.msg:
	cat $(OnlineRender_MSGS) > $@
