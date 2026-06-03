MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtappgather

dtappgather_CXXFLAGS = $(CDE_CXXFLAGS) -I../libCliSrv $(DT_INCDIR) \
	-DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
	-DCDE_DATA_TOP=\"$(CDE_DATA_TOP)\" \
	-DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
	-DCDE_LOGFILES_TOP=\"$(CDE_LOGFILES_TOP)\"

# It needs none of the desktop and X libraries (and is setuid root:
# the fewer libraries it loads the better).
dtappgather_LDADD = ../libCliSrv/libCliSrv.a

if SOLARIS
dtappgather_LDADD += -lintl
endif

dtappgather_SOURCES = Options.C dtappgather.C dtappgather.h Options.h

# can't use INSTALL here as the program is already installed
install-exec-hook:
	-chown root $(DESTDIR)$(bindir)/dtappgather
	-chmod 4755 $(DESTDIR)$(bindir)/dtappgather
