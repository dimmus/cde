noinst_LTLIBRARIES += %reldir%/libStyleSheet.la

%canon_reldir%_libStyleSheet_la_CXXFLAGS = $(CDE_CXXFLAGS) -DEXPAND_TEMPLATES -DCDE_NEXT

%canon_reldir%_libStyleSheet_la_SOURCES = %reldir%/Attribute.C \
	%reldir%/AttributeList.C %reldir%/BitVector.C %reldir%/DocParser.C \
	%reldir%/Element.C %reldir%/Expression.C %reldir%/Feature.C \
	%reldir%/FeatureDefDictionary.C %reldir%/FeatureSet.C \
	%reldir%/FeatureValue.C %reldir%/PathQualifier.C %reldir%/PathTable.C \
	%reldir%/RendererHCV.C %reldir%/Resolver.C %reldir%/ResolverStack.C \
	%reldir%/SSPath.C %reldir%/SSTemplates.C %reldir%/StyleSheet.C \
	%reldir%/StyleSheetExceptions.C %reldir%/SymTab.C \
	%reldir%/VariableTable.C
nodist_%canon_reldir%_libStyleSheet_la_SOURCES = %reldir%/defParser.C \
	%reldir%/defToken.C %reldir%/style.C %reldir%/tokenStyle.C

CLEANFILES += %reldir%/defParser.C %reldir%/defParser.tab.h \
	%reldir%/defToken.C %reldir%/style.C %reldir%/style.tab.h \
	%reldir%/tokenStyle.C

%reldir%/defParser.C: %reldir%/defParser.yy
	$(AM_V_GEN)$(MKDIR_P) $(@D) && \
	$(YACC) $(AM_YFLAGS) -p defParser -b $(@D)/defParser $< && \
	mv -f $(@D)/defParser.tab.c $@

%reldir%/defToken.C: %reldir%/defToken.ll
	$(AM_V_GEN)$(MKDIR_P) $(@D) && $(LEX) $(AM_LFLAGS) -P defParser -o $@ $<

%reldir%/style.C: %reldir%/style.yy
	$(AM_V_GEN)$(MKDIR_P) $(@D) && \
	$(YACC) $(AM_YFLAGS) -p style -b $(@D)/style $< && \
	mv -f $(@D)/style.tab.c $@

%reldir%/tokenStyle.C: %reldir%/tokenStyle.ll
	$(AM_V_GEN)$(MKDIR_P) $(@D) && $(LEX) $(AM_LFLAGS) -P style -o $@ $<

# The scanners include the .tab.h that the parser rules write next to
# the parsers' .C files: make the parsers first.
BUILT_SOURCES += %reldir%/defParser.C %reldir%/style.C

EXTRA_DIST += %reldir%/defParser.yy %reldir%/defToken.ll \
	%reldir%/style.yy %reldir%/tokenStyle.ll

noinst_HEADERS += %reldir%/Attribute.h %reldir%/AttributeList.h \
	%reldir%/BitVector.h %reldir%/Debug.h %reldir%/Feature.h \
	%reldir%/FeatureDefDictionary.h %reldir%/PathQualifier.h \
	%reldir%/PathTable.h %reldir%/RendererHCV.h %reldir%/ResolverStack.h \
	%reldir%/StyleSheet.h %reldir%/StyleSheetExceptions.h %reldir%/SymTab.h \
	%reldir%/Types.h
