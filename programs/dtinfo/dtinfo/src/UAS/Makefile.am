MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = Base MMDB DtSR

noinst_LIBRARIES = libUAS.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libUAS_a_CXXFLAGS = $(DTINFO_DEFINES) $(DTINFO_INCLUDES)
libUAS_a_SOURCES =

libUAS_a_LIBADD = Base/libBase.a MMDB/libMMDB.a DtSR/libDtSR.a

RANLIB = echo
libUAS_a_AR =  $(LD) -r -o

noinst_DATA = UAS.msg

UAS.msg: $(UAS_MSGS)
	cat $(UAS_MSGS) > $@

CLEANFILES = UAS.msg
