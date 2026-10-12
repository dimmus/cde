MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

CLEANFILES = TermLineData.c

noinst_LTLIBRARIES = libTerm.la

libTerm_la_CFLAGS = $(CDE_CFLAGS) -I$(srcdir)/../TermPrim @DT_INCDIR@

$(top_builddir)/lib/DtTerm/util/lineToData:
	$(MAKE) -C $(top_builddir)/lib/DtTerm/util lineToData

TermLineData.c: TermLineData.data $(top_builddir)/lib/DtTerm/util/lineToData
	$(RM) $@
	$(top_builddir)/lib/DtTerm/util/lineToData -p _DtTerm -f $< > $@

includes:: TermLineData.c

libTerm_la_SOURCES = \
        Term.c \
        TermAction.c \
        TermBuffer.c \
        TermColor.c \
        TermEnhance.c \
        TermFunction.c \
        TermFunctionKey.c \
        TermParse.c \
        TermParseTable.c \
        TermVersion.c

# made by lineToData (above)
nodist_libTerm_la_SOURCES = TermLineData.c

