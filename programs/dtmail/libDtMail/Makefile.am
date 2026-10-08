MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = Common RFC

noinst_LIBRARIES = libDtMail.a

libDtMail_a_SOURCES =

# Unfortunately, this does not work -
# libDtMail_a_LIBADD = Common/libCommon.a RFC/libRFC.a
libDtMail_a_LIBADD = Common/*.o RFC/*.o
