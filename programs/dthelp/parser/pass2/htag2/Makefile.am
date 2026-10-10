MAINTAINERCLEANFILES = Makefile.in

HTAG2       = ..
HTAG2SRC    = $(HTAG2)/htag2
SDLPARSERSRC = $(HTAG2)/parser
SDLELTDEFSRC = $(HTAG2)/eltdef
SDLDTDFILE   = $(HTAG2SRC)/sdl.dtd
SDLUTILSRC   = $(HTAG2)/util
SDLBUILDSRC  = $(HTAG2)/build
SDLUTILLIB   = $(HTAG2)/util/libutil.a
SDLIFFILE    = $(HTAG2SRC)/help.if

# The helptag library includes headers that ../parser generates (its
# CDONE step), and the parser directory is built after this one.  Ask
# ../parser to bring them up to date on every run; XDONE, and with it the
# objects here, are remade only when they did change.
XDONE: ../parser/CDONE
	touch XDONE

../parser/CDONE: FORCE
	cd ../parser && $(MAKE) $(AM_MAKEFLAGS) CDONE

FORCE:
.PHONY: FORCE

$(libhelptag_a_OBJECTS): XDONE

BUILT_SOURCES = XDONE
CLEANFILES = $(BUILT_SOURCES)

AM_CPPFLAGS = $(CDE_CPPFLAGS) -I$(SDLPARSERSRC) -I$(SDLUTILSRC) \
	    -I$(SDLELTDEFSRC) $(DT_INCDIR)

# needed by parser/
noinst_LIBRARIES = libhelptag.a
libhelptag_a_SOURCES = custom.c sdl.c option.c out.c global.c lzw.c lzw.h \
		       ../parser/delim.h
