MAINTAINERCLEANFILES = Makefile.in

ABINCLUDES = ../include

EXTRA_DEFINES =  -D_POSIX_SOURCE=1

if BSD
UTILLIB = -lutil
endif

SOURCES.msg = \
        dtcodegen.msg   \
        ../libAButil/libAButil.msg \
        ../libABobjXm/libABobjXm.msg \
        ../libABil/libABil.msg

bin_PROGRAMS = dtcodegen

#noinst_PROGRAMS = ab_func_strings test_parse test_merge

noinst_DATA = dtcodegen.cat

dtcodegen_CFLAGS = $(EXTRA_DEFINES) -I$(ABINCLUDES) -I../libABil

dtcodegen_LDADD = ../libABil/libABil.a  \
		  ../libABobjXm/libABobjXm.a  \
                  ../libABobj/libABobj.a ../libAButil/libAButil.a \
		  $(LIBTERM) $(LIBSVC) $(LIBTT) $(LIBHELP) $(LIBXIN) \
		  $(LIBWIDGET) $(UTILLIB) $(XTOOLLIB)


dtcodegen_SOURCES = abmf.c                  args.c \
		    cdefs.c                 connect.c \
		    create_decls.c          dtb_utils.c \
		    generate_code.c         instances.c \
		    lib_func_strings.c      make_file.c \
		    merge_c.c               motifdefs.c \
		    msg_cvt.c               msg_file.c \
		    obj_names.c             parse_c.c \
		    proj_c_file.c           proj_header_file.c \
		    resource_file.c         stubs_c_file.c \
		    ui_c_file.c             ui_header_file.c \
		    utils.c                 utils_c_file.c \
		    utils_header_file.c     write_c.c \
		    write_code.c

#ab_func_strings_LDADD = ../libAButil/libAButil.a

#ab_func_strings_SOURCES = ab_func_strings.c

#test_parse_CFLAGS = -DMAIN -D_POSIX_SOURCE=1 -I$(ABINCLUDES)

#test_parse_LDADD = ../libAButil/libAButil.a

#test_parse_SOURCES = parse_c.c

#test_merge_CFLAGS = -DMAIN -D_POSIX_SOURCE=1 -I$(ABINCLUDES)

#test_merge_LDADD = ../libAButil/libAButil.a parse_c.o

#test_merge_SOURCES = merge_c.c

dtcodegen.cat:: $(SOURCES.msg)
	$(GENCAT) dtcodegen.cat $(SOURCES.msg)
