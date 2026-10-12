MAINTAINERCLEANFILES = Makefile.in

# Delete a generated file whose recipe failed instead of keeping it
# half-written.
.DELETE_ON_ERROR:

if HAS_PAM_LIBRARY
CPP_TARGET = dtsession
dist_pam_DATA = $(CPP_TARGET)
pamdir = $(PAM_CONFDIR)
if FREEBSD
PAM_AUTHENTICATION_MODULE = /usr/local/lib/security/pam_pwauth_suid.so
GENCPP_DEFINES = -DPAM_AUTHENTICATION_MODULE=$(PAM_AUTHENTICATION_MODULE)
else
if NETBSD
PAM_AUTHENTICATION_MODULE = /usr/pkg/lib/security/pam_pwauth_suid.so
GENCPP_DEFINES = -DPAM_AUTHENTICATION_MODULE=$(PAM_AUTHENTICATION_MODULE)
endif
endif
endif

BUILT_SOURCES = $(CPP_TARGET)
CLEANFILES = $(CPP_TARGET)

dtsession: dtsession.src
	$(RM) $@
	$(GENCPP) -P -DXCOMM=# $(GENCPP_DEFINES) $< > $@
