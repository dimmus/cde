MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dthelp_htag2

# Due to the wierd interdepencencies between helptag and parser,
# disable parallel builds here.  This is some really horrible code and
# design - not surprising since it dates to 1989.
.NOTPARALLEL:

HTAG2       = ..
HTAG2SRC    = $(HTAG2)/htag2
SDLPARSERSRC = $(HTAG2)/parser
SDLELTDEFSRC = $(HTAG2)/eltdef
SDLDTDFILE   = $(HTAG2SRC)/sdl.dtd
SDLUTILSRC   = $(HTAG2)/util
SDLBUILDSRC  = $(HTAG2)/build
SDLUTILLIB   = $(HTAG2)/util/libutil.a
SDLIFFILE    = $(HTAG2SRC)/help.if

AM_CPPFLAGS = -I. -I$(SDLUTILSRC) -I$(HTAG2SRC) -I$(SDLBUILDSRC) \
	   -I$(SDLELTDEFSRC)

dthelp_htag2_SOURCES = parser.c actutil.c \
        closent.c code.c cyclent.c dcurelt.c \
        end.c entity.c \
        error.c getsign.c \
        input.c level.c lookent.c \
        minim.c optstrg.c param.c parent.c \
        parutil.c pi.c \
        scan.c setpar.c start.c string.c struct.c \
        text.c

APPLOBJECTS = ../htag2/libhelptag.a

dthelp_htag2_LDADD = $(APPLOBJECTS) $(SDLUTILLIB) $(DTCLIENTLIBS) $(XTOOLLIB)

CPRODS = case.c context.h delim.h
CTRASH = error delim.dat

BUILT_SOURCES = $(CPRODS)
CLEANFILES = $(BUILT_SOURCES) $(CTRASH) CDONE

$(CPRODS): CDONE

CDONE:  $(SDLUTILSRC)/context $(HTAG2SRC)/delim.par context.dat
	$(RM) $(CPRODS) $(CTRASH)
	$(CP) $(HTAG2SRC)/delim.par delim.dat  # appl.-specific delim.dat
	$(SDLUTILSRC)/context
	touch CDONE

