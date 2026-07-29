MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libdstr.la

libdstr_la_CXXFLAGS = $(CDE_CXXFLAGS) -I..

libdstr_la_SOURCES = set.C bset.C slist.C dlist_cell.C dlist.C heap.C \
		     void_ptr_array.C void_ptr_stack.C memory_pool.C \
		     dstr_test.C index_agent.C token_stack.C \
		     slist_char_ptr_cell.C

noinst_HEADERS = \
    bset.h \
    dlist.h \
    dlist_cell.h \
    dstr_test.h \
    heap.h \
    index_agent.h \
    memory_pool.h \
    set.h \
    slist.h \
    slist_char_ptr_cell.h \
    token_stack.h \
    void_ptr_array.h \
    void_ptr_stack.h
