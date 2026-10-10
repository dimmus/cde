MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = dtsr infolib instant locales sgml spec tcl tpt doc_utils

include dtdocbook.am

bin_SCRIPTS = dtdocbook2man dtdocbook2sdl
bin_PROGRAMS = dtdocbook2infolib

dtdocbook2infolib_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

dtdocbook2infolib_CPPFLAGS = $(CDE_CPPFLAGS) \
			     -DDTDCBK_DATAROOTDIR=\"$(dtdocbookdatadir)\" \
			     -DINFOLIB_LIBEXECDIR=\"$(dtdocbooklibexecdir)/infolib\"
