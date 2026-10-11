# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libQuery.a
%canon_reldir%_libQuery_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libQuery_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libQuery_a_SOURCES = $(Query_SRCS)
%canon_reldir%_libQuery_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Query_CLASSES).
Query_DH = %reldir%/Query.d %reldir%/Query.h
BUILT_SOURCES += $(Query_DH)
%reldir%/Query.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Query $(Query_CLASSES)
%reldir%/Query.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Query $(Query_CLASSES)

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/Query.msg: $(Query_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/Query.msg
