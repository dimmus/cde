# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libGraphics.a
%canon_reldir%_libGraphics_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libGraphics_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libGraphics_a_SOURCES = $(Graphics_SRCS)
%canon_reldir%_libGraphics_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Graphics_CLASSES).
Graphics_DH = %reldir%/Graphics.d %reldir%/Graphics.h
BUILT_SOURCES += $(Graphics_DH)
%reldir%/Graphics.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Graphics $(Graphics_CLASSES)
%reldir%/Graphics.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Graphics $(Graphics_CLASSES)
