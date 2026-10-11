# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libOliasSearch.a
%canon_reldir%_libOliasSearch_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libOliasSearch_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libOliasSearch_a_SOURCES = $(OliasSearch_SRCS)
%canon_reldir%_libOliasSearch_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (OliasSearch_CLASSES).
OliasSearch_DH = %reldir%/OliasSearch.d %reldir%/OliasSearch.h
BUILT_SOURCES += $(OliasSearch_DH)
%reldir%/OliasSearch.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d OliasSearch $(OliasSearch_CLASSES)
%reldir%/OliasSearch.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h OliasSearch $(OliasSearch_CLASSES)
