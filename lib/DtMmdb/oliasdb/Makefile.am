MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = liboliasdb.la

liboliasdb_la_CXXFLAGS = $(CDE_CXXFLAGS) -DNO_DB_LOCK -I..

liboliasdb_la_SOURCES = asciiIn_filters.C \
			collectionIterator.C \
			dlp_hd.C \
			doc_hd.C \
			graphic_hd.C \
			locator_hd.C \
			mark.C \
			mark_base.C \
			mmdb.C \
			node_hd.C \
			olias_funcs.C \
			stylesheet_hd.C \
			toc_hd.C \
			user_base.C

noinst_HEADERS = \
    asciiIn_filters.h \
    collectionIterator.h \
    dlp_hd.h \
    doc_hd.h \
    graphic_hd.h \
    locator_hd.h \
    mark.h \
    mark_base.h \
    mmdb.h \
    node_hd.h \
    olias_funcs.h \
    stylesheet_hd.h \
    toc_hd.h \
    user_base.h
