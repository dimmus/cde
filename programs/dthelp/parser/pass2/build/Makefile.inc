# Included by ../../Makefile.am (%reldir% is pass2/build).

noinst_PROGRAMS += %reldir%/build
%canon_reldir%_build_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% $(HELP_CPPFLAGS)
%canon_reldir%_build_CFLAGS = $(CDE_CFLAGS) -I$(srcdir)/pass2/util
%canon_reldir%_build_LDADD = $(HELPUTILLIB)
%canon_reldir%_build_SOURCES = %reldir%/build.c %reldir%/buildutl.c \
	%reldir%/eltree.c %reldir%/except.c %reldir%/fsa.c %reldir%/out.c \
	%reldir%/param.c %reldir%/scan.c %reldir%/sref.c %reldir%/tree.c

# the context tables (CPRODS)
%canon_reldir%_CPRODS = %reldir%/case.c %reldir%/context.h %reldir%/delim.h
$(%canon_reldir%_CPRODS): %reldir%/CDONE
%reldir%/CDONE: $(HELPCONTEXT) $(srcdir)/pass2/htag2/delim.bld \
		%reldir%/context.dat
	$(AM_V_GEN)$(MKDIR_P) %reldir% && cd %reldir% && \
	rm -f case.c context.h delim.h error delim.dat && \
	cp $(abs_srcdir)/pass2/htag2/delim.bld delim.dat && \
	if test -f context.dat; then $(RUN_CONTEXT) sparse; else \
	  cp $(abs_srcdir)/%reldir%/context.dat . && \
	  { $(RUN_CONTEXT) sparse; st=$$?; rm -f context.dat; test $$st = 0; }; \
	fi && touch CDONE
CLEANFILES += $(%canon_reldir%_CPRODS) %reldir%/error %reldir%/delim.dat \
	%reldir%/CDONE

# build includes its CPRODS.
$(%canon_reldir%_build_OBJECTS): %reldir%/CDONE

# the DTD tables (BPRODS)
%canon_reldir%_BPRODS = %reldir%/arc.h %reldir%/dtd.h %reldir%/entity.h
$(%canon_reldir%_BPRODS): %reldir%/BDONE
%reldir%/BDONE: %reldir%/build$(EXEEXT) $(srcdir)/pass2/htag2/sdl.dtd
	$(AM_V_GEN)cd %reldir% && rm -f arc.h dtd.h entity.h error template && \
	./build < $(abs_srcdir)/pass2/htag2/sdl.dtd && touch BDONE
CLEANFILES += $(%canon_reldir%_BPRODS) %reldir%/template %reldir%/BDONE
