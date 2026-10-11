# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libMarks.a
%canon_reldir%_libMarks_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libMarks_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libMarks_a_SOURCES = $(Marks_SRCS)
%canon_reldir%_libMarks_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Marks_CLASSES).
Marks_DH = %reldir%/Marks.d %reldir%/Marks.h
BUILT_SOURCES += $(Marks_DH)
%reldir%/Marks.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Marks $(Marks_CLASSES)
%reldir%/Marks.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Marks $(Marks_CLASSES)
