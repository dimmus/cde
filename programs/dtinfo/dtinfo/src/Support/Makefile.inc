# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libSupport.a
%canon_reldir%_libSupport_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libSupport_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libSupport_a_SOURCES = $(Support_SRCS)
%canon_reldir%_libSupport_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Support_CLASSES).
Support_DH = %reldir%/Support.d %reldir%/Support.h
BUILT_SOURCES += $(Support_DH)
%reldir%/Support.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Support $(Support_CLASSES)
%reldir%/Support.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Support $(Support_CLASSES)
