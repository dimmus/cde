MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libutil.a

noinst_PROGRAMS = context fclndir

context_SOURCES = context.c

fclndir_SOURCES = context.c

libutil_a_SOURCES = allwhite.c inctest.c letter.c triedump.c		\
	triepdmp.c trieplk.c trierset.c upcmp8.c upstrcmp.c wchar.c	\
	conutil.c error.c exit.c itoa.c lower.c malloc.c openchk.c	\
	trie.c triecnt.c upper.c whitesp.c basic.h common.h		\
	dtddef.h entdef.h sparse.h trie.h chartype.h cont.h dtdext.h	\
	entext.h sref.h version.h

# FIXME I think emptyfil.c and fclndir.c can be deleted?

context_LDADD = libutil.a
fclndir_LDADD = libutil.a
