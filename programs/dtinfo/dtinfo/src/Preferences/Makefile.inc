# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libPreferences.a
%canon_reldir%_libPreferences_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libPreferences_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libPreferences_a_SOURCES = $(Preferences_SRCS)
%canon_reldir%_libPreferences_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Preferences_CLASSES).
Preferences_DH = %reldir%/Preferences.d %reldir%/Preferences.h
BUILT_SOURCES += $(Preferences_DH)
%reldir%/Preferences.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Preferences $(Preferences_CLASSES)
%reldir%/Preferences.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Preferences $(Preferences_CLASSES)
