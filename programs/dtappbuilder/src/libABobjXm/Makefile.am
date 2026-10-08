MAINTAINERCLEANFILES = Makefile.in

ABINCLUDES = ../include

EXTRA_DEFINES =  -D_POSIX_SOURCE=1

noinst_LIBRARIES = libABobjXm.a

libABobjXm_a_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES)

libABobjXm_a_LIBADD =

libABobjXm_a_SOURCES = objxm_args.c    objxm_config.c  objxm_conn.c \
		       objxm_inst.c    objxm_objects.c objxm_strs.c \
		       objxm_util.c
