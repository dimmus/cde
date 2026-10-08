MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libindex.la

libindex_la_CXXFLAGS = -DPORTABLE_DB -I..

libindex_la_SOURCES = btree_index.C dyn_disk_index.C dyn_index.C  \
		      dyn_memory_index.C fast_mphf.C hash.C index.C \
		      inv_lists.C mphf_index.C
