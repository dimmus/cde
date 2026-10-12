noinst_LTLIBRARIES += %reldir%/libcompression.la

# (-I: the lex and yacc outputs, which are in the build directory, include
# this directory's headers)
%canon_reldir%_libcompression_la_CXXFLAGS = $(CDE_CXXFLAGS) \
	-I$(srcdir)/%reldir%

%canon_reldir%_libcompression_la_SOURCES = %reldir%/abs_agent.C \
	%reldir%/zip.C %reldir%/huffman.C %reldir%/trie.C %reldir%/code.C \
	%reldir%/lzss.C
nodist_%canon_reldir%_libcompression_la_SOURCES = %reldir%/sgml.C %reldir%/ps.C

CLEANFILES += %reldir%/sgml.C %reldir%/ps.C

%reldir%/sgml.C: %reldir%/sgml.ll
	$(AM_V_GEN)$(MKDIR_P) $(@D) && $(LEX) $(AM_LFLAGS) -P sgml -o $@ $<

%reldir%/ps.C: %reldir%/ps.ll
	$(AM_V_GEN)$(MKDIR_P) $(@D) && $(LEX) $(AM_LFLAGS) -P ps -o $@ $<

EXTRA_DIST += %reldir%/ps.ll %reldir%/sgml.ll

noinst_HEADERS += %reldir%/abs_agent.h %reldir%/huffman.h %reldir%/lzss.h \
	%reldir%/trie.h %reldir%/zip.h
