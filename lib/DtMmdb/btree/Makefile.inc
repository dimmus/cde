noinst_LTLIBRARIES += %reldir%/libbtree.la

%canon_reldir%_libbtree_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libbtree_la_SOURCES = %reldir%/btree.C

noinst_HEADERS += %reldir%/btree.h
