MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libutility.la

libutility_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB -I..

libutility_la_SOURCES = funcs.C         pm_random.C \
			atoi_pearson.C 	xtime.C         buffer.C \
		        atoi_larson.C   atomic_lock.C 	rw_lock.C \
		        atoi_fast.C     filter.C        mmdb_exception.C \
			randomize.C

noinst_HEADERS = \
    atoi_fast.h \
    atoi_larson.h \
    atomic_lock.h \
    buffer.h \
    config.h \
    const.h \
    debug.h \
    filter.h \
    funcs.h \
    mmdb_exception.h \
    pm_random.h \
    randomize.h \
    xtime.h
