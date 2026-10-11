# Included by ../../Makefile.am (%reldir% is canon1/parser).

bin_PROGRAMS += %reldir%/dthelp_ctag1
%canon_reldir%_dthelp_ctag1_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(HELP_CPPFLAGS) -I$(srcdir)/canon1/util \
	-I$(srcdir)/canon1/helptag -Icanon1/build \
	-I$(srcdir)/canon1/build -Icanon1/eltdef \
	-I$(srcdir)/canon1/eltdef
%canon_reldir%_dthelp_ctag1_SOURCES = %reldir%/parser.c %reldir%/actutil.c \
	%reldir%/closent.c %reldir%/code.c %reldir%/cyclent.c %reldir%/dcurelt.c \
	%reldir%/end.c %reldir%/entity.c %reldir%/error.c %reldir%/getsign.c \
	%reldir%/input.c %reldir%/level.c %reldir%/lookent.c %reldir%/minim.c \
	%reldir%/optstrg.c %reldir%/param.c %reldir%/parent.c %reldir%/parutil.c \
	%reldir%/pi.c %reldir%/scan.c %reldir%/setpar.c %reldir%/start.c \
	%reldir%/string.c %reldir%/struct.c %reldir%/text.c
%canon_reldir%_dthelp_ctag1_LDADD = canon1/helptag/libhelptag.a $(HELPUTILLIB)

# the context tables (CPRODS)
%canon_reldir%_CPRODS = %reldir%/case.c %reldir%/context.h %reldir%/delim.h
$(%canon_reldir%_CPRODS): %reldir%/CDONE
%reldir%/CDONE: $(HELPCONTEXT) $(srcdir)/canon1/helptag/delim.par \
		%reldir%/context.dat
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && \
	rm -f case.c context.h delim.h error delim.dat && \
	cp $(abs_srcdir)/canon1/helptag/delim.par delim.dat && \
	if test -f context.dat; then $(RUN_CONTEXT); else \
	  cp $(abs_srcdir)/%reldir%/context.dat . && \
	  { $(RUN_CONTEXT); st=$$?; rm -f context.dat; test $$st = 0; }; \
	fi && touch CDONE
CLEANFILES += $(%canon_reldir%_CPRODS) %reldir%/error %reldir%/delim.dat \
	%reldir%/CDONE

# The parser includes its CPRODS, the DTD tables of build and the action
# tables of eltdef.
$(%canon_reldir%_dthelp_ctag1_OBJECTS): %reldir%/CDONE canon1/build/BDONE \
	canon1/eltdef/EDONE
