# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libOther.a
%canon_reldir%_libOther_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libOther_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libOther_a_SOURCES = $(Other_SRCS)
%canon_reldir%_libOther_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Other_CLASSES).
Other_DH = %reldir%/Other.d %reldir%/Other.h
BUILT_SOURCES += $(Other_DH)
%reldir%/Other.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Other $(Other_CLASSES)
%reldir%/Other.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Other $(Other_CLASSES)

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/Other.msg: $(Other_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/Other.msg
