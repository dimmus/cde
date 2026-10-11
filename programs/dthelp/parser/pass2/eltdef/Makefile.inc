# Included by ../../Makefile.am (%reldir% is pass2/eltdef).

noinst_PROGRAMS += %reldir%/eltdef
%canon_reldir%_eltdef_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% $(HELP_CPPFLAGS)
%canon_reldir%_eltdef_CFLAGS = $(CDE_CFLAGS) -I$(srcdir)/pass2/util \
	-Ipass2/build -I$(srcdir)/pass2/build
%canon_reldir%_eltdef_LDADD = $(HELPUTILLIB)
%canon_reldir%_eltdef_SOURCES = %reldir%/eltdef.c %reldir%/eltutil.c \
	%reldir%/entity.c %reldir%/scan.c

# the context tables (CPRODS)
%canon_reldir%_CPRODS = %reldir%/case.c %reldir%/context.h %reldir%/delim.h
$(%canon_reldir%_CPRODS): %reldir%/CDONE
%reldir%/CDONE: $(HELPCONTEXT) $(srcdir)/pass2/htag2/delim.elt \
		%reldir%/context.dat
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && \
	rm -f case.c context.h delim.h error delim.dat && \
	cp $(abs_srcdir)/pass2/htag2/delim.elt delim.dat && \
	if test -f context.dat; then $(RUN_CONTEXT); else \
	  cp $(abs_srcdir)/%reldir%/context.dat . && \
	  { $(RUN_CONTEXT); st=$$?; rm -f context.dat; test $$st = 0; }; \
	fi && touch CDONE
CLEANFILES += $(%canon_reldir%_CPRODS) %reldir%/error %reldir%/delim.dat \
	%reldir%/CDONE

# eltdef includes its CPRODS and the DTD tables of build.
$(%canon_reldir%_eltdef_OBJECTS): %reldir%/CDONE pass2/build/BDONE

# the action tables (EPRODS).  eltdef puts the name of the interface file
# it reads in the sign-on message of the pass: give it the same relative
# name as ever, unless the sources are elsewhere (a VPATH build).
%canon_reldir%_EPRODS = %reldir%/efile.c %reldir%/entfile.c %reldir%/pfile.c \
	%reldir%/sfile.c %reldir%/stfile.c %reldir%/tfile.c %reldir%/entity2.h \
	%reldir%/globdec.h %reldir%/globdef.h %reldir%/if.h %reldir%/pval.h \
	%reldir%/signon.h %reldir%/signonx.h
$(%canon_reldir%_EPRODS): %reldir%/EDONE
%reldir%/EDONE: %reldir%/eltdef$(EXEEXT) $(srcdir)/pass2/htag2/sdl.if
	$(AM_V_GEN)cd %reldir% && \
	rm -f $(notdir $(%canon_reldir%_EPRODS)) estring.h error && \
	f=../htag2/sdl.if; test -f $$f || f=$(abs_srcdir)/pass2/htag2/sdl.if; \
	./eltdef $$f && touch EDONE
CLEANFILES += $(%canon_reldir%_EPRODS) %reldir%/estring.h %reldir%/EDONE
