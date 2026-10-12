MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

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

tttypesdir = $(CDE_DATA_TOP)/appconfig/tttypes
dist_tttypes_DATA = $(BUILT_SOURCES)

TT_TYPE_COMP = $(top_builddir)/lib/tt/bin/tt_type_comp/tt_type_comp

# Let the dtinfo ptypes pick up CDE_DATA_TOP / CDE_INSTALLATION_TOP from
# cde_config.h via tt_type_comp's internal cpp pass, so their start
# strings resolve against the configured --with-cde-* layout.
TT_CPP_FLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include

# One tt_type_comp run parses every file and writes types.xdr once.
# DISPLAY and the session variables are unset so that tt_type_comp
# does not contact, or autostart, a ttsession just to announce the
# new build-tree types.xdr (the libtool wrapper would also make an
# installed ttsession run against the build tree's libtt).
# tt_type_comp has cpp write a dependency file next to each source
# (X.ptype.deps): in a VPATH build, where the source directory may be
# read-only, it reads copies of the sources in $(PTYPES_VPATH).
PTYPES_VPATH = ptypes.vpath
types.xdr: $(PTYPES) dtinfo_start.opnums $(TT_TYPE_COMP)
	$(RM) $@ .tt_lock
	srcs="$(PTYPES)"; \
	if test "$(srcdir)" != .; then \
	  rm -rf $(PTYPES_VPATH) && mkdir $(PTYPES_VPATH) && \
	  for f in $(PTYPES) dtinfo_start.opnums; do \
	    cp $(srcdir)/$$f $(PTYPES_VPATH)/ || exit 1; \
	  done; \
	  srcs=; for f in $(PTYPES); do srcs="$$srcs $(PTYPES_VPATH)/$$f"; done; \
	fi; \
	(unset DISPLAY TT_SESSION _SUN_TT_SESSION; \
	 TTPATH=. $(TT_TYPE_COMP) $(TT_CPP_FLAGS) -sd user -m $$srcs); \
	status=$$?; $(RM) .tt_lock; exit $$status

clean-local:
	rm -rf $(PTYPES_VPATH)
