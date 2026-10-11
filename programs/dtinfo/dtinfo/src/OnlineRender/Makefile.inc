# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libOnlineRender.a
%canon_reldir%_libOnlineRender_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libOnlineRender_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libOnlineRender_a_SOURCES = %reldir%/CanvasRenderer.C %reldir%/FeatureSupport.C \
	%reldir%/FontCache.C %reldir%/SegClientData.C
%canon_reldir%_libOnlineRender_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (OnlineRender_CLASSES).
OnlineRender_DH = %reldir%/OnlineRender.d %reldir%/OnlineRender.h
BUILT_SOURCES += $(OnlineRender_DH)
%reldir%/OnlineRender.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d OnlineRender $(OnlineRender_CLASSES)
%reldir%/OnlineRender.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h OnlineRender $(OnlineRender_CLASSES)

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/OnlineRender.msg: $(OnlineRender_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/OnlineRender.msg
