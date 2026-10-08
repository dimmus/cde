noinst_PROGRAMS = merge mkcatdefs

BUILT_SOURCES = MsgCat.c
CLEANFILES = MsgCat.c

MsgCat.c: ${top_srcdir}/lib/DtSvc/DtUtil2/MsgCat.c
	$(RM) MsgCat.c && ln -s ${top_srcdir}/lib/DtSvc/DtUtil2/MsgCat.c MsgCat.c

AM_CFLAGS = -DNO_XLIB

merge_SOURCES = merge.c MsgCat.c

mkcatdefs_SOURCES = mkcatdefs.c MsgCat.c
