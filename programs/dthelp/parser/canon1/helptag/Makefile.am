MAINTAINERCLEANFILES = Makefile.in

HELPTAG       = ..
HELPTAGSRC    = $(HELPTAG)/helptag
HELPPARSERSRC = $(HELPTAG)/parser
HELPELTDEFSRC = $(HELPTAG)/eltdef
HELPDTDFILE   = $(HELPTAGSRC)/hptag.dtd
HELPUTILSRC   = $(HELPTAG)/util
HELPBUILDSRC  = $(HELPTAG)/build
HELPUTILLIB   = $(HELPTAG)/../util/libhelputil.a
HELPCONTEXT   = $(HELPTAG)/../util/context
HELPIFFILE    = $(HELPTAGSRC)/help.if
HELPTSSFILE   = $(HELPTAGSRC)/htag.tss

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

AM_CPPFLAGS = $(CDE_CPPFLAGS) $(KEEP_ASSERTS) -I$(HELPPARSERSRC) -I$(HELPUTILSRC) \
	    -I$(HELPELTDEFSRC) $(DT_INCDIR)

# needed by parser/
noinst_LIBRARIES = libhelptag.a
libhelptag_a_SOURCES = custom.c default.c help.c option.c \
		      special.c out.c xref.c global.c ../parser/delim.h
