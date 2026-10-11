# Included by ../../Makefile.am (%reldir% is canon1/helptag).

noinst_LIBRARIES += %reldir%/libhelptag.a
%canon_reldir%_libhelptag_a_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(HELP_CPPFLAGS) $(KEEP_ASSERTS) -Icanon1/parser \
	-I$(srcdir)/canon1/parser -I$(srcdir)/canon1/util \
	-Icanon1/eltdef -I$(srcdir)/canon1/eltdef $(DT_INCDIR) \
	$(HELP_TOP_DEFINES)
%canon_reldir%_libhelptag_a_SOURCES = %reldir%/custom.c %reldir%/default.c \
	%reldir%/help.c %reldir%/option.c %reldir%/special.c %reldir%/out.c \
	%reldir%/xref.c %reldir%/global.c

# The library includes the action tables of eltdef and the delimiter
# table of the parser.
$(%canon_reldir%_libhelptag_a_OBJECTS): canon1/eltdef/EDONE \
	canon1/parser/CDONE
