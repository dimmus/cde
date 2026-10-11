# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libAgents.a
%canon_reldir%_libAgents_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libAgents_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libAgents_a_SOURCES = $(Agents_SRCS)
%canon_reldir%_libAgents_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (Agents_CLASSES).
Agents_DH = %reldir%/Agents.d %reldir%/Agents.h
BUILT_SOURCES += $(Agents_DH)
%reldir%/Agents.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d Agents $(Agents_CLASSES)
%reldir%/Agents.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h Agents $(Agents_CLASSES)

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/Agents.msg: $(Agents_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/Agents.msg
