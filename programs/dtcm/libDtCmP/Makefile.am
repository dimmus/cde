MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libDtCmP.a

libDtCmP_a_CFLAGS = $(CSA_INCDIR) -I../../../lib/csa -DRFC_MIME	\
	-DLINE_COUNT -DV2 -DOW_I18N

libDtCmP_a_SOURCES = cm_tty.c cmfns.c dtfns.c props.c resource.c	\
		     timeops.c util.c getdate.y

# This is strange - ususally you generate a .h and a .c.  But CDE
# comes with getdate.h, and you cannot generate it via yacc/lex.  So -
# someday this should be redone properly...  see
# https://opensource.apple.com/source/shell_cmds/shell_cmds-198/find/getdate.y.auto.html
# for what seems like a more recent version...
BUILT_SOURCES = getdate.c
CLEANFILES = $(BUILT_SOURCES)
