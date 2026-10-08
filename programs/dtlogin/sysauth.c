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
/* $XConsortium: sysauth.c /main/7 1996/10/30 11:12:45 drk $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        sysauth.c
 **
 **   Project:     DT
 **
 **   Description: Dtgreet user authentication routines
 **
 **                These routines validate the user; checking name, password,
 **		   number of users on the system, password aging, etc.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **	Conditional compiles for HPUX:
 **
 **	AUDIT       HP C2 security enhancements; checks for existence of
 **                 SECUREPASSWD file and authenticates user against
 **                 password contained in that file. Also performs
 **                 self-auditing of login actions.  Incompatible with 
 **                 #ifdef SecureWare
 **
 **     __AFS        AFS 3 authentication mechanism
 **     __KERBEROS   Kerberos authentication mechanism
 **     __PASSWD_ETC Domain/OS Registry from HP-UX authentication mechanism
 **
 **     Platform identification:
 **
 **     sun         SUN OS only
 **     SVR4        SUN OS et al.
 **     _AIX        AIX only
 **     _POWER      AIX version 4 only
 **
 ****************************************************************************
 ************************************<+>*************************************/


/***************************************************************************
 *
 *  Includes & Defines
 *
 ***************************************************************************/

#include	<stdio.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<pwd.h>

#if defined(PAM) || defined(HAS_PAM_LIBRARY)
#include	<security/pam_appl.h>
#ifdef PAM
#include        "pam_svc.h"
#else
#include	<Dt/SvcPam.h>
#endif
#endif

#ifdef _AIX
#include	<usersec.h>
#include	<login.h>
#include	<sys/access.h>
#include 	<sys/sem.h>
#include 	<sys/stat.h>
#include	<sys/ipc.h>
#include	<sys/audit.h>
#endif

/* necessary for bzero */
#ifdef SVR4
#include        <X11/Xfuncs.h>
#if defined(sun)
#include        <shadow.h>
#endif
#endif

#include	"dm.h"
#include	"vg.h" 
#include	"vgmsg.h"
#include	"sysauth.h"

/*
 * Define as generic those without platform specific code.
 */
#if !(defined(_AIX) || defined(sun) || \
        defined(HAS_PAM_LIBRARY))
#define generic
#endif

#if defined(__linux__)
#    include <shadow.h>
#endif

#if defined(sun) || defined(HAS_PAM_LIBRARY)

/***************************************************************************
 *
 *  Start authentication routines (SUN)
 *
 ***************************************************************************/

#include "solaris.h"
#ifdef SUNAUTH
#include <security/ia_appl.h>
#endif


/***************************************************************************
 *
 *  Procedure declarations (SUN)
 *
 ***************************************************************************/

static void Audit( struct passwd *p, char *msg, int errnum) ;



/***************************************************************************
 *
 *  Audit (SUN)
 *
 ***************************************************************************/

static void 
Audit( struct passwd *p, char *msg, int errnum )
{

    /*
     * make sure program is back to super-user...
     */

    seteuid(0);

    Debug("Audit: %s\n", msg);

    return;
}


/***************************************************************************
 *
 *  Authenticate (SUN)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/

int 
Authenticate( struct display *d, char *name, char *passwd, char **msg )
{
   extern char *progName;

   int	status;
   char* ttyLine = d->gettyLine;

   /*
    * Nothing to do if no name provided.
    */
    if (!name) {
      return(VF_INVALID);
    }

   /*
    * Construct device line
    */
#ifdef DEF_NETWORK_DEV
            /*
             * If location is not local (remote XDMCP dtlogin) and
             * remote accouting is enabled (networkDev start with /dev/...)
             * Set tty line name to match network device for accouting.
             * Unless the resource was specifically set, default is value
             * of DEF_NETWORK_DEV define (/dev/dtremote)
             */

            if ( d->displayType.location != Local &&
                 networkDev && !strncmp(networkDev,"/dev/",5)) {
                ttyLine = networkDev+5;
            }
#endif

   /*
    * Authenticate user and return status
    */

#if defined(PAM) || defined(HAS_PAM_LIBRARY)
#ifdef PAM
    status = PamAuthenticate("dtlogin", d->name, passwd, name, ttyLine);
#else
    status = _DtSvcPamAuthenticate(progName, name, d->name, passwd);
#endif

    switch(status) {
        case PAM_SUCCESS:
            return(VF_OK);

        case PAM_NEW_AUTHTOK_REQD:
	    return(VF_PASSWD_AGED);

	default:
	    return(passwd ? VF_INVALID : VF_CHALLENGE);
    }
#else
    status = solaris_authenticate("dtlogin", d->name, passwd, name, ttyLine);

    switch(status) {
        case IA_SUCCESS:
            return(VF_OK);

        case IA_NEWTOK_REQD:
	    return(VF_PASSWD_AGED);

	default:
	    return(passwd ? VF_INVALID : VF_CHALLENGE);
    }
#endif /* !PAM */
}



/***************************************************************************
 *
 *  End authentication routines (SUN)
 *
 ***************************************************************************/
#endif /* sun */

/***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/
#ifdef _AIX 
/***************************************************************************
 *
 *  GetLoginInfo
 *
 *  get the information from the display about local/remote login and
 *  create a dummy tty name for loginrestrictions.
 *
 ***************************************************************************/
void
GetLoginInfo(struct display *d, int *loginType, char *ttyName, char **hostname)
{
  char workarea[128];

  CleanUpName(d->name, workarea, 128);
  sprintf(ttyName, "/dev/dtlogin/%s", workarea);

  if (d->displayType.location == Foreign) {
    *loginType = S_RLOGIN;
    *hostname = d->name;
    Debug("Login Info - Remote user on tty=%s.\n", ttyName);
  } else {
    *loginType = S_LOGIN;
    *hostname = NULL;
    Debug("Login Info - Local user on tty=%s.\n", ttyName);
  }
}
#ifdef _POWER
/***************************************************************************
 *
 *  Authenticate (AIX version 4.1)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/
Authenticate( struct display *d, char *name, char *passwd, char **msg )
{
  int arc;
  int rc;
  int reenter;
  static int unknown;
  static char *msgpending;
  int loginType;
  char tty[128];
  char *hostname;

  GetLoginInfo(d, &loginType, tty, &hostname);

  if (name == NULL)
  {
    unknown = 0;
  }

  if (unknown)
  {
 /*
    * No more challenges. User failed login.
    */
    unknown = 0;
    loginfailed(name, hostname, tty);
    return(VF_INVALID);
  }

 /*
  * Authenticate with response to last challenge.
  */
  rc = authenticate(name, passwd, &reenter, msg);

  if (reenter)
  {
   /*
    * System has presented user with new challenge.
    */
    return(VF_CHALLENGE);
  }

  if (rc && errno == ENOENT)
  { 
   /*
    * User is unknown to the system. Simulate a password
    * challenge, but save message for display for next call.
    */
    unknown = 1;
    return(VF_CHALLENGE);
  }

  if (rc)
  {
   /*
    * No more challenges. User failed login.
    */
    loginfailed(name, hostname, tty);
    return(VF_INVALID);
  }

 /*
  * User authenticated. Check login restrictions.
  */
  rc = loginrestrictions(name, loginType, tty, msg);

  if (rc)
  {
   /* 
    * Login restrictions disallow login.
    */
    loginfailed(name, hostname, tty);
    return(VF_MESSAGE);
  }

 /*
  * Check password expiration.
  */
  rc = passwdexpired(name, msg);

  if (rc)
  {
   /*
    * Login succeeded, but password expired.
    */
    return(VF_PASSWD_AGED);
  }

 /*
  * Login succeeded.
  */
  loginsuccess(name, hostname, tty, msg);
  return(VF_OK);
}



#else /* !_POWER */
/***************************************************************************
 *
 *  Start authentication routines (AIX)
 *
 ***************************************************************************/

#include	<time.h>
#include	<sys/types.h>
#include	<sys/errno.h>
#include	<usersec.h>
#include	<userpw.h>
#include	<userconf.h>
#include	<utmp.h>
#include	<time.h>

/***************************************************************************
 *
 *  External declarations (AIX)
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  Procedure declarations (AIX)
 *
 ***************************************************************************/

static void Audit( struct passwd *p, char *msg, int errnum) ;
static int  PasswordAged(char *name, struct passwd *pw) ;
static void WriteBtmp( char *name) ;




/***************************************************************************
 *
 *  Global variables (AIX)
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  Audit (AIX)
 *
 ***************************************************************************/

static void 
Audit( struct passwd *p, char *msg, int errnum )
{

    /*
     * make sure program is back to super-user...
     */

    seteuid(0);
    if ( (auditwrite ("USER_Login", AUDIT_OK,
            p->pw_name, strlen (p->pw_name) + 1,
            msg, strlen (msg) + 1, NULL))  == -1 )
          Debug(" Could not do Auditing\n");

}




/***************************************************************************
 *
 *  WriteBtmp (AIX)
 *
 *  log bad login attempts to /etc/security/failedlogin file
 *  
 *  RK	09.13.93
 ***************************************************************************/

static void 
WriteBtmp( char *name )
{
	int	fd;
	struct  utmp	ut;

	if( (fd = open("/etc/security/failedlogin",O_CREAT|O_RDWR,0644)) != -1) {
		bzero(&ut,sizeof(struct utmp));
		if(name)
			strncpy(ut.ut_user, name, sizeof ut.ut_user);
		ut.ut_type = USER_PROCESS;
		ut.ut_pid = getpid();
		ut.ut_time = time((time_t *)0);
		write(fd, (char *)&ut, sizeof(struct utmp));
		close(fd);
	}
}




/***************************************************************************
 *
 *  PasswordAged (AIX)
 *
 *  see if password has aged
 ***************************************************************************/
#define SECONDS_IN_WEEK		604800L

static int 
PasswordAged(char *name, struct passwd *pw )
{
  struct userpw *pupw; /* authentication information from getuserpw() */
  struct userpw  upw;  /* working authentication information */
  int err;             /* return code from getconfattr() */
  ulong maxage;        /* maximun age from getconfattr() */
  ulong now;           /* time now */

#ifdef	_POWER
  return(FALSE);
#else	/* _POWER */
 /*
  * Determine user password aging criteria. Note that only
  * the 'lastupdate' and 'flags' fields are set by this operation.
  */
  setpwdb(S_READ);
  if ((pupw = getuserpw(name)) != NULL)
  {
    upw.upw_lastupdate = pupw->upw_lastupdate;
    upw.upw_flags = pupw->upw_flags;
  }
  else
  {
    upw.upw_lastupdate = 0;
    upw.upw_flags = 0;
  }
  endpwdb();

 /*
  * Consider password as having not expired if nocheck set.
  */
  if (upw.upw_flags & PW_NOCHECK) return(FALSE);

 /*
  * Get system password aging criteria.
  */
  err = getconfattr (SC_SYS_PASSWD, SC_MAXAGE, (void *)&maxage, SEC_INT);
  if (!err && maxage)
  {
   /*
    * Change from weeks to seconds
    */
    maxage = maxage * SECONDS_IN_WEEK;
    now = time ((long *) 0);

    if ((upw.upw_lastupdate + maxage) >= now)
    {
     /*
      * Password has not expired.
      */
      return(FALSE);
    }
  }
  else
  {
   /*
    * Could not retrieve system password aging info or maxage set to
    * zero. In either case, consider password has having not expired.
    */
    return(FALSE);
  }

 /* 
  * We haven't returned by now, so indicate password has expired.
  */
  return(TRUE);
#endif	/* _POWER */
}

/***************************************************************************
 *  dt_failedlogin (AIX)
 *
 * log failed login in /etc/security/lastlog
 ***************************************************************************/
struct  lastlogin {
        time_t  ftime;
        time_t  stime;
        int     fcount;
        char    user[32];
        char    *stty;
        char    *ftty;
        char    *shost;
        char    *fhost;
};
extern void 
	dt_lastlogin ( char * user, struct lastlogin * llogin);

void 
dt_failedlogin(char *name, char *ttyName, char *hostName)
{
	struct lastlogin last_login;

	last_login.stime = 0;

        time(&last_login.ftime);

        last_login.ftty = ttyName;

        last_login.fhost = (char *) malloc (MAXHOSTNAMELEN);
        if (hostName == NULL) {
            gethostname (last_login.fhost , MAXHOSTNAMELEN);
        } else {
            strncpy(last_login.fhost, hostName, MAXHOSTNAMELEN);
            last_login.fhost[MAXHOSTNAMELEN -1] = '\0';
        }

        Debug("logging failed lastlogin entry (user=%s)\n",name);
        dt_lastlogin(name, &last_login);
        free(last_login.fhost);
}

    

/***************************************************************************
 *
 *  Authenticate (AIX)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/

#define MAXATTEMPTS	3

struct  passwd nouser = {"", "nope"};	/* invalid user password struct	   */

int 
Authenticate( struct display *d, char *name, char *passwd, char **msg )
{

    static int		login_attempts = 0; /* # failed authentications	   */
    struct passwd	*p;		/* password structure */
    char 		*crypt();
    char               *origpw;
    int loginType;
    char tty[128];
    char *hostname;

   /*
    * Nothing to do if no name provided.
    */
    if (!name)
      return(VF_INVALID);

   /*
    * Save provided password.
    */
    origpw = passwd;
    if (!passwd) passwd = "";

    if(strlen(name) > S_NAMELEN)
         return(VF_INVALID);

    GetLoginInfo(d, &loginType, tty, &hostname);

    p = getpwnam(name);
     
    if (!p  || strcmp (crypt (passwd, p->pw_passwd), p->pw_passwd)) {

	WriteBtmp(name);  

	if ((++login_attempts % MAXATTEMPTS) == 0 ) {

	    if (p == NULL )
		p = &nouser;

	    Audit(p, " Failed login (bailout)", 1);
	}

	if (origpw) {
	    dt_failedlogin(name, tty, hostname);
	    return (VF_INVALID);
	} else
	    return(VF_CHALLENGE);
    }

    /* Note: The password should be checked if it is the first time
             the user is logging in or whether the sysadm has changed
             the password for the user. Code should be added here if
             this functionality should be supported. The "upw_flags"
             of the password structure gets set to PW_ADMCHG in this
             case.				RK 09.13.93.
    */

    /*
     *  check password aging...
     */

    if ( PasswordAged(name,p) ) return(VF_PASSWD_AGED);


    /* Validate for User Account  RK 09.13.93 */
    if(ckuseracct(name, loginType, tty) == -1)  {
	dt_failedlogin(name, tty, hostname);
	return(VF_INVALID);
    }


    /*
     *  validate uid and gid...
     */


    if ((p->pw_gid < 0)      || 
 	(setgid(p->pw_gid) == -1)) {

	Audit(p, " attempted to login - bad group id", 1);
	return(VF_BAD_GID);
    }

    if ((p->pw_uid < 0)) {
	Audit(p, " attempted to login - bad user id", 1);
	return(VF_BAD_UID);
    }

    /* Check for max number of logins  RK 09.13.93 */
    if (tsm_check_login(p->pw_uid) == -1) {
	dt_failedlogin(name, tty, hostname);
        return(VF_INVALID);
    }

    /* Check for /etc/nologin file   RK 09.13.93 */
    if ( (access("/etc/nologin",R_OK) == 0) && (p->pw_uid != 0) ) {
	dt_failedlogin(name, tty, hostname);
	return(VF_INVALID);
    }

    /*
     *  verify home directory exists...
     */

    if(chdir(p->pw_dir) < 0) {
	Audit(p, " attempted to login - no home directory", 1);
        return(VF_HOME);
    }

    /*
     * verify ok...
     */

    Audit(p, " Successful login", 0);
    return(VF_OK);
}

/**************************************************************************
* 
* tsm_check_login() 
*
* Checks for max number of logins on the system. If the new user trying to
* login exceeds the max limit then the user is not allowed to login.
*
* RK 09.13.93
**************************************************************************/

/**************************************************************************
* 
* tsm_check_login() 
*
* Checks for max number of logins on the system. If the new user trying to
* login exceeds the max limit then the user is not allowed to login.
*
* RK 09.13.93
**************************************************************************/

int
tsm_check_login(uid_t uid)
{

	key_t	key;
	char    *buffer;
	int	semid;
	int 	fd;
	struct  stat stat_buf;
	static	struct	sembuf	sop = { 0, -1, (SEM_UNDO|IPC_NOWAIT) };
	static	struct	sembuf	initsop = { 0, 0, (IPC_NOWAIT) };

	/*
	 * The login counter semaphore may not be set yet.  See if it exists
	 * and try creating it with the correct count if it doesn't.  An
	 * attempt is made to create the semaphore.  Only if that attempt fails
	 * is the semaphore set to maxlogins from login.cfg.
	 */


	/*
	 * Don't Check if the user is already logged. ie running login
	 * from a shell
	 */
		/*
		 * Generate the semaphore key from the init program.
		 */
		Debug("Start of maxlogin check\n");
		if ((key = ftok (CDE_INSTALLATION_TOP "/bin/dtlogin", 1)) != (key_t) -1) {
			Debug("key created\n");
			if ((semid = semget (key, 1, IPC_CREAT|IPC_EXCL|0600)) != -1) {
				int	i;
				Debug("Completed IPCkey\n");
				if (! getconfattr ("usw", "maxlogins", &i, SEC_INT)) {
                                        Debug("Max logins from login.cfg is :%d\n",i);
					if (i <= 0)
						i = 10000; /* a very large number */

					initsop.sem_op = i;
					if (semop (semid, &initsop, 1))
						{
                                                    Debug("failed while decrementing\n");
						return(-1);	
						}
				} else {
					semctl (semid, 1, IPC_RMID, 0);
				}
			}

			/*
			 * Only 'n' login sessions are allowed on the system. 
			 * This code block decrements a semaphore.
			 * The semundo value will be set to adjust the
			 * semaphore when tsm exits.
			 *
			 * This code will be ignored if the appropriate
			 * semaphore set does not exist.
			 */

			if ((semid = semget (key, 1, 0)) != -1) {
                                  Debug("getting key for maxlogins\n");
				/*
				 * If the semaphore is zero and we are not
				 * root, then we fail as there are already the
				 * allotted number of login sessions on the
				 * system.
				 */
				if ((semop (semid, &sop, 1)  == -1) && uid) {
                                                    Debug("reached MAXLOGINS limit\n");
					errno = EAGAIN;
					return(-1);
				}
			}
		}

}
#endif /* !_POWER */
#endif /* _AIX */
/***************************************************************************
 *
 *  End authentication routines (AIX)
 *
 ***************************************************************************/

/***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/


#ifdef generic
/***************************************************************************
 *
 *  Start authentication routines (generic)
 *
 ***************************************************************************/


/***************************************************************************
 *
 *  These are a set of routine to do simple password, home dir, uid, and gid
 *  validation. They can be used as a first pass validation for future
 *  porting efforts.
 *
 *  When platform specific validation is developed, those routines should be
 *  included in their own section and the use of these routines discontinued.
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  External declarations (generic)
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  Procedure declarations (generic)
 *
 ***************************************************************************/

static void Audit( struct passwd *p, char *msg, int errnum) ;
static int  PasswordAged( struct passwd *pw) ;
static void WriteBtmp( char *name) ;




/***************************************************************************
 *
 *  Global variables (generic)
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  Audit (generic)
 *
 ***************************************************************************/

static void 
Audit( struct passwd *p, char *msg, int errnum )
{

    /*
     * make sure program is back to super-user...
     */
    if(-1 == seteuid(0)) {
        perror(strerror(errno));
    }

    return;
}




/***************************************************************************
 *
 *  WriteBtmp (generic)
 *
 *  log bad login attempts
 *  
 ***************************************************************************/

static void 
WriteBtmp( char *name )
{
    return;
}




/***************************************************************************
 *
 *  PasswordAged (Generic)
 *
 *  see if password has aged
 ***************************************************************************/
#define SECONDS_IN_WEEK		604800L

static int 
PasswordAged( struct passwd *pw )
{
  return(FALSE);
}

    

/***************************************************************************
 *
 *  Authenticate (generic)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/

#define MAXATTEMPTS	3

struct  passwd nouser = {"", "nope"};	/* invalid user password struct	   */

int 
Authenticate( struct display *d, char *name, char *passwd, char **msg )
{

    static int		login_attempts = 0; /* # failed authentications	   */
    
    struct passwd	*p;		/* password structure */
    char 		*crypt();

    int			n;

    char               *origpw;

   /*
    * Nothing to do if no name provided.
    */
    if (!name)
      return(VF_INVALID);

   /*
    * Save provided password.
    */
    origpw = passwd;
    if (!passwd) passwd = "";

    p = getpwnam(name);
    
#if defined(__linux__)
    /*
     * Use the Linux Shadow Password system to get the crypt()ed password
     */
    if(p) {
        struct spwd *s = getspnam(name);
	if(s) {
            p->pw_passwd = s->sp_pwdp;
        }
    }
#endif

#if defined(__OpenBSD__)
    /*
     * Use the OpenBSD getpwnam_shadow function to get the crypt()ed password
     */
     p = getpwnam_shadow(name);
#endif

    if (!p || strlen(name) == 0 ||
        strcmp (crypt (passwd, p->pw_passwd), p->pw_passwd)) {

	    WriteBtmp(name);

	    if ((++login_attempts % MAXATTEMPTS) == 0 ) {

		if (p == NULL )
		    p = &nouser;

		Audit(p, " Failed login (bailout)", 1);

	    }
	
	return(origpw ? VF_INVALID : VF_CHALLENGE);
    }


    /*
     *  check password aging...
     */

     if ( PasswordAged(p) ) return(VF_PASSWD_AGED);
         

    /*
     *  verify home directory exists...
     */

    if(chdir(p->pw_dir) < 0) {
	Audit(p, " attempted to login - no home directory", 1);
        return(VF_HOME);
    }


    /*
     *  validate uid and gid...
     */


    if (setgid(p->pw_gid) == -1) {

	Audit(p, " attempted to login - bad group id", 1);
	return(VF_BAD_GID);
    }

    if (seteuid(p->pw_uid) == -1) {

	Audit(p, " attempted to login - bad user id", 1);
	return(VF_BAD_UID);
    }



    /*
     * verify ok...
     */

    Audit(p, " Successful login", 0);
    return(VF_OK);
}




/***************************************************************************
 *
 *  End authentication routines (generic)
 *
 ***************************************************************************/
#endif /* generic */



/***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/
