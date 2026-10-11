noinst_LTLIBRARIES += %reldir%/libdynhash.la

%canon_reldir%_libdynhash_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libdynhash_la_SOURCES = %reldir%/data_t.C \
	%reldir%/imp_bucket.C %reldir%/imp_die.C

noinst_HEADERS += %reldir%/data_t.h %reldir%/imp_bucket.h \
	%reldir%/imp_die.h
