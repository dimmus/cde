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
/* $TOG: Invoke.C /main/7 1997/07/30 15:42:39 samborn $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */


#include "Invoke.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

const int BUFFER_SIZE = 512;

// Growable, always NUL-terminated buffer for one output stream of the child.
struct InvokeBuffer
{
   char *data;
   size_t len;
   size_t size;
};

static int InvokeBufferInit(InvokeBuffer *buf)
{
   buf->len = 0;
   buf->size = BUFFER_SIZE;
   if (!(buf->data = (char *) malloc(buf->size)))
      return -1;
   *buf->data = '\0';
   return 0;
}

// Read what is available on fd. Returns 1 while the stream is open, 0 at
// end of file or on a read error, -1 if memory ran out.
static int InvokeBufferRead(InvokeBuffer *buf, int fd)
{
   for (;;)
    {
      if (buf->size - buf->len < BUFFER_SIZE / 2)
       {
	 char *data = (char *) realloc(buf->data, buf->size * 2);
	 if (!data)
	    return -1;
	 buf->data = data;
	 buf->size *= 2;
       }
      ssize_t n = read(fd, buf->data + buf->len, buf->size - buf->len - 1);
      if (n > 0)
       {
	 buf->len += n;
	 buf->data[buf->len] = '\0';
	 continue;
       }
      if (n < 0 && errno == EINTR)
	 continue;
      if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
	 return 1;
      return 0;
    }
}

Invoke::Invoke(const char *command,   // Command to Run
               char **out_ptr,        // ptr to output buffer ptr 
               char **err_ptr,        // ptr to error buffer ptr
               uid_t _uid)            // run command as this UID
{
   int m_stdout[2] = { -1, -1 }, m_stderr[2] = { -1, -1 };
   InvokeBuffer out = { NULL, 0, 0 }, err = { NULL, 0, 0 };
   pid_t c_pid;
   pid_t w;
   int wstatus = 0;
   int trap_out = (out_ptr != NULL);
   int trap_err = (err_ptr != NULL);

   struct sigaction action;            // parameters of sigaction 
   struct sigaction oldsigint_act;
   struct sigaction oldsigquit_act;

   status = 0;

   // The callers always get a string back when they asked for one, even
   // when the command could not be run.
   if (trap_out)
    {
      *out_ptr = NULL;
      if (InvokeBufferInit(&out) < 0 || pipe(m_stdout) < 0)
	 goto fail;
    }
   if (trap_err)
    {
      *err_ptr = NULL;
      if (InvokeBufferInit(&err) < 0 || pipe(m_stderr) < 0)
	 goto fail;
    }

   // ignore these signals
   memset(&action, '\0', sizeof (struct sigaction));
   memset(&oldsigquit_act, '\0', sizeof (struct sigaction));
   memset(&oldsigint_act, '\0', sizeof (struct sigaction));

   action.sa_handler = SIG_IGN;

   sigaction(SIGINT, &action, &oldsigint_act);
   sigaction(SIGQUIT, &action, &oldsigquit_act);

   if ((c_pid = fork()) == 0)
    { // ------------------------ child process --------------------------

      if (_uid != (uid_t)-1 && setuid(_uid) != 0)
	 _exit(127);

      if (trap_out)
       { // duplicate stdout
         close(m_stdout[0]);
         dup2(m_stdout[1], 1);
         close(m_stdout[1]);
       }

      if (trap_err)
       { // duplicate stderr
         close(m_stderr[0]);
         dup2(m_stderr[1], 2);
         close(m_stderr[1]);
       }

      // start the program 
      execlp(KORNSHELL, "ksh", "-c", command, (char *) 0);

      _exit(-1);
    }

   // -------------------------- parent process --------------------------

   // restore signals
   sigaction(SIGINT, &oldsigint_act, NULL);
   sigaction(SIGQUIT, &oldsigquit_act, NULL);

   if (c_pid == -1)
      goto fail;

   // close the write side of the pipes for the parent
   if (trap_out)
    {
      close(m_stdout[1]);
      m_stdout[1] = -1;
      fcntl(m_stdout[0], F_SETFL, O_NONBLOCK);
    }
   if (trap_err)
    {
      close(m_stderr[1]);
      m_stderr[1] = -1;
      fcntl(m_stderr[0], F_SETFL, O_NONBLOCK);
    }

   while (trap_out || trap_err)
    {
      struct pollfd fds[2];
      int nfds = 0;

      if (trap_out)
       {
	 fds[nfds].fd = m_stdout[0];
	 fds[nfds].events = POLLIN;
	 nfds++;
       }
      if (trap_err)
       {
	 fds[nfds].fd = m_stderr[0];
	 fds[nfds].events = POLLIN;
	 nfds++;
       }
      if (poll(fds, nfds, -1) < 0)
       {
	 if (errno == EINTR)
	    continue;
	 status = -1;
	 break;
       }
      int i;
      for (i = 0; i < nfds; i++)
       {
	 if (!fds[i].revents)
	    continue;
	 bool is_out = (trap_out && fds[i].fd == m_stdout[0]);
	 int rc = InvokeBufferRead(is_out ? &out : &err, fds[i].fd);
	 if (rc < 0)
	    status = -1;
	 if (rc <= 0)
	  {
	    if (is_out)
	       trap_out = 0;
	    else
	       trap_err = 0;
	  }
       }
      if (status == -1)
	 break;
    }

   // Close the read sides before reaping, so that a child still writing
   // gets EPIPE instead of blocking forever.
   if (m_stdout[0] != -1)
      close(m_stdout[0]);
   if (m_stderr[0] != -1)
      close(m_stderr[0]);
   m_stdout[0] = m_stderr[0] = -1;

   // Reap only our own child: wait() could steal a child that an
   // asynchronous reader (MotifThread) is waiting for.
   while ((w = waitpid(c_pid, &wstatus, 0)) == -1 && errno == EINTR)
      ;
   if (status == 0)
      status = (w == c_pid) ? ((wstatus >> 8) & 0xFF) : -1;

   if (out_ptr)
      *out_ptr = out.data;
   if (err_ptr)
      *err_ptr = err.data;
   return;

fail:
   status = -1;
   int i;
   for (i = 0; i < 2; i++)
    {
      if (m_stdout[i] != -1)
	 close(m_stdout[i]);
      if (m_stderr[i] != -1)
	 close(m_stderr[i]);
    }
   if (out_ptr)
      *out_ptr = out.data ? out.data : strdup("");
   if (err_ptr)
      *err_ptr = err.data ? err.data : strdup("");
}
