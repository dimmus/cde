MAINTAINERCLEANFILES = Makefile.in

if HAS_PAM_LIBRARY
CPP_TARGET = dtsession
dist_pam_DATA = $(CPP_TARGET)
if FREEBSD
pamdir = /usr/local/etc/pam.d
PAM_AUTHENTICATION_MODULE = /usr/local/lib/security/pam_pwauth_suid.so
AM_CPPFLAGS = -DPAM_AUTHENTICATION_MODULE=$(PAM_AUTHENTICATION_MODULE)
else
pamdir = /etc/pam.d
if NETBSD
PAM_AUTHENTICATION_MODULE = /usr/pkg/lib/security/pam_pwauth_suid.so
AM_CPPFLAGS = -DPAM_AUTHENTICATION_MODULE=$(PAM_AUTHENTICATION_MODULE)
endif
endif
endif

BUILT_SOURCES = $(CPP_TARGET)
CLEANFILES = $(CPP_TARGET)

dtsession: dtsession.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(AM_CPPFLAGS) $< > $@
