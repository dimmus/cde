MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libdti_excs.la

libdti_excs_la_CXXFLAGS = -DNATIVE_EXCEPTIONS

if SOLARIS
libdti_excs_la_CXXFLAGS += -DHAS_TERMINATE
endif

libdti_excs_la_SOURCES = Jump_Environment.C      Exceptions.C    Exception.C \
			 Destructable.C          terminate.C
