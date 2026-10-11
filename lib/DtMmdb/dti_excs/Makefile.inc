noinst_LTLIBRARIES += %reldir%/libdti_excs.la

%canon_reldir%_libdti_excs_la_CXXFLAGS = $(CDE_CXXFLAGS) -DNATIVE_EXCEPTIONS

if SOLARIS
%canon_reldir%_libdti_excs_la_CXXFLAGS += -DHAS_TERMINATE
endif

%canon_reldir%_libdti_excs_la_SOURCES = %reldir%/Jump_Environment.C \
	%reldir%/Exceptions.C %reldir%/Exception.C %reldir%/Destructable.C \
	%reldir%/terminate.C

noinst_HEADERS += %reldir%/Exceptions.hh
