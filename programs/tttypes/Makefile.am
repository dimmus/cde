MAINTAINERCLEANFILES = Makefile.in

# Merged into types.xdr in this order; a type defined in a later file
# replaces one of the same name from an earlier file.
PTYPES = \
        dtfile.ptype \
        dticon.ptype \
        dtpad.ptype \
        dtcm.ptype \
        dtmail.ptype \
        dtdbcache.ptype \
        dtterm.ptype \
        dtinfo.ptype \
        dtinfo_start.ptype

BUILT_SOURCES = types.xdr
CLEANFILES = types.xdr .tt_lock *.ptype.deps *.done

tttypesdir = $(CDE_INSTALLATION_TOP)/appconfig/tttypes
dist_tttypes_DATA = $(BUILT_SOURCES)

TT_TYPE_COMP = $(top_builddir)/lib/tt/bin/tt_type_comp/tt_type_comp

# One tt_type_comp run parses every file and writes types.xdr once.
# DISPLAY and the session variables are unset so that tt_type_comp
# does not contact, or autostart, a ttsession just to announce the
# new build-tree types.xdr (the libtool wrapper would also make an
# installed ttsession run against the build tree's libtt).
# (The sources are looked up in $(srcdir) as well, for VPATH builds.)
types.xdr: $(PTYPES) dtinfo_start.opnums $(TT_TYPE_COMP)
	$(RM) $@ .tt_lock
	srcs=; for f in $(PTYPES); do \
	  if test -f $$f; then srcs="$$srcs $$f"; \
	  else srcs="$$srcs $(srcdir)/$$f"; fi; \
	done; \
	(unset DISPLAY TT_SESSION _SUN_TT_SESSION; \
	 TTPATH=. $(TT_TYPE_COMP) -sd user -m $$srcs); \
	status=$$?; $(RM) .tt_lock; exit $$status
