MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = rpc.cmsd

rpc_cmsd_CFLAGS = -I../../../lib/csa $(DT_INCDIR) $(CSA_INCDIR)

rpc_cmsd_LDADD = $(LIBCSA) $(DTCLIENTLIBS) $(TIRPCLIB) $(XTOOLLIB)

if SOLARIS
rpc_cmsd_LDADD += -lsocket -lnsl -lintl
endif

rpc_cmsd_SOURCES = parser.c access.c callback.c cmscalendar.c		\
		   cmsconvert.c cmsentry.c cmsmatch.c delete.c		\
		   garbage.c insert.c lexit.c list.c log.c lookup.c	\
		   reclotick.c recount.c relasttick.c reminder.c	\
		   renexttick.c repeat.c reprevtick.c rerule.c		\
		   reutil.c tree.c utility.c v4ops.c v5ops.c		\
		   cmsfunc.c programtable.c rtable2.c rtable3.c		\
		   rtable4.c svcmain.c update.c access.h cmsentry.h	\
		   insert.h parser.h tree.h callback.h cmsmatch.h	\
		   lexer.h reminder.h update.h cmsattr.h data.h		\
		   list.h repeat.h utility.h cmscalendar.h delete.h	\
		   log.h reutil.h v4ops.h cmsconvert.h garbage.h	\
		   lookup.h rpcextras.h v5ops.h

BUILT_SOURCES = parser.c parser.h
CLEANFILES = parser.c parser.h y.tab.c  y.tab.h

AM_YFLAGS = -d

parser.c parser.h: parser.y
	$(YACC) $(AM_YFLAGS) $<
	$(SED) -e "s/yy/yyy/g" -e "/# line/d" y.tab.c > parser.c
	$(SED) -e 's/yy/yyy/g' y.tab.h >  parser.h

