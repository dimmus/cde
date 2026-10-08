MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtsrcreate dtsrdbrec dtsrhan dtsrload dtsrindex dtsrdelete \
	       huffcode dtsrkdump dtsrclean

AM_CFLAGS = -I../../lib/DtSearch -I../../lib/DtSearch/raima	\
	$(DT_INCDIR) -DMAIN_PROGRAM -DNO_XLIB

BUILT_SOURCES = MsgCat.c
CLEANFILES = MsgCat.c

MsgCat.c: ${top_srcdir}/lib/DtSvc/DtUtil2/MsgCat.c
	$(RM) MsgCat.c && ln -s ${top_srcdir}/lib/DtSvc/DtUtil2/MsgCat.c MsgCat.c


LDADD = ../../lib/DtSearch/libDtSearch.la

dtsrcreate_SOURCES = dtsrcreate.c MsgCat.c

dtsrdbrec_SOURCES = dtsrdbrec.c MsgCat.c

dtsrhan_SOURCES = dtsrhan.c MsgCat.c

dtsrload_SOURCES = dtsrload.c MsgCat.c

dtsrclean_SOURCES = dtsrclean.c MsgCat.c

dtsrindex_SOURCES = dtsrindex.c MsgCat.c

dtsrdelete_SOURCES = tomita.c MsgCat.c

huffcode_SOURCES = huffcode.c MsgCat.c

dtsrkdump_SOURCES = dtsrkdump.c MsgCat.c
