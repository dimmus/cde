MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

# get our env variables
include $(top_srcdir)/programs/dtinfo/dtinfo_env.mk

# get the toplevel Classlist (CLASSLIBS, MSGS)
include Classlist.mk

# dtinfo is linked from one partially linked object ("libX.a", made with
# ld -r) per directory.  They are all built from this one Makefile (each
# directory's Makefile.inc, included below) rather than by a make per
# directory, so that make sees all of dtinfo at once: the directories no
# longer wait for one another, and a no-op build reads one Makefile
# instead of eighteen.  Paths in the Makefile.inc files are relative to
# this directory (%reldir%).
#
# The objects of each directory are compiled with the include path that
# the make in that directory gave them: the directory itself first, then
# include/, then this directory.  nostdinc turns off automake's own
# -I. -I$(top_builddir)/include, which would put this directory (and its
# config.h) ahead of include/.
AUTOMAKE_OPTIONS = nostdinc

DTINFO_DIR_CPPFLAGS = -I$(top_builddir)/include $(CDE_CPPFLAGS)

# In the objects of the directories, __FILE__ (which the assertion
# messages print) names the source file without its directory, as when
# each directory had its own make: $(@D) is the directory of the object
# being compiled.  (-fmacro-prefix-map came with -ffile-prefix-map.)
DTINFO_FILE_MAP = $(if $(findstring -ffile-prefix-map,$(CDE_CXXFLAGS)),-fmacro-prefix-map=$(@D)/=)
DTINFO_OTHER_INCLUDES = $(UAS_INCLUDES) $(MMDB_INCLUDES) $(WWL_INCLUDES)
DTINFO_LIB_INCLUDES = -I. -I$(srcdir) $(DTINFO_OTHER_INCLUDES)
DTINFO_LIB_CXXFLAGS = $(CDE_CXXFLAGS) $(DTINFO_DEFINES) $(DTINFO_LIB_INCLUDES)

noinst_LIBRARIES =
noinst_PROGRAMS =
BUILT_SOURCES = Prelude.h Dtinfo
CLEANFILES = $(BUILT_SOURCES)

# The libraries are relocatable objects, not archives.
RANLIB = echo

PMAKER = $(top_builddir)/programs/dtinfo/tools/misc/pmaker
DFILES = $(abs_top_builddir)/programs/dtinfo/tools/misc/dfiles

include Widgets/Makefile.inc
include Support/Makefile.inc
include UAS/Makefile.inc
include Basic/Makefile.inc
include OliasSearch/Makefile.inc
include Marks/Makefile.inc
include Graphics/Makefile.inc
include Preferences/Makefile.inc
include Managers/Makefile.inc
include Other/Makefile.inc
include Query/Makefile.inc
include Agents/Makefile.inc
include OnlineRender/Makefile.inc
include Messages/Makefile.inc
include cgm/Makefile.inc

RESFILES = Macros.res Strings.res Messages.res Query/QueryEditor.res	\
	Agents/BookmarkEdit.res Agents/MapAgent.res			\
	Agents/MarkList.res Agents/PrintPanelAgent.res			\
	Agents/Preferences.res Agents/SearchResultsAgent.res		\
	Agents/NodeHistoryAgent.res Agents/NodeWindowAgent.res		\
	Agents/GraphicAgent.res Query/QueryTermView.res

# Prelude.h starts with #include "config.h", meaning the config.h next to
# it.  In a VPATH build Prelude.h is in the build directory, so name the
# one in $(srcdir) there.
Prelude.h: Classlist.mk $(PMAKER)
	$(PMAKER) $(CLASSLIBS)
	if test "$(srcdir)" != .; then \
	  sed -e '1s|^#include "config.h"$$|#include "$(srcdir)/config.h"|' \
	    $@ > $@.tmp && mv -f $@.tmp $@; \
	fi

Dtinfo: Dtinfo.res $(RESFILES)
	$(CPP_PROGRAM) -traditional -nostdinc -C $(srcdir)/Dtinfo.res | $(TREERES) > $@

# gather up the messages and generate the file. MSGS comes from the
# dtinfo/src/Classlist.mk file.

noinst_DATA = dtinfo.msg.updated

dtinfo.msg.updated: $(foreach i,$(MSGS),$(i)/$(i).msg) dtinfo_msg.tmpl $(MSGSETS)
	msgs=; for i in $(MSGS) ; do msgs="$$msgs $$i/$$i.msg"; done; \
	cat $$msgs >dtinfo.cpp && \
	$(CPP_PROGRAM) -traditional -nostdinc -P -C -I. $(srcdir)/dtinfo_msg.tmpl | $(MSGSETS) > $@.tmp && \
	mv -f $@.tmp $@; \
	status=$$?; $(RM) dtinfo.cpp; exit $$status

CLEANFILES += dtinfo.msg.updated dtinfo.msg.updated.tmp dtinfo.cpp Prelude.h.tmp

## dtinfo proper
OLIASNORMALLIBS= Agents/libAgents.a Other/libOther.a			\
           Basic/libBasic.a Preferences/libPreferences.a		\
           UAS/libUAS.a Query/libQuery.a Graphics/libGraphics.a	\
           Managers/libManagers.a Support/libSupport.a		\
           Marks/libMarks.a OnlineRender/libOnlineRender.a		\
           OliasSearch/libOliasSearch.a Widgets/libWidgets.a		\
           cgm/libcgm.a

bin_PROGRAMS = dtinfo

dtinfo_CPPFLAGS = -I. -I$(srcdir) $(DTINFO_DIR_CPPFLAGS)
dtinfo_CXXFLAGS = $(CDE_CXXFLAGS) $(DTINFO_DEFINES) $(DTINFO_INCLUDES)

dtinfo_LDADD = $(OLIASNORMALLIBS) $(WWL_LIBS) $(LIBMMDB) $(LIBSEARCH) \
	$(LIBPRINT) $(LIBHELP) $(LIBSVC) $(LIBTT) \
	$(XMULIB) $(XMLIB) $(XTLIB) $(X11LIB) $(MATHLIB)

dtinfo_SOURCES = main.C Support/InputNotifier.C
