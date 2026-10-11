# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

noinst_LIBRARIES += %reldir%/libcgm.a
%canon_reldir%_libcgm_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libcgm_a_CFLAGS = $(CDE_CFLAGS) $(DTINFO_DEFINES) \
	$(DTINFO_LIB_INCLUDES) -I$(top_srcdir)/include/DtI
%canon_reldir%_libcgm_a_SOURCES = %reldir%/cgmcmds.c %reldir%/cgmcommon.c \
	%reldir%/cgmio.c %reldir%/cgmtext.c %reldir%/getcgm.c
%canon_reldir%_libcgm_a_AR = $(LD) -r -o

noinst_PROGRAMS += %reldir%/testcgm
%canon_reldir%_testcgm_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP) $(DTINFO_DEFINES) \
	$(DTINFO_LIB_INCLUDES) -I$(top_srcdir)/include/DtI
%canon_reldir%_testcgm_SOURCES = %reldir%/testcgm.c
%canon_reldir%_testcgm_LDADD = %reldir%/libcgm.a $(LIBHELP) $(X11LIB) $(MATHLIB)
