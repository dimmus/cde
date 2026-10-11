# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libManagers.a
%canon_reldir%_libManagers_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libManagers_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libManagers_a_SOURCES = $(Managers_SRCS)
%canon_reldir%_libManagers_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Managers_CLASSES).
Managers_DH = %reldir%/Managers.d %reldir%/Managers.h
BUILT_SOURCES += $(Managers_DH)
%reldir%/Managers.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Managers $(Managers_CLASSES)
%reldir%/Managers.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Managers $(Managers_CLASSES)

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/Managers.msg: $(Managers_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/Managers.msg
