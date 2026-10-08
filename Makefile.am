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

if BUILD_DOCS
SUBDIRS += doc
endif

