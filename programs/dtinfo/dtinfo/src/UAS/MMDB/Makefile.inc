# Included by ../Makefile.am: paths are relative to dtinfo/src
# (%reldir% is this directory).

include %reldir%/Classlist.mk

noinst_LIBRARIES += %reldir%/libMMDB.a
%canon_reldir%_libMMDB_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(DTINFO_DIR_CPPFLAGS) $(DTINFO_FILE_MAP)
%canon_reldir%_libMMDB_a_CXXFLAGS = $(CDE_CXXFLAGS) -I$(srcdir)/%reldir%/../Base -I. -I$(srcdir) \
	$(DTINFO_DEFINES) -I$(srcdir)/%reldir%/.. $(DTINFO_OTHER_INCLUDES)
%canon_reldir%_libMMDB_a_SOURCES = $(MMDB_SRCS)
%canon_reldir%_libMMDB_a_AR = $(LD) -r -o

# The messages of this directory, for dtinfo.msg.updated.
%reldir%/MMDB.msg: $(MMDB_MSGS)
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cat $^ > $@.tmp && mv -f $@.tmp $@
CLEANFILES += %reldir%/MMDB.msg
