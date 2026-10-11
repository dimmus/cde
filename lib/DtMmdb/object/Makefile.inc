noinst_LTLIBRARIES += %reldir%/libobject.la

%canon_reldir%_libobject_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB

%canon_reldir%_libobject_la_SOURCES = %reldir%/composite.C \
	%reldir%/compressed_pstring.C %reldir%/cset.C %reldir%/dl_list.C \
	%reldir%/dl_list_cell.C %reldir%/handler.C %reldir%/integer.C \
	%reldir%/long_pstring.C %reldir%/oid.C %reldir%/oid_list.C \
	%reldir%/oid_t.C %reldir%/primitive.C %reldir%/pstring.C \
	%reldir%/random_gen.C %reldir%/root.C %reldir%/short_list.C \
	%reldir%/tuple.C

noinst_HEADERS += %reldir%/composite.h %reldir%/compressed_pstring.h \
	%reldir%/cset.h %reldir%/dl_list.h %reldir%/dl_list_cell.h \
	%reldir%/handler.h %reldir%/integer.h %reldir%/long_pstring.h \
	%reldir%/oid.h %reldir%/oid_list.h %reldir%/oid_t.h %reldir%/primitive.h \
	%reldir%/pstring.h %reldir%/random_gen.h %reldir%/root.h \
	%reldir%/short_list.h %reldir%/tuple.h
