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
//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $TOG: mp_typedb.C /main/5 1998/03/20 14:29:07 mgreess $ 			 				
/*
 * @(#)mp_typedb.C	1.54 93/07/29 SMI
 *
 * mp_typedb.cc - _Tt_typedb represents the database of ToolTalk types
 *
 * Copyright (c) 1990,1992 by Sun Microsystems, Inc.
 */

//
// Contains methods for reading and writing type databases which can be
// stored either as Classing Engine databases or a native xdr format
// databases. 
//
#include <stdlib.h>
#if defined(__linux__) || defined(CSRG_BASED)
/*# include <g++/minmax.h>*/
#else
# include <macros.h>
#endif
#include <fcntl.h>
#include "tt_options.h"
#include "mp/mp_arg.h"
#include "mp/mp.h"
#include "mp_otype.h"
#include "mp_ptype.h"
#include "mp_typedb.h"
#include "api/c/api_api.h"
#include "Tt/tttk.h"
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include "mp_ce_attrs.h"
#include <stdlib.h>
#include <unistd.h>
#include "util/tt_enumname.h"
#include "util/tt_port.h"
#include "util/tt_gettext.h"
#include "util/tt_global_env.h"
#include "util/tt_xdr_version.h"


enum _Tt_typedb_flags {
	_TT_TYPEDB_USER,
	_TT_TYPEDB_SYSTEM,
	_TT_TYPEDB_NETWORK,
	_TT_TYPEDB_XDR_MODE,
	_TT_TYPEDB_LOCKED
};



_Tt_typedb::
_Tt_typedb(char * /* ce_dir */)
{
	_flags = 0;
	ptable = new _Tt_ptype_table(_tt_ptype_ptid);
	otable = new _Tt_otype_table(_tt_otype_otid);
	ceDB2Use = TypedbAll;
}


_Tt_typedb::
~_Tt_typedb()
{
}


//
// XDR's a _Tt_typedb object. 
bool_t _Tt_typedb::
xdr(XDR *xdrs)
{
	if (!ptable.xdr(xdrs)) {
		return  0;
	}

	if (!otable.xdr(xdrs)) {
		return  0;
	}

	if  (xdrs->x_op == XDR_DECODE) {

		// We have to be sure that a null ptable or otable are
		// NEVER passed into the _Tt_object_table::xdr. XXX.

		if (ptable.is_null()) {
			ptable = new _Tt_ptype_table(_tt_ptype_ptid);
		}
		if (otable.is_null()) {
			otable = new _Tt_otype_table(_tt_otype_otid);
		}
	}

	return 1;
}



// 
// This function returns the full pathnames to the user database, system
// database, and network databases in udb, sdb, and ndb respectively.
// This three-level model of databases is intended to be similar to the
// Classing Engine model. The intent is that types in the user database
// shadow types in the system and network databases and that the types in
// the system database shadow the types in the network database. In order
// to provide some user-configurability, we provide an
// environment variable TTPATH that is a three-path list separated by ":"
// pointing to the user,system, and network databases. This function
// looks at that variable and returns the paths.
// 
//  If TTPATH isn't set this function should returns
//  $HOME/.tt/types.xdr for the user database, /etc/tt/types.xdr for the
//  system database, and $OPENWINHOME/etc/tt/types.xdr for the network
//  database.
// 
int
_tt_map_xdr_dbpaths(_Tt_string &udb, _Tt_string &sdb, _Tt_string &ndb)
{
	_Tt_string_list_ptr path = _Tt_typedb::tt_path();
	if (path.is_null()) return 1;
	_Tt_string_list_cursor pathC( path );
	if (! pathC.next()) {
		return 1;
	}
	udb = *pathC;
	if (! pathC.next()) {
		return 1;
	}
	sdb = *pathC;
	if (! pathC.next()) {
		return 1;
	}
	ndb = path->bot();
	return 1;
}

_Tt_string_list *
_Tt_typedb::tt_path()
{
	_Tt_string_list *pathlist = new _Tt_string_list;
	if (pathlist == 0) return 0;
	_Tt_string path = getenv("TTPATH");
	if (path.len() <= 0) {
		_Tt_string home = getenv("HOME");
		pathlist->append(home.cat("/.tt/types.xdr"));
		pathlist->append(_Tt_string("/etc/tt/types.xdr"));
		pathlist->append(
			_Tt_string("/usr/dt/appconfig/tttypes/types.xdr"));
		home = getenv("OPENWINHOME");
		if (home.len() == 0) {
			home = "/usr/openwin";
		}
		pathlist->append(home.cat(_Tt_string("/etc/tt/types.xdr")));
	} else {
		// parse the user:system:network from path variable
		int n = 0;
		_Tt_string pathname;
		while (n >= 0) {
			n = path.index(':');
			if (n > 0) {
				pathname = path.left(n);
				if (pathname.len() == 0) break;
				path = path.right(path.len() - n - 1);
			} else {
				pathname = path;
				path = 0;
			}
			if (_tt_isdir(pathname)) {
				pathname = pathname.cat("/types.xdr");
			}
			pathlist->append( pathname );
		}
	}
	return pathlist;
}


Tt_status _Tt_typedb::
init_xdr(const _Tt_string &compiled_file)
{
	//
	// A temporary is needed because
	// a types file is an XDRed _Tt_typedb_ptr, (XXX)
	// instead of an XDRed _Tt_typedb, and
	// _Tt_typedb_ptr::xdr() creates a new _Tt_typedb.
	// This was a bad choice, but we are stuck with it.
	//
	_Tt_typedb_ptr	tmpdb;
	Tt_status	status;
	//
	// Need to remember what kind of _Tt_typedb we are,
	// in case we are asked to remove or merge types.
	//
	_flags |= (1<<_TT_TYPEDB_XDR_MODE);
	
	status = merge_from(compiled_file, tmpdb);
	if (status != TT_OK) {
		return(status);
	}
	//
	// Now snare a reference to the tables of the temp db.
	// The temp db goes away, but we keep its tables.
	//
	if (!tmpdb.is_null()) {
		ptable = tmpdb->ptable;
		otable = tmpdb->otable;
	}
	return TT_OK;
}

Tt_status _Tt_typedb::
init_xdr(FILE *f)
{
	_Tt_typedb_ptr	tmpdb;
	Tt_status	status;
	int		version;

	_flags |= (1<<_TT_TYPEDB_XDR_MODE);

	status = merge_from(f, tmpdb, version);
	if (status != TT_OK) {
		return(status);
	}

	if (!tmpdb.is_null()) {
		ptable = tmpdb->ptable;
		otable = tmpdb->otable;
	}
	return TT_OK;
}

// 
// Initializes this object from the given xdr database.
// Returns:
//	TT_ERR_PATH		Bad $TTPATH
//	TT_ERR_NO_MATCH		version mismatch
//	TT_ERR_DBCONSIST	XDR failure, corrupt database
// 
Tt_status _Tt_typedb::
init_xdr(_Tt_typedbLevel xdb)
{
	_Tt_typedb_ptr		tmpdb;
	Tt_status		status;
	
	if (! _tt_map_xdr_dbpaths(user_db, system_db, network_db)) {
		_tt_syslog(stderr, LOG_ERR, "$TTPATH: %s", strerror(EINVAL));
		return(TT_ERR_PATH);
	}

	_flags |= (1<<_TT_TYPEDB_XDR_MODE);
	
	// type files are read in in reverse order of TTPATH
	// variable so that entries in databases to the left
	// of others in TTPATH shadow those to the right.
	
	_Tt_string_list_ptr path;
	switch (xdb) {
	    case TypedbAll:
	    default:
		path = tt_path();
		if (path.is_null()) {
			status = TT_ERR_NOMEM;
		} else {
			_Tt_string_list_cursor pathC( path );
			status = TT_OK;
			while (pathC.prev() && (status == TT_OK)) {
				status = merge_from(*pathC, tmpdb);
			}
			if (status == TT_OK) {
				_flags |= (1<<_TT_TYPEDB_NETWORK);
				_flags |= (1<<_TT_TYPEDB_SYSTEM);
				_flags |= (1<<_TT_TYPEDB_USER);
			}
		}
		break;
	    case TypedbNetwork:
		if (network_db.len()) {
			status = merge_from(network_db, tmpdb);
			if (status == TT_OK) {
				_flags |= (1<<_TT_TYPEDB_NETWORK);
			}
		} else {
			_tt_syslog(stderr, LOG_ERR, "!network_db.len()");
			status = TT_ERR_INTERNAL;
		}
		break;
	    case TypedbSystem:
		if (system_db.len()) {
			status = merge_from(system_db, tmpdb);
			if (status == TT_OK) {
				_flags |= (1<<_TT_TYPEDB_SYSTEM);
			}
		} else {
			_tt_syslog(stderr, LOG_ERR, "!system_db.len()");
			status = TT_ERR_INTERNAL;
		}
		break;
	    case TypedbUser:
		if (user_db.len()) {
			status = merge_from(user_db, tmpdb);
			if (status == TT_OK) {
				_flags |= (1<<_TT_TYPEDB_USER);
			}
		} else if (xdb != TypedbAll) {
			_tt_syslog(stderr, LOG_ERR, "!user_db.len()");
			status = TT_ERR_INTERNAL;
		}
		break;
	}
	if (status != TT_OK) {
		return status;
	}
	if (!tmpdb.is_null()) {
		ptable = tmpdb->ptable;
		otable = tmpdb->otable;
	}
	return(TT_OK);
}


// 
// Merges (or reads) types from dbpath into a (new) _Tt_typedb in tdb
// 
Tt_status _Tt_typedb::
merge_from(const _Tt_string &dbpath, _Tt_typedb_ptr &tdb)
{
	FILE			*f;
	Tt_status		result;
	int			version;

	// The automatic converter (ttce2xdr) will just touch the
	// user\'s .tt/types.xdr if there were no ToolTalk types in the
	// classing engine db.  This means that a zero-length file
	// is perfectly OK, it just contains no types.

	struct stat	stat_buf;
	if (stat( (char *)dbpath, &stat_buf ) == 0) {
		if (stat_buf.st_size == 0) {
			return TT_OK;
		}
	}

	if ((f = fopen((char *)dbpath, "r"))) {
		fcntl(fileno(f), F_SETFD, 1);	/* close on exec */
		result = merge_from(f, tdb, version);
		fclose(f);
	} else {
		// It is OK for the database not to exist, ToolTalk runs
		// even if there are no types.
		result = TT_OK;
	}
	
	if (result == TT_ERR_NO_MATCH) {
		// This file is newer than we are, so we cannot
		// decode it.
		_tt_syslog(stderr, LOG_ERR,
			   catgets(_ttcatd, 2, 9,
				   "%s is a version %d types "
				   "database, and this version "
				   "can only read versions %d and earlier"),
			   (char *)dbpath, version,
			   TT_PUSH_ROTATE_XDR_VERSION);
	} else if (result == TT_ERR_DBCONSIST) {
		_tt_syslog(stderr, LOG_ERR,
			   catgets(_ttcatd, 2, 10,
				   "could not decode types from types "
				   "database: %s. It may be damaged."),
			   (char *)dbpath);
	}

	return TT_OK;
}


// 
// Merges (or reads) types from f into a (new) _Tt_typedb in tdb
// 
Tt_status _Tt_typedb::
merge_from(FILE *f, _Tt_typedb_ptr &tdb, int &version)
{
	XDR			xdrs;

	xdrstdio_create(&xdrs, f, XDR_DECODE);
	return merge_from(&xdrs, tdb, version);
}

// 
// Merges (or reads) types from xdrs into a (new) _Tt_typedb in tdb
// 
// Picks the version off the xdr xstream and then invokes the
// xdr method on the given _Tt_typedb object to merge in the types.
// Used both by merge_from above to read in files and to handle
// the types sent over from clients via tt_session_types_load().
// 
Tt_status _Tt_typedb::
merge_from(XDR *xdrs, _Tt_typedb_ptr &tdb, int &version)
{
	
	if (! xdr_int(xdrs, &version)) {
		return TT_ERR_DBCONSIST;
	}
	if (version > TT_PUSH_ROTATE_XDR_VERSION) {
		// This file is newer than we are, so we cannot
		// decode it.
		return TT_ERR_NO_MATCH;
	}
	
	_Tt_xdr_version	xvers(version);
	
	if (! tdb.xdr(xdrs)) {
		return TT_ERR_DBCONSIST;
	}
	return TT_OK;
}



// 
// Aborts a write transaction to a databse. If this is a write to an xdr
// database then the lock file is removed. Otherwise it is a Classing
// Engine database and the appropiate abort function is invoked.
// 
int _Tt_typedb::
abort_write()
{

		(void)unlink((char *)_lock_file);
		return(1);

}


// 
// Removes an otype with id otid from the database either in xdr format
// or in Classing Engine format. If the database is in xdr format then
// this method just removes the otype from the in-memory otype table
// since it is assumed that a subsequent _Tt_typedb::end_write will cause
// the in-memory otype table to supersede the one on disk. If the
// database is a Classing Engine database then we remove it using the
// appropiate Classing Engine routines.
// 
int _Tt_typedb::
remove_otype(_Tt_string otid)
{
	_Tt_otype_ptr   ot;
	if (! otable->lookup(otid,ot)) {
		return(1);
	}


	otable->remove(otid);
	return(1);
}



// 
// Removes an ptype with id ptid from the database either in xdr format
// or in Classing Engine format. If the database is in xdr format then
// this method just removes the ptype from the in-memory ptype table
// since it is assumed that a subsequent _Tt_typedb::end_write will cause
// the in-memory ptype table to supersede the one on disk. If the
// database is a Classing Engine database then we remove it using the
// appropiate Classing Engine routines.
// 
int _Tt_typedb::
remove_ptype(_Tt_string ptid)
{
	_Tt_ptype_ptr   pt;
	if (! ptable->lookup(ptid,pt)) {
		return(1);
	}


	ptable->remove(ptid);
	return(1);
}


// 
// Inserts a new ptype into the ptype table for this type database. If
// this database is stored in xdr format then this method just inserts
// the object into the in-memory table since it assumes a subsequent
// invocation of _Tt_typedb::end_write will write out the new table to
// disk. In Classing Engine format we create a Classing Engine entry and
// then use the appropiate ce functions to insert it into the database.
// 
int _Tt_typedb::
insert(_Tt_ptype_ptr &pt)
{

	ptable->insert(pt);


	return(1);

}


// 
// Inserts a new otype into the ptype table for this type database. If
// this database is stored in xdr format then this method just inserts
// the object into the in-memory table since it assumes a subsequent
// invocation of _Tt_typedb::end_write will write out the new table to
// disk. In Classing Engine format we create a Classing Engine entry and
// then use the appropiate ce functions to insert it into the database.
// 
int _Tt_typedb::
insert(_Tt_otype_ptr &ot)
{

	otable->insert(ot);

	return(1);
}


// 
// Prepares the database to be written out to disk. In Classing Engine
// mode this means we have to add the ToolTalk namespace to the database
// if it doesn't exist and invoke the appropiate Classing Engine function
// to start the write transaction. In xdr mode, we have to acquire the
// lock file to write out the database.
// 
int _Tt_typedb::
begin_write(_Tt_typedbLevel /* db */)
{

	int		n;
	int		fd;
	_Tt_string	path;
	_Tt_string	dir_path;

	// acquire a write lock
	if (_flags&(1<<_TT_TYPEDB_USER)) {
		path = user_db;
	} else if (_flags&(1<<_TT_TYPEDB_SYSTEM)) {
		path = system_db;
	} else if (_flags&(1<<_TT_TYPEDB_NETWORK)) {
		path = network_db;
	}

	n = path.rindex('/');
	if (n == -1) {
		dir_path = ".";
	} else {
		dir_path = path.left(n);
		
	}
	_lock_file = dir_path.cat("/.tt_lock");

	// mkdir in case the "tt" or ".tt" subdirectory doesn't exist.

	(void)mkdir((char *)dir_path, 0777); // ignore errors, probably EEXIST

	// The lock is held only while the database is rewritten, so poll
	// for it with a short backoff (it used to be 5 x sleep(2)) within
	// the same 10 s budget.  The holder's "pid hostname" is recorded in
	// the lock so that one left behind by a process that died can be
	// recognised and removed.
	int	waited_ms = 0;
	int	delay_ms = 10;
	int	warned = 0;
	int	broken = 0;

	for (;;) {
		fd = open((char *)_lock_file, O_WRONLY|O_CREAT|O_EXCL, 0777);
		if (fd != -1 || errno != EEXIST) {
			// Locked, or locking is impossible here (e.g. a
			// read-only directory); as before, carry on and let
			// writing the database report the problem.
			break;
		}
		if (broken < 3 && break_stale_lock()) {
			broken++;
			continue;
		}
		if (!warned) {
			errno = EEXIST;
			_tt_syslog(stderr, LOG_ERR, "%s: %m",
				   (char *)_lock_file);
			warned = 1;
		}
		if (waited_ms >= 10000) {
			_flags &= ~(1<<_TT_TYPEDB_LOCKED);
			return(0);
		}
		(void)poll(NULL, 0, delay_ms);
		waited_ms += delay_ms;
		delay_ms = delay_ms < 250 ? 2 * delay_ms : 500;
	}
	if (fd > -1 ) {
		char	owner[320];
		int	len = snprintf(owner, sizeof owner, "%ld %s\n",
				       (long)getpid(),
				       (char *)_tt_gethostname());

		if (len > 0 && len < (int)sizeof owner) {
			(void)::write(fd, owner, len);
		}
		close(fd);	// Cleanup
	}
	_flags |= (1<<_TT_TYPEDB_LOCKED);
	return(1);
}


//
// Removes _lock_file if it records a holder on this host that no longer
// exists.  Locks written by older versions are empty and are left alone.
// Returns 1 if the lock was removed.
//
int _Tt_typedb::
break_stale_lock()
{
	char		buf[512];
	char		host[256];
	long		pid;
	struct stat	st_open, st_now;
	int		fd, len;

	fd = open((char *)_lock_file, O_RDONLY);
	if (fd == -1) {
		return 0;
	}
	len = read(fd, buf, sizeof buf - 1);
	if (fstat(fd, &st_open) != 0) {
		len = -1;
	}
	close(fd);
	if (len <= 0) {
		return 0;
	}
	buf[len] = '\0';
	if (sscanf(buf, "%ld %255s", &pid, host) != 2 || pid <= 0 ||
	    strcmp(host, (char *)_tt_gethostname()) != 0) {
		return 0;
	}
	if (kill((pid_t)pid, 0) == 0 || errno != ESRCH) {
		return 0;	// holder is alive (or not ours to judge)
	}
	// Remove only the file we judged, not a lock someone else has
	// meanwhile broken and re-taken.
	if (stat((char *)_lock_file, &st_now) != 0 ||
	    st_now.st_dev != st_open.st_dev ||
	    st_now.st_ino != st_open.st_ino) {
		return 0;
	}
	if (unlink((char *)_lock_file) != 0) {
		return 0;
	}
	_tt_syslog(stderr, LOG_WARNING, "%s: removed stale lock of pid %ld",
		   (char *)_lock_file, pid);
	return 1;
}


// 
// Commits the write transaction to the database. In Classing Engine mode
// we just call the ce_commit_write function. In xdr mode we write out
// the database to a temporary file and then rename the file to be the
// appropiate path.
// 
int _Tt_typedb::
end_write()
{

	_Tt_string		dbpath;
	_Tt_string		dbpath_tmp;
	int			success;

	if (!(_flags&(1<<_TT_TYPEDB_LOCKED))) {
		return(0);
	}
	if (_flags&(1<<_TT_TYPEDB_USER)) {
		dbpath = user_db;
	} else if (_flags&(1<<_TT_TYPEDB_SYSTEM)) {
		dbpath = system_db;
	} else if (_flags&(1<<_TT_TYPEDB_NETWORK)) {
		dbpath = network_db;
	}
	dbpath_tmp = dbpath.cat("_tmp");
	success = write(dbpath_tmp) == TT_OK;
	if (! success) {
		(void)unlink((char *)dbpath_tmp);
		(void)unlink((char *)_lock_file);
		return(0);
	}

	success = (0 == rename((char *)dbpath_tmp, (char *)dbpath));
	if (success) {
		send_saved( dbpath );
		_tt_syslog(stdout, LOG_INFO,
			   catgets(_ttcatd, 2, 11, "Overwrote %s"),
			   (char *)dbpath);
	} else {
		_tt_syslog(stderr, LOG_ERR,
			   "rename( \"%s\", \"%s\" ): %m",
			   (char *)dbpath_tmp, (char *)dbpath);
	}
	(void)unlink((char *)_lock_file);
	(void)unlink((char *)dbpath_tmp);
	return(success);
}

Tt_status _Tt_typedb::
write(const _Tt_string &outfile)
{
	if (outfile.len() == 0) {
		return TT_DESKTOP_ENOENT;
	}
	FILE *f = fopen((char *)outfile,"w");
	if (f == 0) {
		_tt_syslog(stderr, LOG_ERR, "%s: %m", (char *)outfile);
		return _tt_errno_status( errno );
	}
	fcntl(fileno(f), F_SETFD, 1);	/* Close on exec */
	Tt_status status = write( f );
        fclose(f);
	return status;
}

Tt_status _Tt_typedb::
write(FILE *outfile)
{
	int	xdr_version;
        XDR	xdrs;
	_Tt_typedb_ptr		tdb_ptr = this;

	// For maximum compatibility, only write out types files
	// using the new XDR routines if the signatures have
	// contexts.
	xdr_version = xdr_version_required();
        _Tt_xdr_version xvers(xdr_version);
 
        xdrstdio_create(&xdrs, outfile, XDR_ENCODE);
        if (! xdr_int(&xdrs, &xdr_version) || ! tdb_ptr.xdr(&xdrs)) {
		_tt_syslog(stderr, LOG_ERR, "! _Tt_typedb_ptr::xdr()" );
                return TT_ERR_XDR;
        }
	return TT_OK;
}

//
// Do a tt_open() (if needed) and a ttdt_file_notice(), syslog()ing any error.
//
Tt_status _Tt_typedb::
send_saved(const _Tt_string &savedfile)
{
	const char	*default_opt = "Saved";

	char *procid = tt_default_procid();
	Tt_status status = tt_ptr_error( procid );
	switch (status) {
	    case TT_OK:
		tt_free( procid );
		break;
	    case TT_ERR_NOMP:
	    case TT_ERR_PROCID:
		procid = tt_open();
		status = tt_ptr_error( procid );
		if (status == TT_OK) {
			tt_free( procid );
		}
		break;
	    default:
		break;
	}
	if (status != TT_OK) {
		//
		// No default session from which to send the notice,
		// so silently omit it.  In principle libtt can send
		// a file-scoped notice without having a default session,
		// but the API does not permit this.
		//
		return status;
	}
	//
	// The HP linker thinks that ttdt_file_notice() ultimately
	// depends on some Xt symbols.  The HP linker is wrong.
	// The AIX linker knows better.  On SunOS, of course, we
	// use dlopen().
	//
	Tt_message msg = tt_message_create();
	tt_message_class_set( msg, TT_NOTICE );
	tt_message_scope_set( msg, TT_FILE );
	tt_message_address_set( msg, TT_PROCEDURE );
	tt_message_op_set( msg, default_opt );
	status = tt_message_file_set( msg, savedfile );
	if (status != TT_OK) {
		_tt_syslog(stderr, LOG_ERR,
		   "tt_message_file_set(): %s", tt_status_message(status));
	}
	tt_message_arg_add( msg, TT_IN, "File", 0 );
	status = tt_message_send( msg );
	if (status != TT_OK) {
		_tt_syslog(stderr, LOG_ERR,
			   "tt_message_send(): %s", tt_status_message(status));
	}
	return status;
}



 
Tt_status _Tt_typedb::
init_ce(
	_Tt_typedbLevel
)
{

	_tt_syslog(stderr, LOG_ERR, "_Tt_typedb::init_ce()!");
	return(TT_ERR_INTERNAL);
}


_Tt_typedbLevel _Tt_typedb::
level( const _Tt_string &dbname )
{
	if (dbname == "user") {
		return TypedbUser;
	} else if (dbname == "system") {
		return TypedbSystem;
	} else if (dbname == "network") {
		return TypedbNetwork;
	} else {
		return TypedbNone;
	}
}

const char * _Tt_typedb::
level_name( _Tt_typedbLevel db )
{
	switch (db) {
	    case TypedbUser:
		return "user";
	    case TypedbSystem:
		return "system";
	    case TypedbNetwork:
		return "network";
	    default:
		return 0;
	}
}


int _Tt_typedb::
xdr_version_required() const
{
	int version = TT_TYPESDB_DEFAULT_XDR_VERSION;
	_Tt_ptype_table_cursor 	ptypes(ptable);
	while (ptypes.next()) {
		if(ptypes->xdr_version_required() > version) {
			version = ptypes->xdr_version_required();
		}
//		version = max(version, ptypes->xdr_version_required());
	}
	_Tt_otype_table_cursor	otypes(otable);
	while (otypes.next()) {
		if(otypes->xdr_version_required() > version) {
			version = otypes->xdr_version_required();
		}
//		version = max(version, otypes->xdr_version_required());
	}
	return version;
}


static void
write_out_tt_attrs(const _Tt_ostream &os)
{
	int		i;

	FILE *fs = os.theFILE();
	for (i=0; i < _TT_CE_ATTR_LAST; i++) {
		fprintf(fs,"\t\t(%s,string,<attr>)\n",
			_tt_ce_attr_string((_Tt_ce_attr)i));
	}
	fprintf(fs,"\t)");
}


void _Tt_typedb::
pretty_print(const _Tt_ostream &os) const
{
	_Tt_ptype_table_cursor 	ptypes(ptable);
	while (ptypes.next()) {
		ptypes->pretty_print(os);
		os << "\n";
	}
	_Tt_otype_table_cursor	otypes(otable);
	while (otypes.next()) {
		otypes->pretty_print(os);
		os << "\n";
	}
}



void _Tt_typedb::
print(const _Tt_ostream &os) const
{
	if (write_ce_header(os)) {
		os << "\nNS_ENTRIES= (\n\t(\t";
		write_out_tt_attrs(os);
		ptable->print(_tt_ptype_print, os);
		otable->print(_tt_otype_print, os);
		os << "\n)\n}";
	}
}


int _Tt_typedb::
write_ce_header(const _Tt_ostream &os) const
{
	os << "{\nNS_NAME=" << TT_NS_NAME
	   << "\nNS_ATTR=(\n\t\t(NS_MANAGER,string,"
	      "<$CEPATH/tns_mgr.so>)\n\t)";
	return(1);
}



