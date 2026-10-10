
GENCPP_DEFINES = -DCDE_CONFIGURATION_TOP=$(CDE_CONFIGURATION_TOP) \
            -DCDE_INSTALLATION_TOP=$(CDE_INSTALLATION_TOP) \
            -DCDE_LOGFILES_TOP=$(CDE_LOGFILES_TOP) \
            -DFONTALIASESCONFDIR=$(sysconfdir)/$(PACKAGE_TARNAME)/fontaliases \
            -DFONTALIASESDATADIR=$(pkgdatadir)/fontaliases \
	    -DKORNSHELL=$(KSH)

CPP_TARGETS = 0060.dtsysenv Dtlogin Xaccess Xconfig Xconfig.con	\
	Xconfig.nc Xfailsafe Xreset Xresources Xresources.ive	\
	Xservers Xservers.con Xservers.nc Xsession Xsession.ive	\
	Xsetup Xstartup current.session display.current.session	\
	display.home.session dtlslocale dtprofile home.session	\
	sys.dtprofile



if HAS_PAM_LIBRARY
CPP_TARGETS += dtlogin
dist_pam_DATA = dtlogin
if FREEBSD
pamdir = /usr/local/etc/pam.d
PAM_AUTHENTICATION_MODULE = /usr/local/lib/security/pam_pwauth_suid.so
GENCPP_DEFINES += -DPAM_AUTHENTICATION_MODULE=$(PAM_AUTHENTICATION_MODULE)
else
pamdir = /etc/pam.d
if NETBSD
PAM_AUTHENTICATION_MODULE = /usr/pkg/lib/security/pam_pwauth_suid.so
GENCPP_DEFINES += -DPAM_AUTHENTICATION_MODULE=$(PAM_AUTHENTICATION_MODULE)
endif
endif
endif

if LINUX
GENCPP_DEFINES += -DXPROJECTROOT="/usr" -Dlinux -D__linux__
endif

if FREEBSD
GENCPP_DEFINES += -DFREEBSD -DXPROJECTROOT="/usr/local" -D__FreeBSD__
endif

if SOLARIS
GENCPP_DEFINES += -DXPROJECTROOT="/usr" -Dsun
endif

if OPENBSD
GENCPP_DEFINES += -DXPROJECTROOT="/usr/X11R6" -D__OpenBSD__
endif

if NETBSD
GENCPP_DEFINES += -DXPROJECTROOT="/usr/X11R7" -D__NetBSD__
endif

if BSD
GENCPP_DEFINES += -DCSRG_BASED
endif

GENCPP_DEFINES += -DSESSREG=$(SESSREG)

if SET_UTMP_FILE
GENCPP_DEFINES += -DUTMP_FLAG="-u\ $(UTMP_FILE)"
endif

if SET_WTMP_FILE
GENCPP_DEFINES += -DWTMP_FLAG="-w\ $(WTMP_FILE)"
endif

BUILT_SOURCES = $(CPP_TARGETS)
CLEANFILES = $(CPP_TARGETS)

# Some of these are installed here, others are installed via
# programs/localization/

configsdir=$(CDE_INSTALLATION_TOP)/config
dist_configs_DATA = Xaccess Xconfig Xfailsafe Xservers
dist_configs_SCRIPTS = sys.dtprofile Xfailsafe Xreset Xsetup Xstartup

xsessdir=$(CDE_INSTALLATION_TOP)/config/Xsession.d
dist_xsess_SCRIPTS = 0060.dtsysenv

dist_bin_SCRIPTS = Xsession

# fake - installed under a different name
sys.dtprofile: dtprofile
	$(RM) $@
	$(LN_S) $< $@

# regular targets
0060.dtsysenv: 0060.dtsysenv.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES)  $< > $@

Dtlogin: Dtlogin.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xaccess: Xaccess.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xconfig: Xconfig.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xconfig.con: Xconfig.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xconfig.nc: Xconfig.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) -DNO_CONSOLE $< > $@

Xfailsafe: Xfailsafe.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xreset: Xreset.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xresources: Xresources.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xresources.ive: Xresources.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) -DIVE $< > $@

Xservers: Xservers.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xservers.con: Xservers.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xservers.nc: Xservers.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) -DNO_CONSOLE $< > $@

Xsession: Xsession.src _common.ksh.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xsession.ive: Xsession.src _common.ksh.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) -DIVE $< > $@

Xsetup: Xsetup.src _common.ksh.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

Xstartup: Xstartup.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

current.session: current.session.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

display.current.session: display.current.session.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

display.home.session: display.home.session.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

dtlslocale: dtlslocale.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

dtprofile: dtprofile.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

home.session: home.session.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@

dtlogin: dtlogin.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@
