MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libDtMrm.la

libDtMrm_la_CFLAGS = $(CDE_CFLAGS) -I../DtTerm/Term -I../DtTerm/TermPrim -I../DtHelp \
		     @DT_INCDIR@ @DTI_INCDIR@

libDtMrm_la_SOURCES = DtMrm.c

libDtMrm_la_LIBADD = $(MRESOURCELIB)

# in order to try to keep lib versions the same across platforms, (2.1.0)
if BSD
libDtMrm_la_LDFLAGS = $(CDE_LDFLAGS) -version-info 2:1:0
else
libDtMrm_la_LDFLAGS = $(CDE_LDFLAGS) -version-info 3:0:1
endif

