MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

noinst_LIBRARIES = libDtSR.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libDtSR_a_CXXFLAGS = $(CDE_CXXFLAGS) -I../Base -I./Util_Classes -I../..	\
	$(DTINFO_DEFINES) $(DTINFO_INCLUDES)
libDtSR_a_SOURCES = $(DtSR_SRCS)

RANLIB = echo
libDtSR_a_AR = $(LD) -r -o

BUILT_SOURCES = DtSR.msg
CLEANFILES = $(BUILT_SOURCES)

DtSR.msg:
	cat $(DtSR_MSGS) > $@
