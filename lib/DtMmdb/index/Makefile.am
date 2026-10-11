MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libindex.la

libindex_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB -I..

libindex_la_SOURCES = btree_index.C dyn_disk_index.C dyn_index.C  \
		      dyn_memory_index.C fast_mphf.C hash.C index.C \
		      inv_lists.C mphf_index.C

noinst_HEADERS = \
    btree_index.h \
    dyn_disk_index.h \
    dyn_index.h \
    dyn_memory_index.h \
    fast_mphf.h \
    hash.h \
    index.h \
    inv_lists.h \
    mphf_index.h
