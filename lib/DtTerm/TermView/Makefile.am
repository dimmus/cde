MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libTermView.la

libTermView_la_CFLAGS = $(CDE_CFLAGS) -I$(srcdir)/../TermPrim -I$(srcdir)/../Term @DT_INCDIR@

libTermView_la_SOURCES = TermView.c \
			 TermViewMenu.c \
			 TermViewGlobalDialog.c \
			 TermViewTerminalDialog.c
