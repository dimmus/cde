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
//%%  $XConsortium: tt_db_hostname_global_map_ref.C /main/3 1995/10/23 10:02:29 rswiston $ 			 				
/*
 * tt_db_hostname_global_map_ref.cc - Define the TT DB client hostname global
 *                           map ref class.  This class contains a global map
 *			     that provides a hostname to DB connection mapping.
 *			     This way only one RPC connection per machine is
 *			     created.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "db/tt_db_hostname_global_map_ref.h"
#include "util/tt_global_env.h"

//
// Negative cache: a host whose rpc.ttdbserverd could not be reached is
// not tried again for TT_DB_NEG_CACHE_SECS, since each attempt costs a
// name lookup, a portmapper query (or a connect timeout) and a syslog
// message, and callers retry on every file-scoped operation.
//
#define TT_DB_NEG_CACHE_SECS	30
#define TT_DB_NEG_CACHE_SIZE	8

static struct {
	char		*host;
	time_t		 until;
	_Tt_db_results	 results;
} dbNegCache[TT_DB_NEG_CACHE_SIZE];

static int
negCacheFind(const _Tt_string &host)
{
	for (int i = 0; i < TT_DB_NEG_CACHE_SIZE; i++) {
		if (dbNegCache[i].host &&
		    strcmp(dbNegCache[i].host, (char *)host) == 0) {
			return i;
		}
	}
	return -1;
}

// Returns TT_DB_OK, or the error of a recent failed attempt on host.
static _Tt_db_results
negCacheCheck(const _Tt_string &host)
{
	int i = negCacheFind(host);

	if (i == -1) {
		return TT_DB_OK;
	}
	if (time(0) < dbNegCache[i].until) {
		return dbNegCache[i].results;
	}
	free(dbNegCache[i].host);
	dbNegCache[i].host = 0;
	return TT_DB_OK;
}

static void
negCacheAdd(const _Tt_string &host, _Tt_db_results results)
{
	time_t	now = time(0);
	int	i = negCacheFind(host);

	if (i == -1) {
		// A free or expired slot, else the one expiring first.
		i = 0;
		for (int j = 0; j < TT_DB_NEG_CACHE_SIZE; j++) {
			if (!dbNegCache[j].host || dbNegCache[j].until <= now) {
				i = j;
				break;
			}
			if (dbNegCache[j].until < dbNegCache[i].until) {
				i = j;
			}
		}
		free(dbNegCache[i].host);
		dbNegCache[i].host = strdup((char *)host);
		if (!dbNegCache[i].host) {
			return;
		}
	}
	dbNegCache[i].until = now + TT_DB_NEG_CACHE_SECS;
	dbNegCache[i].results = results;
}

_Tt_db_client_table_ptr*
_Tt_db_hostname_global_map_ref::dbHostnameMap = (_Tt_db_client_table_ptr *)NULL;

_Tt_db_hostname_global_map_ref::_Tt_db_hostname_global_map_ref ()
{
  if (!dbHostnameMap) {
    dbHostnameMap = new _Tt_db_client_table_ptr;
    *dbHostnameMap = new _Tt_db_client_table
			 ((_Tt_object_table_keyfn)
			  &_Tt_db_hostname_global_map_ref::dbHostnameMapKey);
  }
}

_Tt_db_hostname_global_map_ref::~_Tt_db_hostname_global_map_ref ()
{
  // The static table is freed up when the application dies or when
  // _Tt_db_hostname_global_map_ref::flush() is called.
}

void _Tt_db_hostname_global_map_ref::addDB (_Tt_db_client_ptr &db)
{
  (*dbHostnameMap)->insert(db);
}

void _Tt_db_hostname_global_map_ref::removeDB (const _Tt_string &hostname)
{
  (*dbHostnameMap)->remove(hostname);
}

_Tt_db_client_ptr
_Tt_db_hostname_global_map_ref::getDB (const _Tt_string &hostname,
				       _Tt_string       &real_hostname,
				       _Tt_db_results	&results)
{
  _Tt_string temp_hostname = _tt_global->db_hr_map.findEntry(hostname);
  if (temp_hostname.is_null() || !temp_hostname.len()) {
    temp_hostname = hostname;
  }
  real_hostname = temp_hostname;

  return connectDB(real_hostname, results);
}

_Tt_db_client_ptr
_Tt_db_hostname_global_map_ref::getDirectDB (const _Tt_string &hostname,
					     _Tt_db_results   &results)
{
  return connectDB(hostname, results);
}

// Returns the cached connection to the dbserver on hostname, making one
// if there is none (and no recent failure to make one).
_Tt_db_client_ptr
_Tt_db_hostname_global_map_ref::connectDB (const _Tt_string &hostname,
					   _Tt_db_results   &results)
{
  _Tt_db_client_ptr db_ptr = (*dbHostnameMap)->lookup(hostname);
  if (db_ptr.is_null()) {
    results = negCacheCheck(hostname);
    if (results != TT_DB_OK) {
      return db_ptr;
    }
    db_ptr = new _Tt_db_client(hostname, results);
    if (results != TT_DB_OK) {
      negCacheAdd(hostname, results);
      db_ptr = (_Tt_db_client *)0;
      return db_ptr;
    }
    addDB(db_ptr);
  }
  else {
    results = TT_DB_OK;
  }

  return db_ptr;
}

_Tt_string _Tt_db_hostname_global_map_ref::dbHostnameMapKey (_Tt_object_ptr &db)
{
  return (((_Tt_db_client *)db.c_pointer())->getHostname());
}

void _Tt_db_hostname_global_map_ref::flush ()
{
  if (dbHostnameMap) {
    delete dbHostnameMap;
  }
  dbHostnameMap = (_Tt_db_client_table_ptr *)NULL;

  for (int i = 0; i < TT_DB_NEG_CACHE_SIZE; i++) {
    free(dbNegCache[i].host);
    dbNegCache[i].host = 0;
  }
}
