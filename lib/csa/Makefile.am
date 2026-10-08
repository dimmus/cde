csadir = $(includedir)/csa

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libcsa.la

libcsa_la_CFLAGS = -DRFC_MIME -DLINE_COUNT -DV2 -DOW_I18N \
		   $(CSA_INCDIR)

libcsa_la_LIBADD = $(XTOOLLIB) $(TIRPCLIB)

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libcsa_la_LDFLAGS = -version-info 2:1:0
else
libcsa_la_LDFLAGS = -version-info 3:0:1
endif

if LINUX
else
libcsa_la_LIBADD += $(LIBSVC) $(XTOOLLIB)
endif

AM_YFLAGS = -d

GENERATED_SRCS = agent_xdr.c cm_clnt.c reparser.c reparser.h \
        rtable2_clnt.c rtable2_xdr.c rtable3_clnt.c rtable3_xdr.c \
        rtable4_clnt.c rtable4_xdr.c agent.h

BUILT_SOURCES = $(GENERATED_SRCS)
CLEANFILES = $(GENERATED_SRCS) y.tab.c y.tab.h

libcsa_la_SOURCES = agent.c         agent_xdr.c     api.c \
		    appt4.c         attr.c          calendar.c \
		    cm_clnt.c       cmcbxdr.c       cmsdata.c \
		    cmxdr.c         connection.c    convert2-4.c \
		    convert3-4.c    convert4-2.c    convert4-3.c \
		    convert4-5.c    convert5-4.c    debug.c \
		    entry.c         free.c          hash.c \
		    iso8601.c       laccess.c       lutil.c \
		    match.c         nametbl.c       refree.c \
		    reparser.c	    rescan.c        rpccalls.c \
		    rtable2_clnt.c  rtable2_xdr.c   rtable3_clnt.c \
		    rtable3_xdr.c   rtable4_clnt.c  rtable4_xdr.c \
		    table.c         updateattrs.c   xtclient.c \
		    ansi_c.c

reparser.c reparser.h: reparser.y
	$(YACC) $(AM_YFLAGS) $<
	sed -e s/yyparse/_DtCm_rule_parser/g -e s/yy/_DtCm_yy/g y.tab.c > reparser.c
	sed s/yy/_DtCm_yy/g y.tab.h > reparser.h

rtable2_clnt.c rtable3_clnt.c rtable4_clnt.c cm_clnt.c:
	$(RM) $@
	cat EUSinclude > $@
	rpcgen -l $(@:_clnt.c=.x) | sed -f namechange1.sed | sed -f namechange2.sed >> $@

rtable2_xdr.c rtable3_xdr.c rtable4_xdr.c agent_xdr.c:
	$(RM) $@
	cat EUSinclude > $(@:.x=_xdr.c)
	rpcgen -c $(@:_xdr.c=.x) | sed -f namechange1.sed | sed -f namechange2.sed >> $(@:.x=_xdr.c)

rtable4_clnt.c rtable4_xdr.c: rtable4.h namechange1.sed namechange2.sed
rtable3_clnt.c rtable3_xdr.c: rtable3.h namechange1.sed namechange2.sed
rtable2_clnt.c rtable2_xdr.c: rtable3.h namechange1.sed namechange2.sed
agent_xdr.c agent.c: agent.h

agent.h:
	$(RM) $@
	cat agent.wrapbegin > agent.h
	rpcgen -h agent.x | $(SED) -f namechange1.sed | $(SED) -f namechange2.sed >> agent.h
	cat agent.wrapend >> agent.h

