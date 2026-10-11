docsdir = $(CDE_INSTALLATION_TOP)
docs_DATA = CONTRIBUTORS COPYING copyright HISTORY README.md

MAINTAINERCLEANFILES = Makefile.in \
            configure libtool \
            config.guess \
            config.sub \
            config.log \
            depcomp \
            ltconfig ltmain.sh \
            aclocal.m4 \
            config.h.in \
            install-sh \
            missing mkinstalldirs \
            ylwrap \
            compile \
            m4/lt*.m4 \
            m4/libtool.m4 \
            build-aux/compile \
            build-aux/config.guess \
            build-aux/config.sub \
            build-aux/depcomp \
            build-aux/install-sh \
            build-aux/ltmain.sh \
            build-aux/missing \
            build-aux/ylwrap \
            *~

SUBDIRS = util lib programs include

# include/Dt/Dt.h is made by configure (see configure.ac) and remade here,
# before the subdirectories, when Dt.h.in or the version changes.  It is
# replaced only when its content changes, so that a reconfigure does not
# rebuild everything that includes it.
BUILT_SOURCES = include/Dt/Dt.h
DISTCLEANFILES = include/Dt/Dt.h include/Dt/Dt.h.tmp

# (spelt this way so that config.status does not substitute the patterns)
DT_H_AT = @

include/Dt/Dt.h: $(top_srcdir)/include/Dt/Dt.h.in $(top_builddir)/config.status
	@sed -e 's|$(DT_H_AT)CDE_VERSION_MAJOR$(DT_H_AT)|$(CDE_VERSION_MAJOR)|g' \
	    -e 's|$(DT_H_AT)CDE_VERSION_MINOR$(DT_H_AT)|$(CDE_VERSION_MINOR)|g' \
	    -e 's|$(DT_H_AT)CDE_VERSION_MICRO$(DT_H_AT)|$(CDE_VERSION_MICRO)|g' \
	    -e 's|$(DT_H_AT)CDE_VERSION_DEV$(DT_H_AT)|$(CDE_VERSION_DEV)|g' \
	    $(top_srcdir)/include/Dt/Dt.h.in > $@.tmp
	@if cmp -s $@.tmp $@; then rm -f $@.tmp; \
	else echo "updating $@"; mv -f $@.tmp $@; fi

if BUILD_DOCS
SUBDIRS += doc
endif

