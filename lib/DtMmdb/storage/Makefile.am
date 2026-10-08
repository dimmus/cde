MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libstorage.la

libstorage_la_CXXFLAGS = -DPORTABLE_DB -I..

libstorage_la_SOURCES = abs_storage.C   chunks_index.C  heap_comp_funcs.C \
			lru.C           page.C          page_cache.C \
			page_rep.C      page_storage.C  rep_cell.C \
			rep_policy.C    store_test.C    unixf_storage.C \
			version.C       vm_storage.C
