MAINTAINERCLEANFILES = Makefile.in

.NOTPARALLEL:

noinst_PROGRAMS = build

HTAG2       = ..
HTAG2SRC    = $(HTAG2)/htag2
SDLPARSERSRC = $(HTAG2)/parser
SDLELTDEFSRC = $(HTAG2)/eltdef
SDLDTDFILE   = $(HTAG2SRC)/sdl.dtd
SDLUTILSRC   = $(HTAG2)/util
SDLBUILDSRC  = $(HTAG2)/build
SDLUTILLIB   = $(HTAG2)/util/libutil.a
SDLIFFILE    = $(HTAG2SRC)/help.if

build_CFLAGS = -I$(SDLUTILSRC)

build_LDADD = ../util/libutil.a

build_SOURCES = build.c buildutl.c eltree.c except.c fsa.c out.c param.c \
		scan.c sref.c tree.c

CHPRODS = context.h delim.h
CCPRODS = case.c

BHPRODS = dtd.h entity.h arc.h
BCPRODS =

CTRASH  = error delim.dat
BTRASH  = error template

CPRODS  = $(CHPRODS) $(CCPRODS)
BPRODS  = $(BHPRODS) $(BCPRODS)
TRASH   = error delim.dat template

BUILT_SOURCES = $(CPRODS) $(BPRODS)
CLEANFILES = $(BUILT_SOURCES) $(TRASH) BDONE CDONE

# We want the commands that actually create BPRODS and CPRODS to only
# run once during a make so we gate them using these CDONE and BDONE
# targets.  This allows us to enable parallel building too.
$(BPRODS): BDONE

$(CPRODS): CDONE

# build the BRPODS
BDONE: ./build $(SDLDTDFILE)
	$(RM) $(BPRODS) $(BTRASH) && ./build < $(SDLDTDFILE) && touch BDONE


# build the CRPODS
CDONE: $(SDLUTILSRC)/context $(HTAG2SRC)/delim.bld context.dat
	$(RM) $(CPRODS) $(CTRASH)
	$(CP) $(HTAG2SRC)/delim.bld delim.dat # appl.-specific delim.dat
	$(SDLUTILSRC)/context sparse
	touch CDONE

