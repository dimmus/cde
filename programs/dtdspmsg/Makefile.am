MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtdspmsg

dtdspmsg_SOURCES = dtdspmsg.c MsgCat.c

BUILT_SOURCES = MsgCat.c
CLEANFILES = MsgCat.c

MsgCat.c: ${top_srcdir}/lib/DtSvc/DtUtil2/MsgCat.c
	$(RM) MsgCat.c && ln -s ${top_srcdir}/lib/DtSvc/DtUtil2/MsgCat.c MsgCat.c

# this is just to initialize so the += ops can work later if needed
dtdspmsg_LDADD =
dtdspmsg_CFLAGS = -DNO_XLIB

if SOLARIS
dtdspmsg_LDADD += -lgen
endif

