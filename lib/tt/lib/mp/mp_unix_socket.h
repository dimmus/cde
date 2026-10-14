/*
 * CDE - Common Desktop Environment
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
/* -*-C++-*-
 *
 * mp_unix_socket.h
 *
 * The local (AF_UNIX) transport for ttsession RPCs.  ttsession serves
 * its RPCs over TCP, as always, and also on a stream socket in a
 * directory that belongs to its user alone:
 *
 *	$XDG_RUNTIME_DIR/tooltalk/<ttsession pid>.<RPC program number>
 *
 * Both numbers are in the session address, so a client on the same
 * host and of the same user finds the socket without any change to
 * the address.  Each side checks the other end with SO_PEERCRED: the
 * client that the socket is served by that ttsession process (pid and
 * uid), ttsession that the client runs as its user.  Where any of it is
 * missing (no XDG_RUNTIME_DIR, a directory that is not the user's own
 * with mode 0700, no SO_PEERCRED, an older ttsession, TT_UNIX_SOCKET=0
 * in the environment) clients use TCP, as before.
 *
 * Inline, as ttsession (slib) needs it and libtt exports only its API.
 */
#ifndef _MP_UNIX_SOCKET_H
#define _MP_UNIX_SOCKET_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#if defined(SO_PEERCRED) && defined(__linux__)
#define _TT_HAVE_UNIX_SOCKET_RPC 1
#endif

// The RPC program number of a session that only listens on its local
// socket (no portmapper to hand out a transient one; the socket path
// tells such sessions apart by pid).  The first transient number that
// a TCP session tries, so that a client of any vintage takes it for a
// ttsession's.
#define _TT_LOCAL_RPC_PROGRAM	0x4fffffff

#ifdef _TT_HAVE_UNIX_SOCKET_RPC

// A directory that is the effective user's alone: a directory, not a
// symbolic link, owned by the effective uid, with no group or other
// permissions.
static inline int
_tt_unix_socket_private_dir(const char *dir)
{
	struct stat	st;

	return (lstat(dir, &st) == 0 &&
		S_ISDIR(st.st_mode) &&
		st.st_uid == geteuid() &&
		(st.st_mode & 077) == 0);
}

// Sets path (a sockaddr_un sun_path) to the socket of the ttsession
// with the given pid and RPC program number.  create: make the
// tooltalk directory if need be (ttsession).  Returns 0 if there is no
// such private place, or the transport is turned off.
static inline int
_tt_unix_socket_path(char *path, size_t size, long pid, int program,
		     int create)
{
	const char	*off = getenv("TT_UNIX_SOCKET");
	const char	*run = getenv("XDG_RUNTIME_DIR");
	char		dir[sizeof(((struct sockaddr_un *)0)->sun_path)];
	int		n;

	if (off != 0 && strcmp(off, "0") == 0) {
		return 0;
	}
	if (run == 0 || run[0] != '/' || ! _tt_unix_socket_private_dir(run)) {
		return 0;
	}
	n = snprintf(dir, sizeof(dir), "%s/tooltalk", run);
	if (n < 0 || (size_t)n >= sizeof(dir)) {
		return 0;
	}
	if (create && mkdir(dir, 0700) != 0 && errno != EEXIST) {
		return 0;
	}
	if (! _tt_unix_socket_private_dir(dir)) {
		return 0;
	}
	n = snprintf(path, size, "%s/%ld.%d", dir, pid, program);
	return (n > 0 && (size_t)n < size);
}

// The uid (and pid) at the other end of a connected AF_UNIX socket.
static inline int
_tt_unix_socket_peer(int sock, uid_t *uid, pid_t *pid)
{
	struct ucred	cred;
	socklen_t	len = sizeof(cred);

	if (getsockopt(sock, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0 ||
	    len != sizeof(cred)) {
		return 0;
	}
	*uid = cred.uid;
	*pid = cred.pid;
	return 1;
}

#endif	/* _TT_HAVE_UNIX_SOCKET_RPC */

#endif	/* _MP_UNIX_SOCKET_H */
