noinst_LTLIBRARIES += %reldir%/libmgrs.la

%canon_reldir%_libmgrs_la_CXXFLAGS = $(CDE_CXXFLAGS) -DCOMPACTED_DISK_REP -DPORTABLE_DB

%canon_reldir%_libmgrs_la_SOURCES = %reldir%/managers.C %reldir%/misc.C \
	%reldir%/query_mgr.C %reldir%/template_mgr.C

noinst_HEADERS += %reldir%/managers.h %reldir%/misc.h \
	%reldir%/query_mgr.h %reldir%/template_mgr.h
