MAINTAINERCLEANFILES = Makefile.in

include ../dtdocbook.am

instantdir = $(dtdocbooklibexecdir)/instant

instant_PROGRAMS = instant

instant_CFLAGS = $(DT_INCDIR)

instant_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

instant_CFLAGS += @TCL_INCLUDE_SPEC@
instant_LDADD += @TCL_LIB_SPEC@

instant_SOURCES = main.c util.c info.c translate.c traninit.c tranvar.c \
		  tables.c browse.c
