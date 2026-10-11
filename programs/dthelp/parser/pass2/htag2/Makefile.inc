# Included by ../../Makefile.am (%reldir% is pass2/htag2).

noinst_LIBRARIES += %reldir%/libhelptag.a
%canon_reldir%_libhelptag_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(HELP_CPPFLAGS) -Ipass2/parser -I$(srcdir)/pass2/parser \
	-I$(srcdir)/pass2/util -Ipass2/eltdef \
	-I$(srcdir)/pass2/eltdef $(DT_INCDIR)
%canon_reldir%_libhelptag_a_SOURCES = %reldir%/custom.c %reldir%/sdl.c \
	%reldir%/option.c %reldir%/out.c %reldir%/global.c %reldir%/lzw.c \
	%reldir%/lzw.h

# The library includes the action tables of eltdef and the delimiter
# table of the parser.
$(%canon_reldir%_libhelptag_a_OBJECTS): pass2/eltdef/EDONE \
	pass2/parser/CDONE
