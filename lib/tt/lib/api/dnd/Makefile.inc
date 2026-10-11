noinst_LTLIBRARIES += %reldir%/libdnd.la

%canon_reldir%_libdnd_la_CFLAGS = $(CDE_CFLAGS)

%canon_reldir%_libdnd_la_SOURCES = %reldir%/ttdnd.c
