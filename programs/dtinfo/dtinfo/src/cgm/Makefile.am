MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libcgm.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
libcgm_a_CFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)	\
	-I$(top_srcdir)/include/DtI

libcgm_a_SOURCES = cgmcmds.c cgmcommon.c cgmio.c cgmtext.c getcgm.c
RANLIB = echo
libcgm_a_AR = $(LD) -r -o

noinst_PROGRAMS = testcgm
testcgm_CPPFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)	\
	-I$(top_srcdir)/include/DtI
testcgm_SOURCES = testcgm.c
testcgm_LDADD = libcgm.a $(DTCLIENTLIBS) $(XTOOLLIB)


