MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libMMDB.a

# get our env
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# class info
include Classlist.mk

libMMDB_a_CXXFLAGS = -I../Base -I../.. $(DTINFO_DEFINES)	\
	$(DTINFO_INCLUDES)
libMMDB_a_SOURCES = $(MMDB_SRCS)

RANLIB = echo
libMMDB_a_AR = $(LD) -r -o


BUILT_SOURCES = MMDB.msg
CLEANFILES = $(BUILT_SOURCES)

MMDB.msg:
	cat $(MMDB_MSGS) > $@
