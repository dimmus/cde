MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtpdm

dtpdm_CFLAGS = $(CDE_CFLAGS) -DI18N_MSG -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\"

dtpdm_LDADD = $(LIBPRINT) $(LIBHELP) $(LIBSVC) $(XMLIB) $(XTLIB) $(X11LIB)

if SOLARIS
dtpdm_LDADD += -lintl -lgen -lresolv
endif

dtpdm_SOURCES = JobBox.c Main.c MainWindow.c PdmMsgs.c PdmOid.c		\
		PdmXp.c PrinterBox.c JobBox.h PdmMsgs.h PdmOid.h	\
		PdmXp.h MainWindow.h PdmOidDefs.h PdmOidStrs.h		\
		PrinterBox.h
