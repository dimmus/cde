MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libstorage.la

libstorage_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB -I..

libstorage_la_SOURCES = abs_storage.C   chunks_index.C  heap_comp_funcs.C \
			lru.C           page.C          page_cache.C \
			page_rep.C      page_storage.C  rep_cell.C \
			rep_policy.C    store_test.C    unixf_storage.C \
			version.C       vm_storage.C

noinst_HEADERS = \
    abs_storage.h \
    chunks_index.h \
    heap_comp_funcs.h \
    lru.h \
    page.h \
    page_rep.h \
    rep_cell.h \
    rep_policy.h \
    unixf_storage.h \
    version.h \
    vm_storage.h
