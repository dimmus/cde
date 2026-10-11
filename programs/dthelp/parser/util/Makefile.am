MAINTAINERCLEANFILES = Makefile.in

# The utility library and the "context" generator that the three passes
# of the help tag compiler (pass1, canon1 and pass2) share.  Each pass
# keeps its own headers in <pass>/util, together with the util sources
# that it #includes; those headers differ only in that pass2's common.h
# allows longer literals (M_LITLEN, which nothing here uses) and pass2's
# basic.h declares w_strncmp, so pass2's serve for this library too.

noinst_LIBRARIES = libhelputil.a
noinst_PROGRAMS = context

AM_CPPFLAGS = $(CDE_CPPFLAGS) -I$(srcdir)/../pass2/util

libhelputil_a_SOURCES = allwhite.c conutil.c error.c exit.c inctest.c \
	itoa.c letter.c lower.c malloc.c openchk.c trie.c triecnt.c	 \
	triedump.c triepdmp.c trieplk.c trierset.c upcmp8.c upper.c	 \
	upstrcmp.c wchar.c whitesp.c

context_SOURCES = context.c
context_LDADD = libhelputil.a
