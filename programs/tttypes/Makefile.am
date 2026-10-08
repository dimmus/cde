MAINTAINERCLEANFILES = Makefile.in

# We need to ensure that the jobs here are run in sequence to build up
# the types.xdr file.
.NOTPARALLEL:

SUFFIXES: .ptype .done

# FIXME - dtinfo ones use #include which seems to fail with syntax error...

LOCALDEPFILES = \
        dtfile.done \
        dticon.done \
        dtpad.done \
        dtcm.done \
        dtmail.done \
        dtdbcache.done \
        dtterm.done \
        dtinfo.done \
        dtinfo_start.done

BUILT_SOURCES = types.xdr
CLEANFILES = *.done types.xdr .tt_lock

tttypesdir = $(CDE_INSTALLATION_TOP)/appconfig/tttypes
dist_tttypes_DATA = $(BUILT_SOURCES)

types.xdr: $(LOCALDEPFILES)

TT_TYPE_COMP = $(top_builddir)/lib/tt/bin/tt_type_comp/tt_type_comp

.ptype.done:
	$(RM) $@
	TTPATH=. $(TT_TYPE_COMP) -sd user -m $< && touch $@ ; $(RM) .tt_lock
