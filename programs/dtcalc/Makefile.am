MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtcalc

dtcalc_CPPFLAGS = -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\"

dtcalc_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

if SOLARIS
dtcalc_LDADD += -lintl
endif


dtcalc_SOURCES = calctool.c ds_common.c ds_popup.c ds_widget.c		\
		 ds_xlib.c functions.c help.c motif.c mp.c text.c text.h	\
		 calctool.h ds_popup.h ds_xlib.h patchlevel.h		\
		 ds_common.h ds_widget.h motif.h
