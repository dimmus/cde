MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libTermPrim.la

AM_CPPFLAGS = $(DT_INCDIR)

libTermPrim_la_SOURCES = TermPrim.c \
        TermPrimAction.c \
        TermPrimBuffer.c \
        TermPrimBufferWc.c \
        TermPrimCursor.c \
        TermPrimDebug.c \
        TermPrimFunction.c \
        TermPrimLineDraw.c \
        TermPrimParse.c \
        TermPrimParseTable.c \
        TermPrimParser.c \
        TermPrimPendingText.c \
        TermPrimRender.c \
        TermPrimRenderMb.c \
        TermPrimRenderFont.c \
        TermPrimRenderFontSet.c \
        TermPrimRenderLineDraw.c \
        TermPrimRepType.c \
	TermPrimScroll.c \
        TermPrimSelect.c \
        TermPrimSetPty.c \
        TermPrimSetUtmp.c \
        TermPrimSubproc.c \
        TermPrimUtil.c    \
        TermPrimVersion.c \
        TermPrimWMProtocols.c

# JET there may be some more missing here...

if SOLARIS
libTermPrim_la_SOURCES += TermPrimGetPty-svr4.c
else
libTermPrim_la_SOURCES += TermPrimGetPty-pts.c
endif
