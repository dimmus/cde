# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libBasic.a
%canon_reldir%_libBasic_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libBasic_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libBasic_a_SOURCES = $(Basic_SRCS)
%canon_reldir%_libBasic_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Basic_CLASSES).
Basic_DH = %reldir%/Basic.d %reldir%/Basic.h
BUILT_SOURCES += $(Basic_DH)
%reldir%/Basic.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Basic $(Basic_CLASSES)
%reldir%/Basic.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Basic $(Basic_CLASSES)
