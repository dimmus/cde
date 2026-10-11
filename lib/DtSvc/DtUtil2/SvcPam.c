/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */

/*
 * Header Files
 */

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <security/pam_appl.h>

/*
 * libpam is loaded on first use instead of being linked into libDtSvc:
 * only dtlogin and dtsession authenticate, and every other client would
 * otherwise map libpam (and libaudit and the rest of its dependencies)
 * at startup for nothing.
 *
 * LIBPAM_SONAME can name the library at build time; otherwise the usual
 * sonames are tried (Linux-PAM, Solaris, FreeBSD/OpenPAM, unversioned).
 */
static const char *const libpam_names[] = {
#ifdef LIBPAM_SONAME
    LIBPAM_SONAME,
#endif
    "libpam.so.0",
    "libpam.so.1",
    "libpam.so.6",
    "libpam.so",
};

static struct {
    int (*start)(const char *, const char *, const struct pam_conv *,
		 pam_handle_t **);
    int (*end)(pam_handle_t *, int);
    int (*set_item)(pam_handle_t *, int, const void *);
    int (*authenticate)(pam_handle_t *, int);
    int (*open_session)(pam_handle_t *, int);
    int (*close_session)(pam_handle_t *, int);
    int (*setcred)(pam_handle_t *, int);
} pam;

static int pam_state;		/* 0 not tried, 1 loaded, -1 unavailable */

/* POSIX guarantees that a function pointer can hold dlsym()'s result. */
static int PamSym(void *handle, const char *name, void *field)
{
    void *sym = dlsym(handle, name);

    if (!sym)
	return 0;
    memcpy(field, &sym, sizeof(sym));
    return 1;
}

#define PAM_SYM(field, name) PamSym(handle, name, &pam.field)

static int LoadPam(void)
{
    void *handle = NULL;
    size_t i;

    if (pam_state)
	return pam_state > 0;

    pam_state = -1;

    for (i = 0; i < sizeof(libpam_names) / sizeof(libpam_names[0]); i++) {
	/* RTLD_GLOBAL: PAM modules expect to find libpam's symbols. */
	if ((handle = dlopen(libpam_names[i], RTLD_NOW | RTLD_GLOBAL)))
	    break;
    }

    if (!handle)
	return 0;

    if (PAM_SYM(start, "pam_start") &&
	PAM_SYM(end, "pam_end") &&
	PAM_SYM(set_item, "pam_set_item") &&
	PAM_SYM(authenticate, "pam_authenticate") &&
	PAM_SYM(open_session, "pam_open_session") &&
	PAM_SYM(close_session, "pam_close_session") &&
	PAM_SYM(setcred, "pam_setcred")) {
	pam_state = 1;
	return 1;
    }

    dlclose(handle);
    return 0;
}

/*
 * Local function declarations
 */

static int login_conv(int num_msg, const struct pam_message **msg,
        struct pam_response **response, void *appdata_ptr);

/*
 * Local structures and variables
 */

static struct pam_conv pam_conv = {login_conv, NULL};
static char *saved_user_passwd;
static pam_handle_t *pamh;

static int PamStart(const char *service_name, const char *user,
		    const char *display_name)
{
    int status = PAM_SUCCESS;
    char *colon, *hostname;

    if (!LoadPam())
	return PAM_SYSTEM_ERR;

    if (pamh) {
	if (service_name)
	    status = pam.set_item(pamh, PAM_SERVICE, service_name);

	if (status != PAM_SUCCESS && user) pam.set_item(pamh, PAM_USER, user);
    }
    else {
	status = pam.start(service_name, user, &pam_conv, &pamh);
    }

    if (status != PAM_SUCCESS) goto done;

    if (!display_name) goto done;

    if (display_name[0] == ':') {
	status = pam.set_item(pamh, PAM_TTY, display_name);
	goto done;
    }

    if (!(hostname = strdup(display_name))) {
	status = PAM_BUF_ERR;
	goto done;
    }

    if ((colon = strrchr(hostname, ':'))) *colon = '\0';

    status = pam.set_item(pamh, PAM_RHOST, hostname);

    free(hostname);

done:
    if (status != PAM_SUCCESS && pamh && pam.end(pamh, status) == PAM_SUCCESS)
	pamh = NULL;

    return status;
}

/**
 * @brief Authenticate that user / password combination is legal for this
 *        system.
 *
 * @param service_name
 * @param user
 * @param display_name
 * @param user_passwd
 *
 * @return See pam_authenticate.
 */
int _DtSvcPamAuthenticate(const char *service_name, const char *user,
			  const char *display_name, const char *user_passwd)
{
    int status;

    if (!user_passwd) return PAM_AUTH_ERR;

    if ((status = PamStart(service_name, user, display_name)) != PAM_SUCCESS)
	return status;

    saved_user_passwd = (char *) user_passwd;

    return pam.authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
}

/**
 * @brief Start PAM session management.
 *
 * @param service_name
 * @param user
 * @param display_name
 *
 * @return See pam_open_session.
 */
int _DtSvcPamOpenSession(const char *service_name, const char *user,
			 const char *display_name)
{
    int status;

    if ((status = PamStart(service_name, user, display_name)) != PAM_SUCCESS)
	return status;

    return pam.open_session(pamh, 0);
}

/**
 * @brief Terminate PAM session management.
 *
 * @param service_name
 * @param user
 * @param display_name
 *
 * @return See pam_close_session.
 */
int _DtSvcPamCloseSession(const char *service_name, const char *user,
			  const char *display_name)
{
    int status;

    if ((status = PamStart(service_name, user, display_name)) != PAM_SUCCESS)
	return status;

    return pam.close_session(pamh, 0);
}

/**
 * @brief Set Users login credentials.
 *
 * @param service_name
 * @param user
 * @param display_name
 *
 * @return See pam_setcred.
 */
int _DtSvcPamSetcred(const char *service_name, const char *user,
		     const char *display_name)
{
    int status;

    if ((status = PamStart(service_name, user, display_name)) != PAM_SUCCESS)
	return status;

    return pam.setcred(pamh, PAM_ESTABLISH_CRED);
}

/*****************************************************************************
 * login_conv():
 *
 * This is a conv (conversation) function called from the PAM
 * authentication scheme.  It returns the user's password when requested by
 * internal PAM authentication modules and also logs any internal PAM error
 * messages.
 *****************************************************************************/

static int login_conv(int num_msg, const struct pam_message **msg,
        struct pam_response **response, void *appdata_ptr)
{
    const struct pam_message	*m;
    struct pam_response	*r;
    int			k;

#ifdef lint
    conv_id = conv_id;
#endif
    if (num_msg <= 0)
        return (PAM_CONV_ERR);

    *response = (struct pam_response*)
        calloc(num_msg, sizeof (struct pam_response));
    if (*response == NULL)
        return (PAM_BUF_ERR);

    k = num_msg;
    m = *msg;
    r = *response;
    while (k--) {

        switch (m->msg_style) {

            case PAM_PROMPT_ECHO_OFF:
                if (saved_user_passwd != NULL) {
                    r->resp = (char *) malloc(strlen(saved_user_passwd)+1);
                    if (r->resp == NULL) {
                        /* __pam_free_resp(num_msg, *response); */
                        *response = NULL;
                        return (PAM_BUF_ERR);
                    }
                    (void) strcpy(r->resp, saved_user_passwd);
                    r->resp_retcode=0;
                }

                m++;
                r++;
                break;

            case PAM_ERROR_MSG:
                m++;
                r++;
                break;

            case PAM_TEXT_INFO:
                m++;
                r++;
                break;

            default:
                break;
        }
    }

    return (PAM_SUCCESS);
}
