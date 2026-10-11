noinst_LTLIBRARIES += %reldir%/libutility.la

%canon_reldir%_libutility_la_CXXFLAGS = $(CDE_CXXFLAGS) -DPORTABLE_DB

%canon_reldir%_libutility_la_SOURCES = %reldir%/funcs.C \
	%reldir%/pm_random.C %reldir%/atoi_pearson.C %reldir%/xtime.C \
	%reldir%/buffer.C %reldir%/atoi_larson.C %reldir%/atomic_lock.C \
	%reldir%/rw_lock.C %reldir%/atoi_fast.C %reldir%/filter.C \
	%reldir%/mmdb_exception.C %reldir%/randomize.C

noinst_HEADERS += %reldir%/atoi_fast.h %reldir%/atoi_larson.h \
	%reldir%/atomic_lock.h %reldir%/buffer.h %reldir%/config.h \
	%reldir%/const.h %reldir%/debug.h %reldir%/filter.h %reldir%/funcs.h \
	%reldir%/mmdb_exception.h %reldir%/pm_random.h %reldir%/randomize.h \
	%reldir%/xtime.h
