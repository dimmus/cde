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
//%%  $XConsortium: tt_global_env.C /main/4 1995/11/21 19:27:28 cde-sun $ 			 				
/*
 *
 * @(#)tt_global_env.C	1.25 95/09/26
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#include <unistd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mp/mp_mp.h"
#include "util/tt_global_env.h"
#include "util/tt_host.h"
#include "util/tt_port.h"     



_Tt_global	*_tt_global = (_Tt_global *)0;
extern          _Tt_mp * _tt_mp;

_Tt_global::
_Tt_global()
{
	silent = 0;
	_xdr_version = TT_XDR_VERSION;
	_host_cache = new _Tt_host_table(_tt_host_addr, 50);
	_host_byname_cache = new _Tt_host_table(_tt_host_name, 50);
	_multithreaded = 0;
	uid = geteuid();
	gid = getegid();
	xdisplayname = getenv("DISPLAY");
	universal_null_string = new _Tt_string_buf;
	event_counter = 0;
	next_garbage_run = -1;		// Force 1st time.
#ifdef OPT_XTHREADS
	xmutex_init(&global_mutex);
	xthread_key_create(&threadkey, NULL);
	xcondition_init(&_proceed);
	_lock_free = 1;
#endif
}


_Tt_global::
~_Tt_global()
{
}


int _Tt_global::
find_host(_Tt_string haddr, _Tt_host_ptr &h, int create_ifnot)
{
	_Tt_host_ptr hc;
	// haddr is either a raw 4-byte IPv4 address (what the server
	// keeps for procids) or the same address in '.' notation (what
	// session addresses carry).  The cache is keyed on the raw form,
	// so convert dotted addresses first; otherwise they never hit.
	int		dotted = (haddr.len() != 4);
	_Tt_string	key = haddr;
	struct in_addr	ia;

	if (dotted && inet_aton((char *)haddr, &ia)) {
		key.set((const unsigned char *)&ia.s_addr, 4);
	}

	if (_host_cache->lookup(key, hc)) {
		h = hc;
		return(1);
	}
	
	if (! create_ifnot) {
		return(0);
	}
	h = new _Tt_host();
	/* host not found, add it to list */
	// Clients only need the address of a session host: its name
	// is used only in diagnostics (and by the TLI and secure-RPC
	// transports, which need it).  A reverse DNS lookup on every
	// tt_open can stall for seconds when the resolver is slow, so
	// clients skip it.  A dotted address is never passed to
	// init_byaddr(), which wants the raw bytes.
#if defined(OPT_TLI) || defined(OPT_SECURE_RPC)
	int resolve_name = 1;
#else
	int resolve_name = _tt_mp->in_server();
#endif
	if (dotted) {
		if (! h->init_bystringaddr(haddr, resolve_name)) {
			return(0);
		}
	} else if (_tt_mp->in_server()) {
		if (! h->init_byaddr(haddr)) {
			// No name for this address: it is still a
			// perfectly usable address.  (This used to fail,
			// so a client on a host without reverse DNS got
			// TT_ERR_PROCID.)
			const unsigned char *b =
				(const unsigned char *)(const char *)haddr;
			char str[16];
			sprintf(str, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
			if (! h->init_bystringaddr(str, 0)) {
				return(0);
			}
		}
	} else {
		// Raw address on the client: nothing to parse it as.
		return(0);
	}
	_host_cache->insert(h);
	_host_byname_cache->insert(h);
	return(1);
}


int _Tt_global::
find_host_byname(_Tt_string name, _Tt_host_ptr &h)
{
	_Tt_host_ptr hc;
	if (_host_byname_cache->lookup(name,hc)) {
		h = hc;
		return(1);
	}
	h = new _Tt_host();
	if (h->init_byname(name)) {
		_host_cache->insert(h);
		_host_byname_cache->insert(h);
		return(1);
	}
	
	return(0);
}


int _Tt_global::
get_local_host(_Tt_host_ptr &h)
{
	if (_local_host.is_null()) {
		_local_host = new _Tt_host();
		if (! _local_host->init_byname((char *)0)) {
			_local_host = (_Tt_host *)0;
			return(0);
		}
	}
	
	h = _local_host;
	return(1);
}

int _Tt_global::_maxfds = -1;

int _Tt_global::
maxfds()
{
	if (_maxfds != -1) {
		return _maxfds;
	}
	_maxfds = _tt_getdtablesize();
	return _maxfds;
}

// The following version set/get functions used to be inline.  For
// some reason, cfront started generating references to the default
// new and delete operators because of this.  Of course, we can't
// tolerate that since we want our clients to link without libC.

// returns current xdr/rpc version
int _Tt_global::
xdr_version() {
	return _xdr_version;
}


// sets the current xdr version, defaults to the
// TT_XDR_VERSION constant.
void _Tt_global::
set_xdr_version(int v) {
	_xdr_version = v;
}


void _Tt_global::
grab_mutex()
{
#ifdef OPT_XTHREADS
	int lockstat;
	while (lockstat = xmutex_lock(&global_mutex)) {}

	while (!_lock_free) {
		xcondition_wait(&_proceed, &global_mutex);
	}
	_lock_free = 0;
#endif
}

void _Tt_global::
drop_mutex()
{
#ifdef OPT_XTHREADS
	int lockstat;

	_lock_free = 1;
	xcondition_signal(&_proceed);
	while (lockstat = xmutex_unlock(&global_mutex)) {}
#endif
}



