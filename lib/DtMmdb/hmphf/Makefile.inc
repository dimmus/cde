noinst_LTLIBRARIES += %reldir%/libhmphf.la

%canon_reldir%_libhmphf_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB

%canon_reldir%_libhmphf_la_SOURCES = %reldir%/buckets.C \
	%reldir%/mphf_funcs.C %reldir%/mphf_hash_table.C %reldir%/params.C \
	%reldir%/pattern.C %reldir%/sorter.C

noinst_HEADERS += %reldir%/buckets.h %reldir%/mphf_funcs.h \
	%reldir%/mphf_hash_table.h %reldir%/params.h %reldir%/pattern.h \
	%reldir%/sorter.h
