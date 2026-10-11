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
/* $XConsortium: task.h /main/2 1996/05/09 04:20:28 drk $ */
/*
 *   COMPONENT_NAME: austext
 *
 *   FUNCTIONS: Pi
 *		d_close
 *		d_cmstat
 *		d_cmtype
 *		d_connect
 *		d_costat
 *		d_cotype
 *		d_crget
 *		d_crread
 *		d_crset
 *		d_crstat
 *		d_crtype
 *		d_crwrite
 *		d_csmget
 *		d_csmread
 *		d_csmset
 *		d_csmwrite
 *		d_csoget
 *		d_csoread
 *		d_csoset
 *		d_csowrite
 *		d_csstat
 *		d_ctscm
 *		d_ctsco
 *		d_ctscr
 *		d_dbdpath
 *		d_dbfpath
 *		d_dblog
 *		d_dbuserid
 *		d_delete
 *		d_destroy
 *		d_discon
 *		d_disdel
 *		d_fillnew
 *		d_findco
 *		d_findfm
 *		d_findlm
 *		d_findnm
 *		d_findpm
 *		d_freeall
 *		d_gtscm
 *		d_gtsco
 *		d_gtscr
 *		d_gtscs
 *		d_initfile
 *		d_initialize
 *		d_ismember
 *		d_isowner
 *		d_keydel
 *		d_keyexist
 *		d_keyfind
 *		d_keyfree
 *		d_keyfrst
 *		d_keylast
 *		d_keylock
 *		d_keylstat
 *		d_keynext
 *		d_keyprev
 *		d_keyread
 *		d_keystore
 *		d_lock
 *		d_makenew
 *		d_members
 *		d_off_opt
 *		d_on_opt
 *		d_open
 *		d_rdcurr
 *		d_recfree
 *		d_recfrst
 *		d_reclast
 *		d_reclock
 *		d_reclstat
 *		d_recnext
 *		d_recover
 *		d_recprev
 *		d_recread
 *		d_recset
 *		d_recstat
 *		d_recwrite
 *		d_renfile
 *		d_rerdcurr
 *		d_retries
 *		d_rlbclr
 *		d_rlbset
 *		d_rlbtst
 *		d_set_dberr
 *		d_setdb
 *		d_setfree
 *		d_setkey
 *		d_setlock
 *		d_setlstat
 *		d_setmm
 *		d_setmo
 *		d_setmr
 *		d_setom
 *		d_setoo
 *		d_setor
 *		d_setrm
 *		d_setro
 *		d_stscm
 *		d_stsco
 *		d_stscr
 *		d_stscs
 *		d_timeout
 *		d_trabort
 *		d_trbegin
 *		d_trend
 *		d_utscm
 *		d_utsco
 *		d_utscr
 *		d_utscs
 *		d_wrcurr
 *		dt_closetask
 *		dt_cmstat
 *		dt_cmtype
 *		dt_connect
 *		dt_costat
 *		dt_cotype
 *		dt_crget
 *		dt_crread
 *		dt_crset
 *		dt_crtype
 *		dt_crwrite
 *		dt_csmget
 *		dt_csmread
 *		dt_csmset
 *		dt_csmwrite
 *		dt_csoget
 *		dt_csoread
 *		dt_csoset
 *		dt_csowrite
 *		dt_csstat
 *		dt_ctbpath
 *		dt_ctscm
 *		dt_ctsco
 *		dt_ctscr
 *		dt_dbdpath
 *		dt_dbfpath
 *		dt_dblog
 *		dt_dbuserid
 *		dt_destroy
 *		dt_discon
 *		dt_fillnew
 *		dt_findco
 *		dt_findfm
 *		dt_findlm
 *		dt_findnm
 *		dt_findpm
 *		dt_gtscm
 *		dt_gtsco
 *		dt_gtscr
 *		dt_gtscs
 *		dt_initfile
 *		dt_ismember
 *		dt_isowner
 *		dt_keydel
 *		dt_keyexist
 *		dt_keyfind
 *		dt_keyfree
 *		dt_keyfrst
 *		dt_keylast
 *		dt_keylock
 *		dt_keylstat
 *		dt_keynext
 *		dt_keyprev
 *		dt_keyread
 *		dt_keystore
 *		dt_lock
 *		dt_makenew
 *		dt_mapchar
 *		dt_members
 *		dt_off_opt
 *		dt_on_opt
 *		dt_open
 *		dt_opentask
 *		dt_rdcurr
 *		dt_recfree
 *		dt_recfrst
 *		dt_reclast
 *		dt_reclock
 *		dt_reclstat
 *		dt_recover
 *		dt_recread
 *		dt_recset
 *		dt_recstat
 *		dt_recwrite
 *		dt_renfile
 *		dt_rerdcurr
 *		dt_retries
 *		dt_set_dberr
 *		dt_setdb
 *		dt_setfree
 *		dt_setkey
 *		dt_setlock
 *		dt_setlstat
 *		dt_setmm
 *		dt_setmo
 *		dt_setmr
 *		dt_setom
 *		dt_setoo
 *		dt_setor
 *		dt_setrm
 *		dt_setro
 *		dt_stscm
 *		dt_stsco
 *		dt_stscr
 *		dt_stscs
 *		dt_timeout
 *		dt_trbegin
 *		dt_utscm
 *		dt_utsco
 *		dt_utscr
 *		dt_utscs
 *		dt_wrcurr
 *
 *   ORIGINS: 157
 *
 */
/*----------------------------------------------------------------------------
   task.h: db_VISTA task cover function declaration header file

   (C) Copyright 1988 by Raima Corporation.
----------------------------------------------------------------------------*/
/* ********************** EDIT HISTORY *******************************

 SCR    DATE    INI                   DESCRIPTION
----- --------- --- -----------------------------------------------------
  571 31-Jan-89 RSC Placed ctbpath inside NO_COUNTRY, not NO_TIMESTAMP
*/


int EXTERNAL_FIXED dt_opentask(DB_TASK *);
int EXTERNAL_FIXED dt_closetask(DB_TASK);

int EXTERNAL_FIXED dt_close(void);				/* dblfcns.c */
int EXTERNAL_DBN   dt_cmstat(int, ...);		/* cmstat.c */
int EXTERNAL_DBN   dt_cmtype(int, int *, ...); 
								/* cmtype.c */
int EXTERNAL_DBN   dt_connect(int, ...);		/* connect.c */
int EXTERNAL_DBN   dt_cotype(int, int *, ...); 
								/* cotype.c */
int EXTERNAL_DBN   dt_crget(DB_ADDR *, ...);	/* crget.c */
int EXTERNAL_DBN   dt_crread(long, char *, ...);
								/* crread.c */
int EXTERNAL_DBN   dt_crset(DB_ADDR *, ...);	/* crset.c */
int EXTERNAL_DBN   dt_crtype(int *, ...);	/* crtype.c */
int EXTERNAL_DBN   dt_crwrite(long, char *, ...); 
								/* crwrite.c */
int EXTERNAL_DBN   dt_csmget(int, DB_ADDR *, ...); 
								/* csmget.c */
int EXTERNAL_DBN   dt_csmread(int, long, char *, ...); 
								/* csmread.c */
int EXTERNAL_DBN   dt_csmset(int, DB_ADDR *, ...); 
								/* csmset.c */
int EXTERNAL_DBN   dt_csmwrite(int, long, const char *
				, ...); 			/* csmwrite.c */
int EXTERNAL_DBN   dt_csoget(int, DB_ADDR *, ...); 
								/* csoget.c */
int EXTERNAL_DBN   dt_csoread(int, long, char *, ...); 
								/* csoread.c */
int EXTERNAL_DBN   dt_csoset(int, DB_ADDR *, ...); 
								/* csoset.c */
int EXTERNAL_DBN   dt_csowrite(int, long, const char *
				, ...); 			/* csowrite.c */
int EXTERNAL_FIXED dt_ctbpath(const char *);	/* pathfcns.c */
int EXTERNAL_FIXED dt_dbdpath(const char *);	/* dbdpath.c */
int EXTERNAL_FIXED dt_dbfpath(const char *);	/* dbfpath.c */
int EXTERNAL_FIXED dt_dblog(const char *);	/* dblog.c */
int EXTERNAL_FIXED dt_dbuserid(const char *);	/* dbuserid.c */
int EXTERNAL_DBN   dt_delete(void, ...);			/* delete.c */
int EXTERNAL_DBN   dt_disdel(void, ...);			/* disdel.c */
int EXTERNAL_DBN   dt_recnext(void, ...);			/* recnext.c */
int EXTERNAL_DBN   dt_recprev(void, ...);			/* recprev.c */
int EXTERNAL_FIXED dt_destroy(const char *);	/* destroy.c */
int EXTERNAL_DBN   dt_discon(int, ...);		/* discon.c */
int EXTERNAL_DBN   dt_fillnew(int, const char *, ...); 
								/* fillnew.c */
int EXTERNAL_DBN   dt_findco(int, ...);		/* findco.c */
int EXTERNAL_DBN   dt_findfm(int, ...);		/* findfm.c */
int EXTERNAL_DBN   dt_findlm(int, ...);		/* findlm.c */
int EXTERNAL_DBN   dt_findnm(int, ...);		/* findnm.c */
int EXTERNAL_DBN   dt_findpm(int, ...);		/* findpm.c */
int EXTERNAL_FIXED dt_freeall(void);				/* dblfcns.c */
int EXTERNAL_DBN   dt_initialize(void, ...);		/* initial.c */
int EXTERNAL_DBN   dt_initfile(FILE_NO, ...);  	/* initial.c */
int EXTERNAL_DBN   dt_ismember(int, ...);		/* ismember.c */
int EXTERNAL_DBN   dt_isowner(int, ...);		/* isowner.c */
int EXTERNAL_DBN   dt_keydel(long, ...);		/* keydel.c */
int EXTERNAL_DBN   dt_keyexist(long, ...);	/* keyexist.c */
int EXTERNAL_DBN   dt_keyfind(long, const char *, ...); 
								/* keyfind.c */
int EXTERNAL_DBN   dt_keyfree(long, ...);		/* dblfcns.c */
int EXTERNAL_DBN   dt_keyfrst(long, ...);		/* keyfrst.c */
int EXTERNAL_DBN   dt_keylast(long, ...);		/* keylast.c */
int EXTERNAL_DBN   dt_keylock(long, char *, ...); 
								/* dblfcns.c */
int EXTERNAL_DBN   dt_keylstat(long, char *, ...); 
								/* dblfcns.c */
int EXTERNAL_DBN   dt_keynext(long, ...);		/* keynext.c */
int EXTERNAL_DBN   dt_keyprev(long, ...);		/* keyprev.c */
int EXTERNAL_FIXED dt_keyread(char *);		/* keyfcns.c */
int EXTERNAL_DBN   dt_keystore(long, ...);	/* keystore.c */
int EXTERNAL_DBN   dt_lock(int, LOCK_REQUEST *, ...); 
								/* dblfcns.c */
int EXTERNAL_DBN   dt_makenew(int, ...);		/* makenew.c */
int EXTERNAL_FIXED dt_mapchar(unsigned char, unsigned char 
		, const char *, unsigned char);
								/* mapchar.c */
int EXTERNAL_DBN   dt_members(int, LONG *, ...); 
								/* members.c */
int EXTERNAL_FIXED dt_off_opt(int);		/* options.c */
int EXTERNAL_FIXED dt_on_opt(int);			/* options.c */
int EXTERNAL_FIXED dt_open(const char *, const char *);
								/* dblfcns.c */
int EXTERNAL_FIXED dt_rdcurr(DB_ADDR **, int *); 
								/* rwcurr.c */
int EXTERNAL_FIXED dt_rerdcurr(DB_ADDR **); 	/* rwcurr.c */
int EXTERNAL_FIXED dt_wrcurr(DB_ADDR *);	/* rwcurr.c */
int EXTERNAL_DBN   dt_recfree(int, ...);		/* dblfcns.c */
int EXTERNAL_DBN   dt_recfrst(int, ...);		/* recfrst.c */
int EXTERNAL_DBN   dt_reclast(int, ...);		/* reclast.c */
int EXTERNAL_DBN   dt_reclock(int, char *, ...); 
								/* dblfcns.c */
int EXTERNAL_DBN   dt_reclstat(int, char *, ...); 
								/* dblfcns.c */
int EXTERNAL_FIXED dt_recover(const char *);	/* recover.c */
int EXTERNAL_DBN   dt_recread(char *, ...);	/* recread.c */
int EXTERNAL_DBN   dt_recset(int, ...);   	/* recset.c */
int EXTERNAL_DBN   dt_recwrite(const char *, ...); 
								/* recwrite.c */
int EXTERNAL_FIXED dt_renfile(const char *, FILE_NO 
				, const char *); /* renfile.c */
int EXTERNAL_FIXED dt_retries(int);		/* dblfcns.c */
int EXTERNAL_FIXED dt_rlbclr(void);				/* dblfcns.c */
int EXTERNAL_FIXED dt_rlbset(void);				/* dblfcns.c */
int EXTERNAL_FIXED dt_rlbtst(void);				/* dblfcns.c */
int EXTERNAL_FIXED dt_set_dberr(FARPROC);		/* dberr.c */
int EXTERNAL_FIXED dt_setdb(int);			/* setdb.c */
int EXTERNAL_DBN   dt_setfree(int, ...);		/* dblfcns.c */
int EXTERNAL_DBN   dt_setkey(long, const char *, ...); 
								/* makenew.c */
int EXTERNAL_DBN   dt_setlock(int, char *, ...); 
								/* dblfcns.c */
int EXTERNAL_DBN   dt_setlstat(int, char *, ...); 
								/* dblfcns.c */
int EXTERNAL_DBN   dt_setmm(int, int, ...); 	/* setmm.c */
int EXTERNAL_DBN   dt_setmo(int, int, ...); 	/* setmo.c */
int EXTERNAL_DBN   dt_setmr(int, ...);		/* setmr.c */
int EXTERNAL_DBN   dt_setom(int, int, ...); 	/* setom.c */
int EXTERNAL_DBN   dt_setoo(int, int, ...); 	/* setoo.c */
int EXTERNAL_DBN   dt_setor(int, ...);		/* setor.c */
int EXTERNAL_DBN   dt_setrm(int, ...);		/* setrm.c */
int EXTERNAL_DBN   dt_setro(int, ...);		/* setro.c */
int EXTERNAL_FIXED dt_timeout(int);		/* dblfcns.c */
int EXTERNAL_FIXED dt_trabort(void);				/* dblfcns.c */
int EXTERNAL_FIXED dt_trbegin(const char *);	/* dblfcns.c */
int EXTERNAL_FIXED dt_trend(void);				/* dblfcns.c */

#ifndef NO_DT_COVER

#define d_close()                    dt_close(CURRTASK_ONLY)
#define d_dbdpath(a)                 dt_dbdpath(a CURRTASK_PARM)
#define d_dbfpath(a)                 dt_dbfpath(a CURRTASK_PARM)
#define d_dblog(a)                   dt_dblog(a CURRTASK_PARM)
#define d_dbuserid(a)                dt_dbuserid(a CURRTASK_PARM)
#define d_destroy(a)                 dt_destroy(a CURRTASK_PARM)
#define d_freeall()                  dt_freeall(CURRTASK_ONLY)
#define d_off_opt(a)                 dt_off_opt(a CURRTASK_PARM)
#define d_on_opt(a)                  dt_on_opt(a CURRTASK_PARM)
#define d_open(a, b)                 dt_open(a, b CURRTASK_PARM)
#define d_rdcurr(a, b)               dt_rdcurr(a, b CURRTASK_PARM)
#define d_rerdcurr(a, b)             dt_rerdcurr(a, b CURRTASK_PARM)
#define d_recover(a)                 dt_recover(a CURRTASK_PARM)
#define d_renfile(a, b, c)           dt_renfile(a, b, c CURRTASK_PARM)
#define d_retries(a)                 dt_retries(a CURRTASK_PARM)
#define d_set_dberr(a, b)            dt_set_dberr(a, b, CURRTASK_PARM)
#define d_setdb(a)                   dt_setdb(a CURRTASK_PARM)
#define d_timeout(a)                 dt_timeout(a CURRTASK_PARM)
#define d_trabort()                  dt_trabort(CURRTASK_ONLY)
#define d_trbegin(a)                 dt_trbegin(a CURRTASK_PARM)
#define d_trend()                    dt_trend(CURRTASK_ONLY)
#define d_wrcurr(a)                  dt_wrcurr(a CURRTASK_PARM)

#define d_cmstat(a , dbn)         dt_cmstat(a CURRTASK_PARM , dbn)
#define d_cmtype(a, b , dbn)      dt_cmtype(a, b CURRTASK_PARM , dbn)
#define d_connect(a , dbn)        dt_connect(a CURRTASK_PARM , dbn)
#define d_cotype(a, b , dbn)      dt_cotype(a, b CURRTASK_PARM , dbn)
#define d_crget(a , dbn)          dt_crget(a CURRTASK_PARM , dbn)
#define d_crread(a, b , dbn)      dt_crread(a, b CURRTASK_PARM , dbn)
#define d_crset(a , dbn)          dt_crset(a CURRTASK_PARM , dbn)
#define d_crtype(a , dbn)         dt_crtype(a CURRTASK_PARM , dbn)
#define d_crwrite(a, b , dbn)     dt_crwrite(a, b CURRTASK_PARM , dbn)
#define d_csmget(a, b , dbn)      dt_csmget(a, b CURRTASK_PARM , dbn)
#define d_csmread(a, b, c , dbn)  dt_csmread(a, b, c CURRTASK_PARM , dbn)
#define d_csmset(a, b , dbn)      dt_csmset(a, b CURRTASK_PARM , dbn)
#define d_csmwrite(a, b, c , dbn) dt_csmwrite(a, b, c CURRTASK_PARM , dbn)
#define d_csoget(a, b , dbn)      dt_csoget(a, b CURRTASK_PARM , dbn)
#define d_csoread(a, b, c , dbn)  dt_csoread(a, b, c CURRTASK_PARM , dbn)
#define d_csoset(a, b , dbn)      dt_csoset(a, b CURRTASK_PARM , dbn)
#define d_csowrite(a, b, c , dbn) dt_csowrite(a, b, c CURRTASK_PARM , dbn)
#define d_delete(, dbn)           dt_delete(CURRTASK_ONLY , dbn)
#define d_disdel(, dbn)           dt_disdel(CURRTASK_ONLY , dbn)
#define d_recnext(, dbn)          dt_recnext(CURRTASK_ONLY , dbn)
#define d_recprev(, dbn)          dt_recprev(CURRTASK_ONLY , dbn)
#define d_discon(a , dbn)         dt_discon(a CURRTASK_PARM , dbn)
#define d_fillnew(a, b , dbn)     dt_fillnew(a, b CURRTASK_PARM , dbn)
#define d_findco(a , dbn)         dt_findco(a CURRTASK_PARM , dbn)
#define d_findfm(a , dbn)         dt_findfm(a CURRTASK_PARM , dbn)
#define d_findlm(a , dbn)         dt_findlm(a CURRTASK_PARM , dbn)
#define d_findnm(a , dbn)         dt_findnm(a CURRTASK_PARM , dbn)
#define d_findpm(a , dbn)         dt_findpm(a CURRTASK_PARM , dbn)
#define d_initialize(, dbn)       dt_initialize(CURRTASK_ONLY , dbn)
#define d_initfile(a , dbn)       dt_initfile(a CURRTASK_PARM , dbn)
#define d_ismember(a , dbn)       dt_ismember(a CURRTASK_PARM , dbn)
#define d_isowner(a , dbn)        dt_isowner(a CURRTASK_PARM , dbn)
#define d_keydel(a , dbn)         dt_keydel(a CURRTASK_PARM , dbn)
#define d_keyexist(a , dbn)       dt_keyexist(a CURRTASK_PARM , dbn)
#define d_keyfind(a, b , dbn)     dt_keyfind(a, b CURRTASK_PARM , dbn)
#define d_keyfree(a , dbn)        dt_keyfree(a CURRTASK_PARM , dbn)
#define d_keyfrst(a , dbn)        dt_keyfrst(a CURRTASK_PARM , dbn)
#define d_keylast(a , dbn)        dt_keylast(a CURRTASK_PARM , dbn)
#define d_keylock(a, b , dbn)     dt_keylock(a, b CURRTASK_PARM , dbn)
#define d_keylstat(a, b , dbn)    dt_keylstat(a, b CURRTASK_PARM , dbn)
#define d_keynext(a , dbn)        dt_keynext(a CURRTASK_PARM , dbn)
#define d_keyprev(a , dbn)        dt_keyprev(a CURRTASK_PARM , dbn)
#define d_keyread(a , dbn)        dt_keyread(a CURRTASK_PARM)
#define d_keystore(a , dbn)       dt_keystore(a CURRTASK_PARM , dbn)
#define d_lock(a, b , dbn)        dt_lock(a, b CURRTASK_PARM , dbn)
#define d_makenew(a , dbn)        dt_makenew(a CURRTASK_PARM , dbn)
#define d_members(a, b , dbn)     dt_members(a, b CURRTASK_PARM , dbn)
#define d_recfree(a , dbn)        dt_recfree(a CURRTASK_PARM , dbn)
#define d_recfrst(a , dbn)        dt_recfrst(a CURRTASK_PARM , dbn)
#define d_reclast(a , dbn)        dt_reclast(a CURRTASK_PARM , dbn)
#define d_reclock(a, b , dbn)     dt_reclock(a, b CURRTASK_PARM , dbn)
#define d_reclstat(a, b , dbn)    dt_reclstat(a, b CURRTASK_PARM , dbn)
#define d_recread(a , dbn)        dt_recread(a CURRTASK_PARM , dbn)
#define d_recset(a , dbn)         dt_recset(a CURRTASK_PARM , dbn)
#define d_recwrite(a , dbn)       dt_recwrite(a CURRTASK_PARM , dbn)
#define d_setfree(a , dbn)        dt_setfree(a CURRTASK_PARM , dbn)
#define d_setkey(a, b , dbn)      dt_setkey(a, b CURRTASK_PARM , dbn)
#define d_setlock(a, b , dbn)     dt_setlock(a, b CURRTASK_PARM , dbn)
#define d_setlstat(a, b , dbn)    dt_setlstat(a, b CURRTASK_PARM , dbn)
#define d_setmm(a, b , dbn)       dt_setmm(a, b CURRTASK_PARM , dbn)
#define d_setmo(a, b , dbn)       dt_setmo(a, b CURRTASK_PARM , dbn)
#define d_setmr(a , dbn)          dt_setmr(a CURRTASK_PARM , dbn)
#define d_setom(a, b , dbn)       dt_setom(a, b CURRTASK_PARM , dbn)
#define d_setoo(a, b , dbn)       dt_setoo(a, b CURRTASK_PARM , dbn)
#define d_setor(a , dbn)          dt_setor(a CURRTASK_PARM , dbn)
#define d_setrm(a , dbn)          dt_setrm(a CURRTASK_PARM , dbn)
#define d_setro(a , dbn)          dt_setro(a CURRTASK_PARM , dbn)

#endif /* NO_DT_COVER */
/* vpp -nOS2 -dUNIX -nBSD -nVANILLA_BSD -nVMS -nMEMLOCK -nWINDOWS -nFAR_ALLOC task.h */
