MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libDtXinerama.la

libDtXinerama_la_SOURCES = DtXinerama.c

libDtXinerama_la_LIBADD = -lXinerama

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtXinerama_la_LDFLAGS = -version-info 2:1:0
else
libDtXinerama_la_LDFLAGS = -version-info 3:0:1
endif


