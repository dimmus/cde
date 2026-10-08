MAINTAINERCLEANFILES = Makefile.in

ABINCLUDES = ../include

EXTRA_DEFINES =  -D_POSIX_SOURCE=1

if SOLARIS
EXTRALIBS = -lgen
endif

noinst_LIBRARIES = libABobj.a

#noinst_PROGRAMS = test test_notify

libABobj_a_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES)

libABobj_a_SOURCES = obj.c \
		     obj_action.c \
		     obj_alloc.c \
		     obj_fields.c \
		     obj_geometry.c \
		     obj_init.c \
		     obj_isa.c \
		     obj_names_list.c \
		     obj_list.c \
		     obj_lists.c \
		     obj_notify.c \
		     obj_release.c \
		     obj_scoped.c \
		     obj_select.c \
		     obj_tree.c \
		     obj_utils.c \
		     trav.c \
		     travP.c \
		     trav_public.c \
		     trav_safe.c

#test_LDADD = ../libAButil/libAButil.a libABobj.a $(EXTRALIBS)

#test_SOURCES = test.c

#test_notify_LDADD = ../libAButil/libAButil.a libABobj.a $(EXTRALIBS)

#test_notify_SOURCES = test_notify.c
