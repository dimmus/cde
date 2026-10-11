MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libapi.la

libapi_la_CXXFLAGS = $(CDE_CXXFLAGS) -I..

libapi_la_SOURCES = base.C info_base.C info_lib.C smart_ptr.C transaction.C \
		    utility.C

noinst_HEADERS = \
    base.h \
    info_base.h \
    smart_ptr.h \
    transaction.h \
    utility.h
