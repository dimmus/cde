noinst_LTLIBRARIES += %reldir%/libdiskhash.la

%canon_reldir%_libdiskhash_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libdiskhash_la_SOURCES = %reldir%/disk_bucket.C \
	%reldir%/bucket_array.C %reldir%/disk_hash.C

noinst_HEADERS += %reldir%/bucket_array.h %reldir%/disk_bucket.h \
	%reldir%/disk_hash.h
