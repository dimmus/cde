# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libDtSR.a
%canon_reldir%_libDtSR_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libDtSR_a_CXXFLAGS = $(CDE_CXXFLAGS) -I$(srcdir)/%reldir%/../Base \
	-I$(srcdir)/%reldir%/Util_Classes -I. -I$(srcdir) \
	$(DTINFO_DEFINES) -I$(srcdir)/%reldir%/.. $(DTINFO_OTHER_INCLUDES)
%canon_reldir%_libDtSR_a_SOURCES = $(DtSR_SRCS)
%canon_reldir%_libDtSR_a_AR = $(LD) -r -o

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/DtSR.msg: $(DtSR_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/DtSR.msg
