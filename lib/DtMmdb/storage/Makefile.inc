noinst_LTLIBRARIES += %reldir%/libstorage.la

%canon_reldir%_libstorage_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB

%canon_reldir%_libstorage_la_SOURCES = %reldir%/abs_storage.C \
	%reldir%/chunks_index.C %reldir%/heap_comp_funcs.C %reldir%/lru.C \
	%reldir%/page.C %reldir%/page_cache.C %reldir%/page_rep.C \
	%reldir%/page_storage.C %reldir%/rep_cell.C %reldir%/rep_policy.C \
	%reldir%/store_test.C %reldir%/unixf_storage.C %reldir%/version.C \
	%reldir%/vm_storage.C

noinst_HEADERS += %reldir%/abs_storage.h %reldir%/chunks_index.h \
	%reldir%/heap_comp_funcs.h %reldir%/lru.h %reldir%/page.h \
	%reldir%/page_rep.h %reldir%/rep_cell.h %reldir%/rep_policy.h \
	%reldir%/unixf_storage.h %reldir%/version.h %reldir%/vm_storage.h
