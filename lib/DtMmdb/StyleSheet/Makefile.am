MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libStyleSheet.la

libStyleSheet_la_CXXFLAGS = $(CDE_CXXFLAGS) -DEXPAND_TEMPLATES -DCDE_NEXT -I..

libStyleSheet_la_SOURCES = Attribute.C \
			   AttributeList.C \
			   BitVector.C \
			   DocParser.C \
			   Element.C \
			   Expression.C \
			   Feature.C \
			   FeatureDefDictionary.C \
			   FeatureSet.C \
			   FeatureValue.C \
			   PathQualifier.C \
			   PathTable.C \
			   RendererHCV.C \
			   Resolver.C \
			   ResolverStack.C \
			   SSPath.C \
			   SSTemplates.C \
			   StyleSheet.C \
			   StyleSheetExceptions.C \
			   SymTab.C \
			   VariableTable.C \
			   defParser.C \
			   defToken.C \
			   style.C \
			   tokenStyle.C

AM_YFLAGS = -l -d
AM_LFLAGS = -8 -s

CLEANFILES = defParser.C \
	     defParser.tab.h \
	     defToken.C \
	     style.C \
	     style.tab.h \
	     tokenStyle.C

defParser.C: defParser.yy
	$(YACC) $(AM_YFLAGS) -p defParser -b defParser $<
	mv defParser.tab.c $@

defToken.C: defToken.ll
	$(LEX) $(AM_LFLAGS) -P defParser -o $@ $<

style.C: style.yy
	$(YACC) $(AM_YFLAGS) -p style -b style $<
	mv style.tab.c style.C

tokenStyle.C: tokenStyle.ll
	$(LEX) $(AM_LFLAGS) -P style -o $@ $<

EXTRA_DIST = \
    defParser.yy \
    defToken.ll \
    style.yy \
    tokenStyle.ll

noinst_HEADERS = \
    Attribute.h \
    AttributeList.h \
    BitVector.h \
    Debug.h \
    Feature.h \
    FeatureDefDictionary.h \
    PathQualifier.h \
    PathTable.h \
    RendererHCV.h \
    ResolverStack.h \
    StyleSheet.h \
    StyleSheetExceptions.h \
    SymTab.h \
    Types.h
