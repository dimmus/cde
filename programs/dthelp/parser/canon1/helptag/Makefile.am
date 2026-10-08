MAINTAINERCLEANFILES = Makefile.in

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
HELPIFFILE    = $(HELPTAGSRC)/help.if
HELPTSSFILE   = $(HELPTAGSRC)/htag.tss

XDONE:
	cd ../parser; $(MAKE) CDONE
	touch XDONE

BUILT_SOURCES = XDONE
CLEANFILES = $(BUILT_SOURCES)

AM_CPPFLAGS = -I$(HELPPARSERSRC) -I$(HELPUTILSRC) \
	    -I$(HELPELTDEFSRC) $(DT_INCDIR)

# needed by parser/
noinst_LIBRARIES = libhelptag.a
libhelptag_a_SOURCES = custom.c default.c help.c option.c \
		      special.c out.c xref.c global.c ../parser/delim.h
