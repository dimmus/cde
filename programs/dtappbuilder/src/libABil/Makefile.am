MAINTAINERCLEANFILES = Makefile.in

ABINCLUDES = ../include

EXTRA_DEFINES =  -D_POSIX_SOURCE=1

BUILT_SOURCES = bil_parse.h bil_parse.c
CLEANFILES = bil_parse.h bil_parse.c
AM_YFLAGS = -d

noinst_LIBRARIES = libABil.a

#noinst_PROGRAMS = bil_test

libABil_a_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES)

libABil_a_LIBADD =

libABil_a_SOURCES = bil_parse.y abuil_load.c	  	  abuil_print.c \
		  abuil_resource_attr_map.c       abuil_trav.c \
		  abuil_widget_obj_map.c          bil.c \
		  bil_lexer.c                     bil_loadact.c \
		  bil_loadatt.c                   bil_loadfile.c \
		  bil_loadobj.c 		  bil_parse.y \
		  bil_store.c                     gil.c \
		  gil_loadact.c                   gil_loadatt.c \
		  gil_loadfile.c                  gil_loadobj.c \
		  load.c

#bil_test_LDADD = ./libABil.a \
#        ../libABobjXm/libABobjXm.a \
#        ../libABobj/libABobj.a \
#        ../libAButil/libAButil.a

#bil_test_SOURCES = bil_test.c
