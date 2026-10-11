# Included by ../Makefile.am (%reldir% is util).
#
# The utility library and the "context" generator that the three passes
# of the help tag compiler (pass1, canon1 and pass2) share.  Each pass
# keeps its own headers in <pass>/util, together with the util sources
# that it #includes; those headers differ only in that pass2's common.h
# allows longer literals (M_LITLEN, which nothing here uses) and pass2's
# basic.h declares w_strncmp, so pass2's serve for this library too.

UTIL_CPPFLAGS = -I%reldir% -I$(srcdir)/%reldir% $(HELP_CPPFLAGS) \
	-I$(srcdir)/pass2/util

noinst_LIBRARIES += %reldir%/libhelputil.a
%canon_reldir%_libhelputil_a_CPPFLAGS = $(UTIL_CPPFLAGS)
%canon_reldir%_libhelputil_a_SOURCES = %reldir%/allwhite.c %reldir%/conutil.c \
	%reldir%/error.c %reldir%/exit.c %reldir%/inctest.c %reldir%/itoa.c \
	%reldir%/letter.c %reldir%/lower.c %reldir%/malloc.c %reldir%/openchk.c \
	%reldir%/trie.c %reldir%/triecnt.c %reldir%/triedump.c \
	%reldir%/triepdmp.c %reldir%/trieplk.c %reldir%/trierset.c \
	%reldir%/upcmp8.c %reldir%/upper.c %reldir%/upstrcmp.c %reldir%/wchar.c \
	%reldir%/whitesp.c

noinst_PROGRAMS += %reldir%/context
%canon_reldir%_context_CPPFLAGS = $(UTIL_CPPFLAGS)
%canon_reldir%_context_SOURCES = %reldir%/context.c
%canon_reldir%_context_LDADD = %reldir%/libhelputil.a
