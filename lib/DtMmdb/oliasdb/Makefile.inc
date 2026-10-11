noinst_LTLIBRARIES += %reldir%/liboliasdb.la

%canon_reldir%_liboliasdb_la_CXXFLAGS = $(CDE_CXXFLAGS) -DNO_DB_LOCK

%canon_reldir%_liboliasdb_la_SOURCES = %reldir%/asciiIn_filters.C \
	%reldir%/collectionIterator.C %reldir%/dlp_hd.C %reldir%/doc_hd.C \
	%reldir%/graphic_hd.C %reldir%/locator_hd.C %reldir%/mark.C \
	%reldir%/mark_base.C %reldir%/mmdb.C %reldir%/node_hd.C \
	%reldir%/olias_funcs.C %reldir%/stylesheet_hd.C %reldir%/toc_hd.C \
	%reldir%/user_base.C

noinst_HEADERS += %reldir%/asciiIn_filters.h \
	%reldir%/collectionIterator.h %reldir%/dlp_hd.h %reldir%/doc_hd.h \
	%reldir%/graphic_hd.h %reldir%/locator_hd.h %reldir%/mark.h \
	%reldir%/mark_base.h %reldir%/mmdb.h %reldir%/node_hd.h \
	%reldir%/olias_funcs.h %reldir%/stylesheet_hd.h %reldir%/toc_hd.h \
	%reldir%/user_base.h
