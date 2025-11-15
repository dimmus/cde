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
/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: mp_rpc_fns.C /main/4 1996/04/21 19:12:28 drk $ 			 				 */
/*
 * mp_rpc_fns.C
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <tt_options.h>
#include <rpc/rpc.h>

#if defined(OPT_TLI)

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <tiuser.h>
#include <sys/socket.h>

int
tli_setopt(int fd, int level, int name, int value)
{
	struct t_optmgmt req, resp;
	struct {
		struct opthdr opt;
		int value;
	} reqbuf;

	reqbuf.opt.level = level;
	reqbuf.opt.name = name;
	reqbuf.opt.len = sizeof (int);

	reqbuf.value = value;

	req.flags = T_NEGOTIATE;
	req.opt.len = sizeof (reqbuf);
	req.opt.buf = (char *)&reqbuf;

	resp.flags = 0;
	resp.opt.buf = (char *)&reqbuf;
	resp.opt.maxlen = sizeof (reqbuf);

	if (t_optmgmt(fd, &req, &resp) < 0 || resp.flags != T_SUCCESS) {
		t_error("t_optmgmt");
		return (0);
	}
        return (1);
}


/* 
 * Sets the TCP_NODELAY option for a tli stream bound using tcp. This
 * option is crucial to good message performance since otherwise there is
 * a 200 ms delay between messages. This code is based on the source code
 * for tli rpc (since documentation was sorely lacking).
 */
extern "C" 
int
_tt_tli_set_nodelay(int fd)
{
	return (tli_setopt(fd, IPPROTO_TCP, TCP_NODELAY, 1));
}

extern "C" 
int
_tt_bind_endpoint(int	fd,
		  char	*s,
		  char	*r)
{
	return(t_bind(fd, (struct t_bind *)s, (struct t_bind *)r));
}
#endif				/* OPT_TLI */


/* 
 *   XXX: the following are needed because of bugs in the C++ header files
 *   for these functions. Should use the native definitions when the header
 *   files are fixed.
 */

extern "C" 
void
_tt_svc_freeargs(SVCXPRT	*transp,
		 xdrproc_t	inproc,
		 char		*in)
{
	svc_freeargs(transp, inproc, in);
}

extern "C" 
int
_tt_svc_getargs(SVCXPRT		*transp,
		xdrproc_t	inproc,
		char		*in)
{
	return(svc_getargs(transp, inproc, in));
}
