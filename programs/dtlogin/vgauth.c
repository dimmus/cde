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
/* $XConsortium: vgauth.c /main/4 1996/10/04 16:56:33 drk $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        vgauth.h
 **
 **   Project:     HP Visual User Environment (DT)
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

/* necessary for bzero */
#ifdef SVR4
#include        <X11/Xfuncs.h>
#ifdef  sun
#include        <shadow.h>
#endif
#endif

#include	"vg.h"
#include	"vgmsg.h"

/*
 * Define as generic those without platform specific code.
 */
#if !(defined(_AIX) || defined(sun))
#define generic
#endif

#ifdef sun
/***************************************************************************
 *
 *  Start authentication routines (SUN)
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  External declarations (SUN)
 *
 ***************************************************************************/




/***************************************************************************
 *
 *  Procedure declarations (SUN)
 *
 ***************************************************************************/

static void Audit( struct passwd *p, char *msg, int errnum) ;
static int  PasswordAged( struct passwd *pw) ;
static void WriteBtmp( char *name) ;




/***************************************************************************
 *
 *  Global variables (SUN)
 *
 ***************************************************************************/




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

    return;
}




/***************************************************************************
 *
 *  WriteBtmp (SUN)
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
 *  PasswordAged (SUN)
 *
 *  see if password has aged
 ***************************************************************************/
#define SECONDS_IN_WEEK		604800L

static int 
PasswordAged( struct passwd *pw )
{
    long change_week;	/* week password was changed (1/1/70 = Week 0) */
    long last_week;	/* week after which password must change */
    long first_week;	/* week before which password can't change */
    long this_week;	/* this week derived from time() */
    char *file;		/* help file name */
    char *command;	/* the /bin/passwd command string */

    if (*pw->pw_age == NULL)
	return(0);

    first_week = last_week = change_week = (long) a64l(pw->pw_age);
    last_week &= 0x3f;				/* first six bits */
    first_week = (first_week >> 6) & 0x3f;	/* next six bits */
    change_week >>= 12;				/* everything else */

    this_week = (long) time((long *) 0) / SECONDS_IN_WEEK;

/*
**	Password aging conditions:
**	*   if the last week is less than the first week (e.g., the aging
**	    field looks like "./"), only the superuser can change the
**	    password.  We don't request a new password.
**	*   if the week the password was last changed is after this week,
**	    we have a problem, and request a new password.
**	*   if this week is after the specified aging time, we request
**	    a new password.
*/
    if (last_week < first_week)
	return(0);

    if (change_week <= this_week && this_week <= (change_week + last_week))
	return(0);

    return(1);
}


    

/***************************************************************************
 *
 *  Verify (SUN)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/

#define MAXATTEMPTS	3

extern Widget focusWidget;		/* login or password text field	   */
struct  passwd nouser = {"", "nope"};	/* invalid user password struct	   */

int 
Verify( char *name, char *passwd )
{

    static int		login_attempts = 0; /* # failed authentications	   */
    
    struct passwd	*p;		/* password structure */
    struct spwd         *sp;            /* shadow info */
    char 		*crypt();

    int			n;

    p = getpwnam(name);
    sp = getspnam(name);
    
    if (!p || strlen(name) == 0 ||
        strcmp (crypt (passwd, sp->sp_pwdp), sp->sp_pwdp)) {

	if ( focusWidget == passwd_text ) {
	
	    WriteBtmp(name);

	    if ((++login_attempts % MAXATTEMPTS) == 0 ) {

		if (p->pw_name == NULL )
		    p = &nouser;

		Audit(p, " Failed login (bailout)", 1);

	    }
	}
	
	return(VF_INVALID);
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


    if ((p->pw_gid < 0)      || 
	(setgid(p->pw_gid) == -1)) {

	Audit(p, " attempted to login - bad group id", 1);
	return(VF_BAD_GID);
    }

    if ((p->pw_uid < 0)      || 
	(seteuid(p->pw_uid) == -1)) {

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
 *  End authentication routines (SUN)
 *
 ***************************************************************************/
#endif /* sun */

/***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/

#ifdef _AIX 
/***************************************************************************
 *
 *  Start authentication routines (AIX)
 *
 ***************************************************************************/

#include	<time.h>
#include	<sys/types.h>
#include	<usersec.h>
#include	<userpw.h>
#include	<userconf.h>

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

    return;
}




/***************************************************************************
 *
 *  WriteBtmp (AIX)
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
}

    

/***************************************************************************
 *
 *  Verify (AIX)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/

#define MAXATTEMPTS	3

extern Widget focusWidget;		/* login or password text field	   */
struct  passwd nouser = {"", "nope"};	/* invalid user password struct	   */

int 
Verify( char *name, char *passwd )
{

    static int		login_attempts = 0; /* # failed authentications	   */
    
    struct passwd	*p;		/* password structure */
    char 		*crypt();

    int			n;

    p = getpwnam(name);
    
    if (!p || strlen(name) == 0 ||
        strcmp (crypt (passwd, p->pw_passwd), p->pw_passwd)) {

	if ( focusWidget == passwd_text ) {
	
	    WriteBtmp(name);

	    if ((++login_attempts % MAXATTEMPTS) == 0 ) {

		if (p->pw_name == NULL )
		    p = &nouser;

		Audit(p, " Failed login (bailout)", 1);

	    }
	}
	
	return(VF_INVALID);
    }


    /*
     *  check password aging...
     */

     if ( PasswordAged(name,p) ) return(VF_PASSWD_AGED);

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


    if ((p->pw_gid < 0)      || 
	(setgid(p->pw_gid) == -1)) {

	Audit(p, " attempted to login - bad group id", 1);
	return(VF_BAD_GID);
    }

    if ((p->pw_uid < 0)) {
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
 *  End authentication routines (AIX)
 *
 ***************************************************************************/
#endif /* _AIX */


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

    seteuid(0);

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
 *  Verify (generic)
 *
 *  verify the user
 *
 *  return codes indicate authentication results.
 ***************************************************************************/

#define MAXATTEMPTS	3

extern Widget focusWidget;		/* login or password text field	   */
struct  passwd nouser = {"", "nope"};	/* invalid user password struct	   */

int 
Verify( char *name, char *passwd )
{

    static int		login_attempts = 0; /* # failed authentications	   */
    
    struct passwd	*p;		/* password structure */
    char 		*crypt();

    int			n;

    p = getpwnam(name);
    
    if (!p || strlen(name) == 0 ||
        strcmp (crypt (passwd, p->pw_passwd), p->pw_passwd)) {

	if ( focusWidget == passwd_text ) {
	
	    WriteBtmp(name);

	    if ((++login_attempts % MAXATTEMPTS) == 0 ) {

		if (p->pw_name == NULL )
		    p = &nouser;

		Audit(p, " Failed login (bailout)", 1);

	    }
	}
	
	return(VF_INVALID);
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


    if ((p->pw_gid < 0)      || 
	(setgid(p->pw_gid) == -1)) {

	Audit(p, " attempted to login - bad group id", 1);
	return(VF_BAD_GID);
    }

    if ((p->pw_uid < 0)      || 
	(seteuid(p->pw_uid) == -1)) {

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
