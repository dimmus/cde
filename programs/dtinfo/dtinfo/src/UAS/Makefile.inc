# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

include %reldir%/Base/Makefile.inc
include %reldir%/MMDB/Makefile.inc
include %reldir%/DtSR/Makefile.inc

noinst_LIBRARIES += %reldir%/libUAS.a
%canon_reldir%_libUAS_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS)
%canon_reldir%_libUAS_a_CXXFLAGS = $(DTINFO_LIB_CXXFLAGS)
%canon_reldir%_libUAS_a_SOURCES =
%canon_reldir%_libUAS_a_LIBADD = %reldir%/Base/libBase.a %reldir%/MMDB/libMMDB.a %reldir%/DtSR/libDtSR.a
%canon_reldir%_libUAS_a_AR = $(LD) -r -o

# The class headers that Prelude.h includes (UAS_CLASSES).
UAS_DH = %reldir%/UAS.d %reldir%/UAS.h
BUILT_SOURCES += $(UAS_DH)
%reldir%/UAS.d: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) d UAS $(UAS_CLASSES)
%reldir%/UAS.h: %reldir%/Classlist.mk $(DFILES)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && $(DFILES) h UAS $(UAS_CLASSES)

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/UAS.msg: $(UAS_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/UAS.msg
