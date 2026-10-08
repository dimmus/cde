MAINTAINERCLEANFILES = Makefile.in

.NOTPARALLEL:

noinst_PROGRAMS = eltdef

HTAG2       = ..
HTAG2SRC    = $(HTAG2)/htag2
SDLPARSERSRC = $(HTAG2)/parser
SDLELTDEFSRC = $(HTAG2)/eltdef
SDLDTDFILE   = $(HTAG2SRC)/sdl.dtd
SDLUTILSRC   = $(HTAG2)/util
SDLBUILDSRC  = $(HTAG2)/build
SDLUTILLIB   = $(HTAG2)/util/libutil.a
SDLIFFILE    = $(HTAG2SRC)/sdl.if

eltdef_LDADD = $(SDLUTILLIB)

eltdef_CFLAGS = -I$(SDLUTILSRC) -I$(SDLBUILDSRC)

eltdef_SOURCES = eltdef.c eltutil.c entity.c scan.c

CCPRODS = case.c
ECPRODS = efile.c entfile.c pfile.c sfile.c stfile.c tfile.c

CHPRODS = context.h delim.h
EHPRODS = entity2.h globdec.h globdef.h if.h pval.h signon.h signonx.h

CPRODS  = $(CCPRODS) $(CHPRODS)
EPRODS  = $(ECPRODS) $(EHPRODS)

CTRASH  = delim.dat error
ETRASH  = estring.h error
TRASH   = delim.dat estring.h error

BUILT_SOURCES = $(CPRODS) $(EPRODS)
CLEANFILES = $(BUILT_SOURCES) $(TRASH) CDONE EDONE

$(CPRODS): CDONE

$(EPRODS): EDONE


EDONE: eltdef  $(SDLIFFILE)
	$(RM) $(EPRODS) $(ETRASH)
	./eltdef $(SDLIFFILE)
	touch EDONE

CDONE:  $(SDLUTILSRC)/context $(HTAG2SRC)/delim.elt context.dat
	$(RM) $(CPRODS) $(CTRASH)
	$(CP) $(HTAG2SRC)/delim.elt delim.dat  # appl.-specific delim.dat
	$(SDLUTILSRC)/context
	touch CDONE

