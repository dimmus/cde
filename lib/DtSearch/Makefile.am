MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = raima

lib_LTLIBRARIES = libDtSearch.la

BUILT_SOURCES = boolyac.h boolyac.c
CLEANFILES = boolyac.h boolyac.c
AM_YFLAGS = -d

libDtSearch_la_CFLAGS = -DI18N_MSG -DMULTIBYTE -I./raima \
			$(DT_INCDIR)


# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtSearch_la_LDFLAGS = -version-info 2:1:0
else
libDtSearch_la_LDFLAGS = -version-info 3:0:1
endif

libDtSearch_la_LIBADD = raima/libraima.la

libDtSearch_la_SOURCES = apndext.c	ausdopen.c	ausexit.c \
			 bmstrstr.c	boolpars.c	boolsrch.c \
			 boolyac.y	cuslang.c	dbchange.c \
			 dberr.c	delspace.c	dtoe.c \
			 dtoeinit.c	dtsrapi.c	dtsrdbrec.c \
			 dtsrjoint.c	dtsrswab.c	dtsrutil.c  \
			 dtsrve.c	endslash.c	fileman.c \
			 globals.c	hdecode.c	hencode.c \
			 hilite.c	iscompat.c	isduprec.c \
			 jpn.c		lang.c		langmap.c \
			 msgs.c		msgutil.c	objdate.c \
			 ocf.c		opendblk.c	ophuf.c \
			 readchar.c	strupr.c	userint.c \
			 vedelete.c	vestatis.c	vstfunct.c
