noinst_LTLIBRARIES += %reldir%/libdstr.la

%canon_reldir%_libdstr_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libdstr_la_SOURCES = %reldir%/set.C %reldir%/bset.C \
	%reldir%/slist.C %reldir%/dlist_cell.C %reldir%/dlist.C %reldir%/heap.C \
	%reldir%/void_ptr_array.C %reldir%/void_ptr_stack.C \
	%reldir%/memory_pool.C %reldir%/dstr_test.C %reldir%/index_agent.C \
	%reldir%/token_stack.C %reldir%/slist_char_ptr_cell.C

noinst_HEADERS += %reldir%/bset.h %reldir%/dlist.h %reldir%/dlist_cell.h \
	%reldir%/dstr_test.h %reldir%/heap.h %reldir%/index_agent.h \
	%reldir%/memory_pool.h %reldir%/set.h %reldir%/slist.h \
	%reldir%/slist_char_ptr_cell.h %reldir%/token_stack.h \
	%reldir%/void_ptr_array.h %reldir%/void_ptr_stack.h
