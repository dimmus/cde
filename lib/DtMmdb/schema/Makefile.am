MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libschema.la

libschema_la_CXXFLAGS = -I..

libschema_la_SOURCES = desc.C store_desc.C object_dict.C stored_object_desc.C \
		       index_desc.C inv_desc.C agent_desc.C container_desc.C \
		       sheet.C token.C

AM_YFLAGS = -l -d
AM_LFLAGS = -8 -s

CLEANFILES = sheet.C \
	     sheet.tab.h \
	     token.C

sheet.C: sheet.yy
	$(YACC) $(AM_YFLAGS) -p schema -b sheet $<
	mv sheet.tab.c $@

token.C: token.ll
	$(LEX) $(AM_LFLAGS) -P schema -o $@ $<
