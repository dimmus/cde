noinst_LTLIBRARIES += %reldir%/libindex.la

%canon_reldir%_libindex_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB

%canon_reldir%_libindex_la_SOURCES = %reldir%/btree_index.C \
	%reldir%/dyn_disk_index.C %reldir%/dyn_index.C \
	%reldir%/dyn_memory_index.C %reldir%/fast_mphf.C %reldir%/hash.C \
	%reldir%/index.C %reldir%/inv_lists.C %reldir%/mphf_index.C

noinst_HEADERS += %reldir%/btree_index.h %reldir%/dyn_disk_index.h \
	%reldir%/dyn_index.h %reldir%/dyn_memory_index.h %reldir%/fast_mphf.h \
	%reldir%/hash.h %reldir%/index.h %reldir%/inv_lists.h \
	%reldir%/mphf_index.h
