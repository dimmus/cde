MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libmgrs.la

libmgrs_la_CXXFLAGS = $(CDE_CXXFLAGS) -DCOMPACTED_DISK_REP -DPORTABLE_DB -I..

libmgrs_la_SOURCES = managers.C misc.C query_mgr.C template_mgr.C

noinst_HEADERS = \
    managers.h \
    misc.h \
    query_mgr.h \
    template_mgr.h
