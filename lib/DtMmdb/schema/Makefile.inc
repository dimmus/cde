noinst_LTLIBRARIES += %reldir%/libschema.la

%canon_reldir%_libschema_la_CXXFLAGS = $(CDE_CXXFLAGS)

%canon_reldir%_libschema_la_SOURCES = %reldir%/desc.C \
	%reldir%/store_desc.C %reldir%/object_dict.C \
	%reldir%/stored_object_desc.C %reldir%/index_desc.C %reldir%/inv_desc.C \
	%reldir%/agent_desc.C %reldir%/container_desc.C
nodist_%canon_reldir%_libschema_la_SOURCES = %reldir%/sheet.C %reldir%/token.C

CLEANFILES += %reldir%/sheet.C %reldir%/sheet.tab.h %reldir%/token.C

%reldir%/sheet.C: %reldir%/sheet.yy
	$(AM_V_GEN)$(MKDIR_P) $(@D) && \
	$(YACC) $(AM_YFLAGS) -p schema -b $(@D)/sheet $< && \
	mv -f $(@D)/sheet.tab.c $@

%reldir%/token.C: %reldir%/token.ll
	$(AM_V_GEN)$(MKDIR_P) $(@D) && $(LEX) $(AM_LFLAGS) -P schema -o $@ $<

# The scanner includes the sheet.tab.h that the parser rule writes next
# to sheet.C: make the parser first.
BUILT_SOURCES += %reldir%/sheet.C

EXTRA_DIST += %reldir%/sheet.yy %reldir%/token.ll

noinst_HEADERS += %reldir%/agent_desc.h %reldir%/container_desc.h \
	%reldir%/desc.h %reldir%/index_desc.h %reldir%/inv_desc.h \
	%reldir%/object_dict.h %reldir%/stored_object_desc.h
