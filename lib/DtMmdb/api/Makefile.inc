noinst_LTLIBRARIES += %reldir%/libapi.la

%canon_reldir%_libapi_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libapi_la_SOURCES = %reldir%/base.C %reldir%/info_base.C \
	%reldir%/info_lib.C %reldir%/smart_ptr.C %reldir%/transaction.C \
	%reldir%/utility.C

noinst_HEADERS += %reldir%/base.h %reldir%/info_base.h \
	%reldir%/smart_ptr.h %reldir%/transaction.h %reldir%/utility.h
