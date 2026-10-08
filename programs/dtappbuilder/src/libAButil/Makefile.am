MAINTAINERCLEANFILES = Makefile.in

ABINCLUDES = ../include

EXTRA_DEFINES =  -D_POSIX_SOURCE=1

noinst_LIBRARIES = libAButil.a

#noinst_PROGRAMS = util_test istr_test

libAButil_a_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES)

libAButil_a_SOURCES = abio.c          \
		      abio_path.c     \
		      istr.c          \
		      strlist.c       \
		      util.c          \
		      util_convert.c  \
		      util_ds.c       \
		      util_err.c      \
		      util_file.c     \
		      util_string.c

#util_test_LDADD = libAButil.a

#util_test_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES)

#util_test_SOURCES = util_test.c

#istr_test_LDADD = libAButil.a

#istr_test_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES)

#istr_test_SOURCES = istr_test.c
