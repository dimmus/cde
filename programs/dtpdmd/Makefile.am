MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/build-aux/cde-libs.am

bin_PROGRAMS = dtpdmd

dtpdmd_CFLAGS = $(CDE_CFLAGS) -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\"

dtpdmd_LDADD = $(LIBSVC) $(XTLIB) $(XAULIB) $(X11LIB)

dtpdmd_SOURCES = dispatch.c dtpdmd.c mailbox.c manager.c records.c	\
		 nlmsg.c setup.c util.c dtpdmdP.h nlmsg.h
