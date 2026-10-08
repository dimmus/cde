SUBDIRS = config

bin_PROGRAMS = dtlogin dtgreet dtchooser

XDMDIR = $(CDE_LOGFILES_TOP)

DEF_SERVER = $(XBINDIR)/X
DEF_USER_PATH = $(PATH):$(XBINDIR)
DEF_SYSTEM_PATH = $(PATH):$(XBINDIR)

DEF_BM_PATH = $(CDE_CONFIGURATION_TOP)/appconfig/icons/%L/%B%M.bm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/%L/%B%M.pm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/%L/%B:$(CDE_CONFIGURATION_TOP)/appconfig/icons/C/%B%M.bm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/C/%B%M.pm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/C/%B:$(CDE_INSTALLATION_TOP)/appconfig/icons/%L/%B%M.bm:$(CDE_INSTALLATION_TOP)/appconfig/icons/%L/%B%M.pm:$(CDE_INSTALLATION_TOP)/appconfig/icons/%L/%B:$(CDE_INSTALLATION_TOP)/appconfig/icons/C/%B%M.bm:$(CDE_INSTALLATION_TOP)/appconfig/icons/C/%B%M.pm:$(CDE_INSTALLATION_TOP)/appconfig/icons/C/%B
DEF_PM_PATH = $(CDE_CONFIGURATION_TOP)/appconfig/icons/%L/%B%M.pm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/%L/%B%M.bm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/%L/%B:$(CDE_CONFIGURATION_TOP)/appconfig/icons/C/%B%M.pm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/C/%B%M.bm:$(CDE_CONFIGURATION_TOP)/appconfig/icons/C/%B:$(CDE_INSTALLATION_TOP)/appconfig/icons/%L/%B%M.pm:$(CDE_INSTALLATION_TOP)/appconfig/icons/%L/%B%M.bm:$(CDE_INSTALLATION_TOP)/appconfig/icons/%L/%B:$(CDE_INSTALLATION_TOP)/appconfig/icons/C/%B%M.pm:$(CDE_INSTALLATION_TOP)/appconfig/icons/C/%B%M.bm:$(CDE_INSTALLATION_TOP)/appconfig/icons/C/%B

AM_CFLAGS = -DCDE_CONFIGURATION_TOP=\"$(CDE_CONFIGURATION_TOP)\" \
            -DCDE_INSTALLATION_TOP=\"$(CDE_INSTALLATION_TOP)\" \
            -DCDE_LOGFILES_TOP=\"$(CDE_LOGFILES_TOP)\" \
	    -DBINDIR=\"$(XBINDIR)\" \
	    -DXDMDIR=\"$(XDMDIR)\" \
	    -DKORNSHELL=$(KSH) -DUNIXCONN -DTCPCONN -DXDMCP

dtlogin_SOURCES = access.c auth.c genauth.c daemon.c dm.c \
                  dpylist.c error.c file.c mitauth.c protodpy.c policy.c \
                  reset.c server.c session.c socket.c util.c \
                  verify.c sysauth.c fontpath.c qualify.c choose.c netaddr.c \
		  xdmcp.c

# We play a game here with resources.o (dtlogin).  It needs special options.
noinst_LIBRARIES = libresource.a
libresource_a_SOURCES = resource.c
libresource_a_CFLAGS = $(AM_CFLAGS) \
		 -DDEF_SESSION=\"$(CDE_INSTALLATION_TOP)/bin/Xsession\" \
		 -DDEF_SYSTEM_SHELL=\"/bin/sh\" \
                 -DDEF_CHOOSER=\"$(CDE_INSTALLATION_TOP)/bin/dtchooser\" \
                 -DDEF_XDM_CONFIG=\"Xconfig\" \
		 -DDEF_SERVER_LINE=\"":0 Local local $(DEF_SERVER) :0"\" \
                 -DXRDB_PROGRAM=\"$(XRDB)\" \
                 -DDEF_USER_PATH=\"$(DEF_USER_PATH)\" \
                 -DDEF_SYSTEM_PATH=\"$(DEF_SYSTEM_PATH)\" \
                 -DDEF_FAILSAFE_CLIENT=\"$(XBINDIR)/xterm\" \
                 -DDEF_AUTH_FILE=\"$(XDMDIR)/auth-server\" \
                 -DDEF_AUTH_DIR=\"$(XDMDIR)\" \
                 -DCPP_PROGRAM=\""$(CPP_PROGRAM)"\" \
                 -DDEF_PM_SEARCH_PATH=\"$(DEF_PM_PATH)\" \
                 -DDEF_BM_SEARCH_PATH=\"$(DEF_BM_PATH)\"

dtgreet_SOURCES = vgcallback.c vglogo.c vgmain.c vgutil.c vglang.c

dtchooser_SOURCES = chooser.c dtchooser.c vglogo.c vgutil.c vgcallback.c \
		    vglang.c

dtlogin_LDADD = libresource.a $(DTCLIENTLIBS) $(XDMCPLIB) \
	$(XTOOLLIB) $(TIRPCLIB)

dtgreet_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

dtchooser_LDADD = $(DTCLIENTLIBS) $(XDMCPLIB) $(XTOOLLIB)


if LINUX
AM_CFLAGS += -DHASDES -DHASXDMAUTH
XBINDIR = /usr/bin
dtlogin_SOURCES += xdmauth.c
endif

if FREEBSD
AM_CFLAGS += -DHASDES -DHASXDMAUTH
XBINDIR = /usr/local/bin
dtlogin_SOURCES += xdmauth.c
endif

if OPENBSD
XBINDIR = /usr/X11R6/bin
AM_CFLAGS += -DHASDES -DHASXDMAUTH
dtlogin_SOURCES += xdmauth.c
endif

if NETBSD
XBINDIR = /usr/X11R7/bin
endif

if SOLARIS
XBINDIR = /usr/bin/X11
dtlogin_LDADD += -lgen -lresolv -lbsm -lcmd
dtgreet_LDADD += -lgen -lresolv -lbsm -lcmd
dtchooser_LDADD += -lgen -lresolv -lbsm -lcmd
dtlogin_SOURCES += solaris.c
endif

