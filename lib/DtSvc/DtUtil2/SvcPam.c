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
#include <security/pam_appl.h>

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
    int status;
    char *colon, *hostname;

    if (pamh) {
	if (service_name)
	    status = pam_set_item(pamh, PAM_SERVICE, service_name);

	if (status != PAM_SUCCESS && user) pam_set_item(pamh, PAM_USER, user);
    }
    else {
	status = pam_start(service_name, user, &pam_conv, &pamh);
    }

    if (status != PAM_SUCCESS) goto done;

    if (!display_name) goto done;

    if (display_name[0] == ':') {
	status = pam_set_item(pamh, PAM_TTY, display_name);
	goto done;
    }

    if (!(hostname = strdup(display_name))) {
	status = PAM_BUF_ERR;
	goto done;
    }

    if (colon = strrchr(hostname, ':')) *colon = '\0';

    status = pam_set_item(pamh, PAM_RHOST, hostname);

    free(hostname);

done:
    if (status != PAM_SUCCESS && pamh && pam_end(pamh, status) == PAM_SUCCESS)
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

    return pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
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

    return pam_open_session(pamh, 0);
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

    return pam_close_session(pamh, 0);
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

    return pam_setcred(pamh, PAM_ESTABLISH_CRED);
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
    char 			*temp;
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
