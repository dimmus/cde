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
//%%  $TOG: mp_rpc_server.C /main/11 1999/08/30 11:03:00 mgreess $ 			 				
/*
 *
 * @(#)mp_rpc_server.C	1.46	94/11/17
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#include "tt_options.h"

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

#include "mp_rpc_server.h"
#include "util/tt_port.h"
#include "util/tt_gettext.h"
#include "util/tt_global_env.h"
#include "mp/mp_mp.h"
#include "mp/mp_rpc.h"

#if defined(OPT_TLI)
#include <netdir.h>
static int	gettransient(int, netconfig *, netbuf *);
#if defined(OPT_BUG_SUNOS_5)
extern "C" { char *	nc_sperror(); }
#endif
#else 
#include <rpc/pmap_clnt.h>
#include <netinet/tcp.h>
static int	gettransient(int,int,int *);
#endif /* OPT_TLI */

#if defined(OPT_BUG_AIX)
     typedef void (*SERVICE_FN_TYPE)();
#else
     typedef void (*SERVICE_FN_TYPE)(struct svc_req *, SVCXPRT*);
#endif


/* 
 * Constructs an rpc server for the given program, version and socket.
 */
_Tt_rpc_server::
_Tt_rpc_server(int program, int version, int Rsocket, _Tt_auth &auth)
{
	_version = version;
	_socket = Rsocket;
	_program = program;
	_auth = auth;
	_rpc_fd = 0;
	_transp = NULL;
	_registered = 0;
}


/* 
 * Destroys an rpc server. Unsets the program,version mapping in the
 * portmapper. 
 */
_Tt_rpc_server::
~_Tt_rpc_server()
{
	unset();
}


void _Tt_rpc_server::
unset()
{
	// Only a server that registered with the portmapper unsets
	// (one created on a connected descriptor shares the program
	// number with the main server but must not unregister it).
	if (! _registered) {
		return;
	}
	_registered = 0;
	for (int version = _version; version >= 1; version--) {
#ifndef OPT_TLI
		pmap_unset(_program, version);
#else
		rpcb_unset(_program, version, (netconfig *)0);
#endif				// OPT_TLI
	}
}


// Self-pipe that lets a signal handler interrupt run_until() even
// when the signal arrives after *stop was last checked but before
// the wait began.
static int	_tt_wake_pipe[2] = { -1, -1 };

static void
_tt_wake_pipe_init()
{
	if (_tt_wake_pipe[0] >= 0) {
		return;
	}
	if (pipe(_tt_wake_pipe) != 0) {
		_tt_wake_pipe[0] = _tt_wake_pipe[1] = -1;
		return;
	}
	for (int i = 0; i < 2; i++) {
		fcntl(_tt_wake_pipe[i], F_SETFD, FD_CLOEXEC);
		fcntl(_tt_wake_pipe[i], F_SETFL,
		      fcntl(_tt_wake_pipe[i], F_GETFL) | O_NONBLOCK);
	}
}


void _Tt_rpc_server::
wakeup()
{
	if (_tt_wake_pipe[1] >= 0) {
		int saved_errno = errno;
		ssize_t n = write(_tt_wake_pipe[1], "w", 1);
		(void)n;
		errno = saved_errno;
	}
}


/* 
 * Initializes an rpc server with a service function. If _program is set
 * to -1 then an unused program number is obtained using the gettransient
 * function. If _socket is anything other than RPC_ANYSOCK then it will
 * be used to create the rpc transport using svfd_create and the rpc
 * numbers will not be registered with the portmapper.
 */
int _Tt_rpc_server::
init(void (*service_fn)(struct svc_req *, SVCXPRT *))
{
	char		*bufopt = (char *)0;

#ifndef OPT_TLI

	bufopt = getenv("TT_BUFSIZE");
	unsigned int buffersize = (bufopt != (char *)0) ? atoi(bufopt) : 32000;

	if (_socket != RPC_ANYSOCK) {
		_transp = svcfd_create(_socket, buffersize, buffersize);
		if (_transp == (SVCXPRT *)0) {
			return(0);
		}
		if (!svc_register(_transp, _program, _version,
				  (SERVICE_FN_TYPE)service_fn, 0))
		{
			_tt_syslog(0, LOG_ERR, "svc_register(): %m");
			return(0);
		}

		return(1);
	}

	// From here on the program is (to be) registered with the
	// portmapper, so unset() must remove it again.
	_registered = 1;

	if (_program == -1) {
		if (! (_program =
		       gettransient(IPPROTO_TCP, _version, &_socket))) {
			return(0);
		}
	} else {
		_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (_socket < 0) {
			_tt_syslog(0, LOG_ERR,
				   "_Tt_rpc_server::init(): socket(): %m");
			return 0;
		}
	}
	int optval = 1;
	if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY,
		       (char *)&optval, sizeof(int)) == -1) {
		_tt_syslog(0, LOG_ERR, "setsockopt(TCP_NODELAY): %m");
	}
	if (setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&buffersize,
		       sizeof(int)) == -1) {
		_tt_syslog(0, LOG_ERR, "setsockopt(SO_RCVBUF): %m");
	}
	if (setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&buffersize,
		       sizeof(int)) == -1) {
		_tt_syslog(0, LOG_ERR, "setsockopt(SO_SNDBUF): %m");
	}
	_transp = svctcp_create(_socket, buffersize, buffersize);

	if (_transp == (SVCXPRT *)0) {
		return(0);
	}

#if defined(OPT_TIRPC)
        /* JET: HACK WARNING 7/1/18
         *
         * With earlier versions of RPC and TIRPC it seems that
         * svctcp_create() calles listen() on the socket (as seen by
         * debugger and strace).  This is the expected behavior in TT.
         *
         * However, with newer systems (ArchLinux 5/18+ and similar
         * bleeding edge versions of SuSE's equivalent: Tumbleweed),
         * this behavior seems to have changed.
         *
         * ttsession goes into an infinite loop trying to accept() a
         * connection in the TIRPC library.  It appears listen() is no
         * longer called on the socket via svctcp_create().  The hack
         * below, always causes listen() to be called on the socket.
         * We do not care if it fails, or is called twice on the same
         * socket.
         */
        listen(_socket, 5);
#endif

	if (   !svc_register(_transp, _program, _version,
			     (SERVICE_FN_TYPE)service_fn, IPPROTO_TCP)
	    || !svc_register(_transp, _program, 1,
			     (SERVICE_FN_TYPE)service_fn, 0))
	{
		_tt_syslog(0, LOG_ERR, "svc_register(): %m");
		return(0);
	}
#else
	netconfig		*nconf;
	void			*handlep;
	t_info			tinfo;
	int			fd;


	if ((handlep = setnetconfig()) == (void *)0) {
		_tt_syslog(0, LOG_ERR, "setnetconfig(): %s", nc_sperror());
		return(0);
	}

	// Find a connection-oriented transport.
	while (nconf = getnetconfig(handlep)) {
		if ((nconf->nc_semantics == NC_TPI_COTS) ||
		    (nconf->nc_semantics == NC_TPI_COTS_ORD)) {

		        // Make sure this netconfig maps to an address
		        if (0 == strcmp(nconf->nc_protofmly, NC_INET))
			    break;
		}
	}

	// If we failed to find a suitable transport, exit.
	if (nconf == (netconfig *)0) {
		endnetconfig(handlep);
		_tt_syslog(0, LOG_ERR,
			   catgets(_ttcatd, 2, 3,
				   "No connection-oriented transport"));
		return(0);
	}
	
	fd = t_open(nconf->nc_device, O_RDWR, &tinfo);
	if (fd == -1) {
		_tt_syslog(0, LOG_ERR,
			   "_Tt_rpc_server::init(): t_open(): %s",
			   t_strerror( t_errno ) );
		endnetconfig(handlep);
		return 0;
	}
	// No longer need to try to set NODELAY here as TIRPC does it for us
	// tinfo.tsdu can be negative, but that's not a valid buf size.
	u_int buf_size = 0;
	if (tinfo.tsdu > 0) {
		buf_size = (u_int)tinfo.tsdu;
	}
	_transp = svc_tli_create(fd, nconf, (struct t_bind *)0,
				 buf_size, buf_size);
	if (_transp == (SVCXPRT *)0) {
		_tt_syslog(0, LOG_ERR, "svc_tli_create(): 0");
		(void)t_close(fd);
		return(0);
	}
	if (_program == -1 &&
	    (! (_program = gettransient(_version, nconf,
					&_transp->xp_ltaddr)))) {
		_tt_syslog(0, LOG_ERR, "gettransient(): 0");
		return(0);
	}
	for (int version = _version; version >= 1; version--) {
		if (!svc_reg(_transp, _program, version,
			     (SERVICE_FN_TYPE)service_fn, nconf)) {
			_tt_syslog(0, LOG_ERR, "svc_reg(,,%d): 0", version);
			return(0);
		}
	}
	_registered = 1;
	// it is important to not call endnetconfig until one is done
	// using nconf as endnetconfig frees the nconf storage.
	(void)endnetconfig(handlep);
#endif				/* OPT_TLI */
	_tt_wake_pipe_init();
	// now figure out what fd the rpc package is using
	for (int i=0; i < FD_SETSIZE; i++) {
		if (FD_ISSET(i, &svc_fdset)) {
			_rpc_fd = i;
		}
	}

	return(1);
}


/*
 * Returns the TCP port the server listens on (0 if unknown, e.g. for
 * a TLI transport or a server on a connected descriptor).  It is
 * advertised in the session address so that clients can connect
 * without a portmapper query.
 */
int _Tt_rpc_server::
port()
{
#ifndef OPT_TLI
	sockaddr_in	addr;
	socklen_t	len = sizeof(addr);

	if (_transp != (SVCXPRT *)0 &&
	    getsockname(_socket, (sockaddr *)&addr, &len) == 0 &&
	    addr.sin_family == AF_INET) {
		return ntohs(addr.sin_port);
	}
#endif
	return 0;
}


/* 
 * Runs an rpc server. If a non-negative timeout is given then this
 * function will return if the timeout expired before any rpc requests
 * came in. The values returned are: -1 for error, 0 for timeout, 1
 * for when timeout is 0 and an rpc request was serviced.
 *
 * Waits with poll() on the RPC package's own descriptor array
 * (svc_pollfd), the descriptors in efds and the wakeup pipe.  (This
 * used to rebuild an fd_set and select(FD_SETSIZE) on every wakeup,
 * which also could not watch a descriptor numbered FD_SETSIZE or
 * more.)
 */
#if defined(OPT_TIRPC) || defined(__GLIBC__)
_Tt_rpcsrv_err _Tt_rpc_server::
run_until(int *stop, int timeout, _Tt_int_rec_list_ptr &efds)
{
	static pollfd		*pfds = 0;
	static int		pfds_size = 0;
	int			fd;
	int			done = 0;
	int			poll_stat;
	_Tt_rpcsrv_err		status = _TT_RPCSRV_OK;
	timespec		deadline;

	if (timeout >= 0) {
		clock_gettime(CLOCK_MONOTONIC, &deadline);
		deadline.tv_sec += timeout;
	}
	_Tt_int_rec_list_cursor	efds_c(efds);
	do {
		// The RPC descriptors come first, at the same indices
		// as in svc_pollfd, as svc_getreq_poll() expects.
		int nrpc = svc_max_pollfd;
		int need = nrpc + efds->count() + 1;
		if (need > pfds_size) {
			pollfd *np = (pollfd *)realloc(pfds,
						       need * sizeof(pollfd));
			if (np == 0) {
				return(_TT_RPCSRV_ERR);
			}
			pfds = np;
			pfds_size = need;
		}
		int n = 0;
		for (; n < nrpc; n++) {
			pfds[n].fd = svc_pollfd[n].fd;
			pfds[n].events = svc_pollfd[n].events;
			pfds[n].revents = 0;
		}
		efds_c.reset();
		while (efds_c.next()) {
			fd = efds_c->val;
			// NOTE that it is crucially important that fd 0
			// not be watched.  fd 0 (stdin) is always set
			// to /dev/null, which is always active.
			// The reason fd 0 is in efds at all is that 
			// _Tt_self_procid uses it as a dummy entry
			// for ttsession itself, which doesn\'t need a
			// signalling channel.
			// Negative entries represent signalling
			// channels that were found active on a
			// previous pass but are not yet cleared out.
			if (fd > 0) {
				pfds[n].fd = fd;
				pfds[n].events = POLLIN;
				pfds[n].revents = 0;
				n++;
			}
		}
		int wake_idx = -1;
		if (_tt_wake_pipe[0] >= 0) {
			wake_idx = n;
			pfds[n].fd = _tt_wake_pipe[0];
			pfds[n].events = POLLIN;
			pfds[n].revents = 0;
			n++;
		}

		int ms = -1;
		if (timeout >= 0) {
			timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			long long left =
				(long long)(deadline.tv_sec - now.tv_sec) * 1000
				+ (deadline.tv_nsec - now.tv_nsec) / 1000000;
			ms = (left > 0) ? (int)left : 0;
		}

		// Drop the global mutex around any polling or RPC calls.
		
		_tt_global->drop_mutex();
		
		poll_stat = poll(pfds, n, ms);

		_tt_global->grab_mutex();

		switch (poll_stat) {
		      case -1:
			return(_TT_RPCSRV_ERR);
		      case 0:
			return(_TT_RPCSRV_TMOUT);
		      default:
			if (wake_idx >= 0 && pfds[wake_idx].revents) {
				char	buf[64];
				while (read(_tt_wake_pipe[0], buf,
					    sizeof(buf)) > 0) {
				}
			}
			// check for exception fds
			int i = nrpc;
			efds_c.reset();
			while (efds_c.next()) {
				fd = efds_c->val;
				if (fd <= 0) continue;	// not watched
				// Readable, hung up, in error or not
				// open: in each case the connection is
				// gone (select() reported all of these
				// as readable or failed outright).
				if (pfds[i].revents) {
					efds_c->val = (0 - fd);
					status = _TT_RPCSRV_FDERR;
					done = 1;
				}
				i++;
			}
			int nready = 0;
			for (i = 0; i < nrpc; i++) {
				if (pfds[i].revents) {
					nready++;
				}
			}
			if (nready > 0) {
				svc_getreq_poll(pfds, nready);
			}
		}
	} while ((! done) && ((stop == 0) || (! *stop)));
	return status;
}
#else				/* no svc_pollfd: select() */
_Tt_rpcsrv_err _Tt_rpc_server::
run_until(int *stop, int timeout, _Tt_int_rec_list_ptr &efds)
{
	fd_set			readfds;
	timeval			tmout;
	int			fd;
	int			done = 0;
	int			select_stat;
	_Tt_rpcsrv_err		status = _TT_RPCSRV_OK;

	tmout.tv_sec = timeout;
	tmout.tv_usec = 0;
	_Tt_int_rec_list_cursor	efds_c(efds);
	do {
		// Add our fd's to a copy of the rpc fdset.
		readfds = svc_fdset;
		efds_c.reset();
		while (efds_c.next()) {
			fd = efds_c->val;
			// NOTE that it is crucially important that the bit
			// for fd 0 not be set.  fd 0 (stdin) is always set
			// to /dev/null, which is always active.
			// The reason fd 0 is in efds at all is that 
			// _Tt_self_procid uses it as a dummy entry
			// for ttsession itself, which doesn\'t need a
			// signalling channel.

			// I haven\'t verified this, but I bet it\'s possible
			// for negative entries to be in the efds list too,
			// representing signalling channels that were found
			// active on a previous pass but are not yet cleared
			// out.
			if (fd > 0) {
				FD_SET(fd, &readfds);
			}
		}

		if (_tt_wake_pipe[0] >= 0 && _tt_wake_pipe[0] < FD_SETSIZE) {
			FD_SET(_tt_wake_pipe[0], &readfds);
		}

		// Drop the global mutex around any polling or RPC calls.
		
		_tt_global->drop_mutex();
		
		select_stat = 
			select(FD_SETSIZE,&readfds, 0, 0,
			       (timeout >= 0) ? &tmout : (timeval *)0);

		_tt_global->grab_mutex();

		switch (select_stat) {
		      case -1:
			return(_TT_RPCSRV_ERR);
		      case 0:
			return(_TT_RPCSRV_TMOUT);
		      default:
			if (_tt_wake_pipe[0] >= 0
			    && _tt_wake_pipe[0] < FD_SETSIZE
			    && FD_ISSET(_tt_wake_pipe[0], &readfds)) {
				char	buf[64];
				while (read(_tt_wake_pipe[0], buf,
					    sizeof(buf)) > 0) {
				}
				FD_CLR(_tt_wake_pipe[0], &readfds);
			}
			// check for exception fds
			efds_c.reset();
			while (efds_c.next()) {
				fd = efds_c->val;
				if (fd < 0) continue;	// -1 => not valid fd
				if (FD_ISSET(fd, &readfds)) {
					efds_c->val = (0 - fd);
					status = _TT_RPCSRV_FDERR;
					done = 1;
				}

				// Clear our fd from the fdset so
				// svc_getreqset() won't get confused (bug
				// 2000972).

				FD_CLR(fd, &readfds);
			}
			svc_getreqset(&readfds);
		}
	} while ((! done) && ((stop == 0) || (! *stop)));
	return status;
}

#endif


#ifndef OPT_TLI
// pmap_getport() takes the address of the portmapper to ask (it only
// sets the port); these used to be passed uninitialized.
static void
_tt_local_pmap_addr(sockaddr_in *a)
{
	memset(a, 0, sizeof(*a));
	a->sin_family = AF_INET;
	a->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}


/*
 * A ttsession that is killed (or crashes) leaves its transient program
 * registered with the portmapper.  Only MAX_TRANS_RANGE numbers either
 * side of 0x4fffffff are probed, so enough of those stale entries stop
 * any further ttsession from starting.  Returns 1 if the registration
 * of prognum (versions vers and 1) at TCP port "port" is stale, i.e.
 * nothing accepts connections on that local port, and could be removed.
 * A port that accepts, or that does not answer within half a second,
 * is taken to be live.  (ttsession listens before it registers.)
 */
static int
_tt_reclaim_stale_prognum(int prognum, int vers, int port)
{
	int		s;
	int		dead = 0;
	sockaddr_in	a;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		return 0;
	}
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);
	memset(&a, 0, sizeof(a));
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (connect(s, (sockaddr *)&a, sizeof(a)) < 0) {
		if (errno == ECONNREFUSED) {
			dead = 1;
		} else if (errno == EINPROGRESS) {
			pollfd		p;
			int		err = 0;
			socklen_t	len = sizeof(err);

			p.fd = s;
			p.events = POLLOUT;
			p.revents = 0;
			if (   poll(&p, 1, 500) == 1
			    && getsockopt(s, SOL_SOCKET, SO_ERROR,
					  (char *)&err, &len) == 0
			    && err == ECONNREFUSED) {
				dead = 1;
			}
		}
	}
	close(s);
	if (! dead) {
		return 0;
	}
	int		v1port = 0;
	if (vers != 1) {
		_tt_local_pmap_addr(&a);
		v1port = pmap_getport(&a, prognum, 1, IPPROTO_TCP);
	}
	if (! pmap_unset(prognum, vers)) {
		return 0;	// e.g. registered by another user
	}
	if (v1port == port) {
		pmap_unset(prognum, 1);
	}
	_tt_syslog(0, LOG_WARNING,
		   "removed stale portmapper registration of program "
		   "%d (port %d)", prognum, port);
	return 1;
}
#endif				/* !OPT_TLI */


/* 
 * Returns an unused transient program number. Definition taken out of
 * the RPC manual.
 */
#ifdef OPT_TLI
static int
gettransient(int vers, netconfig *nconf, netbuf *address)
#else
static int
gettransient(int proto, int vers, int *sockp)
#endif				/* OPT_TLI */
{
	int			prognum;

#ifndef OPT_TLI
	int			s;
#if defined(__linux__) || defined(CSRG_BASED)
	socklen_t		len;
#else
	int			len;
#endif
	int			socktype;
	sockaddr_in		addr;
	sockaddr_in		tport;
	sockaddr_in		uport;

	switch (proto) {
	      case IPPROTO_UDP:
		socktype = SOCK_DGRAM;
		break;
	      case IPPROTO_TCP:
		socktype = SOCK_STREAM;
		break;
	      default:
		return(0);
	}
	if (*sockp == RPC_ANYSOCK) {
		s = socket(AF_INET, socktype, 0);
		if (s < 0) {
			_tt_syslog(0, LOG_ERR, "gettransient(): socket(): %m");
			return 0;
		}
		*sockp = s;
	} else {
		s = *sockp;
	}
        memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(0);
        addr.sin_family = AF_INET;
	len = sizeof(addr);
	if(bind(s, (sockaddr *)&addr, len) == -1) {
		_tt_syslog(0, LOG_ERR, "bind(): %m");
		return(0);
	}
	if (getsockname(s, (sockaddr *)&addr, &len) < 0) {
		_tt_syslog(0, LOG_ERR, "getsockname(): %m");
		return(0);
	}

#if !defined(__linux__)
	int optval = 0;
	if (setsockopt(s, SOL_SOCKET, SO_USELOOPBACK,
		       (char *)&optval, sizeof(optval)) == -1) {
	}
#endif
#endif				/* !OPT_TLI */


	// Search for a transient rpc number in the range 0x40000000 -
	// 0x5fffffff by starting in the middle of the range searching
	// up and then searching down if that fails. The reason for
	// this is to make it less likely for other programs to grab
	// this transient number (since pmap_getport doesn't complain
	// if you try to grab a number for udp and we have it grabbed
	// for tcp).

        // JET - this is way too many pnums to search, and causes what
        // appears to be an infinite loop (though it isn't) if the
        // user is running an rpcbind in secure mode and not using
        // libtirpc - a common error.  The end result is staring at
        // the dthello welcome screen.  So - rather than search this
        // immense space, we will only search from start to +-50
        // before bailing.  If a hundred attmepts to get a transient
        // fail, I don't see that doing approximately 537 million
        // attempts are worth it :)
#define MAX_TRANS_RANGE 50

#ifndef OPT_TLI
	// Listen before registering: a concurrent ttsession that finds
	// our registration while we are still starting up must see a
	// live port, not a refused one that it would take for stale
	// (see _tt_reclaim_stale_prognum()).  svctcp_create() and the
	// listen() in _Tt_rpc_server::init() leave this as it is.
	listen(s, 5);

	// Probe 0x4fffffff up to 0x4fffffff + MAX_TRANS_RANGE, then
	// 0x4ffffffe down to 0x4ffffffe - MAX_TRANS_RANGE.
	for (int dir = 0; dir < 2; dir++) {
		int first = dir ? 0x4ffffffe : 0x4fffffff;
		for (int i = 0; i <= MAX_TRANS_RANGE; i++) {
			prognum = dir ? first - i : first + i;
			/* XXX: pmap_set allows the same prognum for	*/
			/* different protocols so we hack around that	*/
			/* by requiring both tcp and udp to be free.	*/
			_tt_local_pmap_addr(&uport);
			_tt_local_pmap_addr(&tport);
			int uport_n = pmap_getport(&uport, prognum, vers,
						   IPPROTO_UDP);
			int tport_n = pmap_getport(&tport, prognum, vers,
						   proto);
			if (uport_n == 0 && tport_n != 0 &&
			    _tt_reclaim_stale_prognum(prognum, vers,
						      tport_n)) {
				tport_n = 0;
			}
			if (uport_n != 0 || tport_n != 0) {
				continue;
			}
			// Register version 1 as well, as _Tt_rpc_server::
			// init() serves it (the downward search used to
			// register only vers).
			if (pmap_set(prognum, vers, proto,
				     ntohs(addr.sin_port))) {
				if (vers == 1 ||
				    pmap_set(prognum, 1, proto,
					     ntohs(addr.sin_port))) {
					return(prognum);
				}
				pmap_unset(prognum, vers);
			}
		}
	}
#else
	// search up in the range 0x4fffffff to 0x4fffffff +  MAX_TRANS_RANGE
	for (prognum = 0x4fffffff; prognum <= (0x4fffffff + MAX_TRANS_RANGE); prognum++) {
		if (rpcb_set(prognum, vers, nconf, address) &&
		    (vers==1 || rpcb_set(prognum, 1, nconf, address))) {
			return(prognum);
		}
	}

	// search down in the range 0x4ffffffe - 0x40000000
	for (prognum = 0x4ffffffe; prognum >= (0x4ffffffe - MAX_TRANS_RANGE); prognum--) {
		if (rpcb_set(prognum, vers, nconf, address)) {
			return(prognum);
		}
	}
#endif				/* !OPT_TLI */

	return(0);
}




