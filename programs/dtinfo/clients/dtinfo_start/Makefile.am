MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

startinstdir = $(CDE_INSTALLATION_TOP)/infolib/etc
startinst_PROGRAMS = dtinfo_start

BUILT_SOURCES = dtinfo_start.opnums

dtinfo_start_LDADD = $(LIBSVC) $(LIBTT) $(XMLIB) $(XTLIB) $(X11LIB)

dtinfo_start.opnums: $(top_builddir)/programs/tttypes/dtinfo_start.opnums
	$(RM) dtinfo_start.opnums
	$(LN_S) $? $@

dtinfo_start_SOURCES = dtinfo_start.c
