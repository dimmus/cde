MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dthelp_ctag1

# Due to the wierd interdepencencies between helptag and parser,
# disable parallel builds here.  This is some really horrible code and
# design - not surprising since it dates to 1989.
.NOTPARALLEL:

HELPTAG       = ..
HELPTAGSRC    = $(HELPTAG)/helptag
HELPPARSERSRC = $(HELPTAG)/parser
HELPELTDEFSRC = $(HELPTAG)/eltdef
HELPDTDFILE   = $(HELPTAGSRC)/hptag.dtd
HELPUTILSRC   = $(HELPTAG)/util
HELPBUILDSRC  = $(HELPTAG)/build
HELPUTILLIB   = $(HELPTAG)/util/libutil.a

AM_CPPFLAGS = -I$(HELPUTILSRC) -I$(HELPTAGSRC) -I$(HELPBUILDSRC) \
	   -I$(HELPELTDEFSRC)

dthelp_ctag1_SOURCES = parser.c actutil.c \
        closent.c code.c cyclent.c dcurelt.c \
        end.c entity.c \
        error.c getsign.c \
        input.c level.c lookent.c \
        minim.c optstrg.c param.c parent.c \
        parutil.c pi.c \
        scan.c setpar.c start.c string.c struct.c \
        text.c

APPLOBJECTS = ../helptag/libhelptag.a

dthelp_ctag1_LDADD = $(APPLOBJECTS) $(HELPUTILLIB) $(DTCLIENTLIBS) $(XTOOLLIB)

CPRODS = case.c context.h delim.h
CTRASH = error delim.dat

BUILT_SOURCES = $(CPRODS)
CLEANFILES = $(BUILT_SOURCES) $(CTRASH) CDONE

$(CPRODS): CDONE

CDONE:  $(HELPUTILSRC)/context $(HELPTAGSRC)/delim.par context.dat
	$(RM) $(CPRODS) $(CTRASH)
	$(CP) $(HELPTAGSRC)/delim.par delim.dat  # appl.-specific delim.dat
	$(HELPUTILSRC)/context
	touch CDONE

