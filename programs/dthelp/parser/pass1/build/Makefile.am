MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = build

.NOTPARALLEL:

HELPTAG       = ..
HELPTAGSRC    = $(HELPTAG)/helptag
HELPPARSERSRC = $(HELPTAG)/parser
HELPELTDEFSRC = $(HELPTAG)/eltdef
HELPDTDFILE   = $(HELPTAGSRC)/hptag.dtd
HELPUTILSRC   = $(HELPTAG)/util
HELPBUILDSRC  = $(HELPTAG)/build
HELPUTILLIB   = $(HELPTAG)/util/libutil.a

build_CFLAGS = -I$(HELPUTILSRC)

build_LDADD = ../util/libutil.a

build_SOURCES = build.c buildutl.c eltree.c except.c fsa.c out.c param.c \
		scan.c sref.c tree.c

CHPRODS = context.h delim.h
CCPRODS = case.c

BHPRODS = arc.h dtd.h entity.h
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
BDONE: ./build $(HELPDTDFILE)
	$(RM) $(BPRODS) $(BTRASH) && ./build < $(HELPDTDFILE) && touch BDONE


# build the CRPODS
CDONE: $(HELPUTILSRC)/context $(HELPTAGSRC)/delim.bld context.dat
	$(RM) $(CPRODS) $(CTRASH)
	$(CP) $(HELPTAGSRC)/delim.bld delim.dat # appl.-specific delim.dat
	$(HELPUTILSRC)/context sparse
	touch CDONE

