# Included by ../../Makefile.am (%reldir% is pass2/parser).

bin_PROGRAMS += %reldir%/dthelp_htag2
%canon_reldir%_dthelp_htag2_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% \
	$(HELP_CPPFLAGS) -I%reldir% -I$(srcdir)/%reldir% \
	-I$(srcdir)/pass2/util -I$(srcdir)/pass2/htag2 \
	-Ipass2/build -I$(srcdir)/pass2/build -Ipass2/eltdef \
	-I$(srcdir)/pass2/eltdef
%canon_reldir%_dthelp_htag2_SOURCES = %reldir%/parser.c %reldir%/actutil.c \
	%reldir%/closent.c %reldir%/code.c %reldir%/cyclent.c %reldir%/dcurelt.c \
	%reldir%/end.c %reldir%/entity.c %reldir%/error.c %reldir%/getsign.c \
	%reldir%/input.c %reldir%/level.c %reldir%/lookent.c %reldir%/minim.c \
	%reldir%/optstrg.c %reldir%/param.c %reldir%/parent.c %reldir%/parutil.c \
	%reldir%/pi.c %reldir%/scan.c %reldir%/setpar.c %reldir%/start.c \
	%reldir%/string.c %reldir%/struct.c %reldir%/text.c
%canon_reldir%_dthelp_htag2_LDADD = pass2/htag2/libhelptag.a $(HELPUTILLIB)

# the context tables (CPRODS)
%canon_reldir%_CPRODS = %reldir%/case.c %reldir%/context.h %reldir%/delim.h
$(%canon_reldir%_CPRODS): %reldir%/CDONE
%reldir%/CDONE: $(HELPCONTEXT) $(srcdir)/pass2/htag2/delim.par \
		%reldir%/context.dat
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && \
	rm -f case.c context.h delim.h error delim.dat && \
	cp $(abs_srcdir)/pass2/htag2/delim.par delim.dat && \
	if test -f context.dat; then $(RUN_CONTEXT); else \
	  cp $(abs_srcdir)/%reldir%/context.dat . && \
	  { $(RUN_CONTEXT); st=$$?; rm -f context.dat; test $$st = 0; }; \
	fi && touch CDONE
CLEANFILES += $(%canon_reldir%_CPRODS) %reldir%/error %reldir%/delim.dat \
	%reldir%/CDONE

# The parser includes its CPRODS, the DTD tables of build and the action
# tables of eltdef.
$(%canon_reldir%_dthelp_htag2_OBJECTS): %reldir%/CDONE pass2/build/BDONE \
	pass2/eltdef/EDONE
