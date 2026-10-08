MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libWidgets.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

libWidgets_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

libWidgets_a_SOURCES = Pixmap.c Tree.c Panner.c Simple.c Porthole.c	\
	XawInitFake.c TabButton.c Restraint.c WXmOpButton.C

RANLIB = echo
libWidgets_a_AR = $(LD) -r -o

