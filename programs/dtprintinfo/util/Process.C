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
/* $XConsortium: Process.C /main/3 1996/10/01 16:10:01 drk $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "Process.h"
#include "Invoke.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

Process::Process()
{
   procs = NULL;
   pprocs = NULL;
   NumProcs = 0;
   last_pid = -1;
   last_proc = NULL;
#ifdef __linux__
   // Only a few ancestors are ever looked up; read them from /proc rather
   // than running "ps -el | awk" over every process on the system.
   use_proc = (access("/proc/self/stat", R_OK) == 0 &&
	       access("/proc/self/status", R_OK) == 0);
   if (use_proc)
      return;
#endif
#ifdef aix
   (void)Invoke("ps -e -F \"pid ppid uid command\"", &procs);
#elif defined(__FreeBSD__)
   (void)Invoke("/bin/ps ax -o pid,ppid,uid,comm", &procs);
#else
   (void)Invoke("/bin/ps -el | awk '{printf(\"%s %s %s %s\\n\",$4,$5,$3,$NF)}'",
		&procs);
#endif
   pprocs = (char **)malloc(sizeof(char *));
   NumProcs = 0;
   strtok(procs, "\n");
   while((pprocs[NumProcs] = strtok(NULL, "\n")))
    {
      NumProcs++;
      pprocs = (char **)realloc(pprocs, sizeof(char *) * (NumProcs + 1));
    }
   last_pid = -1;
}

Process::~Process()
{
   free(procs);
   free(pprocs);
}

#ifdef __linux__
// The line "ps -el | awk '{printf("%s %s %s %s\n",$4,$5,$3,$NF)}'" prints
// for _pid: "PID PPID UID COMMAND", where UID is the effective uid and
// COMMAND the last word of the command name.
char *Process::ReadProc(pid_t _pid)
{
   char path[64], buf[512];
   FILE *fp;
   size_t n;

   snprintf(path, sizeof(path), "/proc/%ld/stat", (long)_pid);
   if (!(fp = fopen(path, "r")))
      return NULL;
   n = fread(buf, 1, sizeof(buf) - 1, fp);
   fclose(fp);
   buf[n] = '\0';

   // "PID (COMMAND) STATE PPID ..."; COMMAND may contain ')' and spaces
   char *open_paren = strchr(buf, '(');
   char *close_paren = strrchr(buf, ')');
   long long_ppid;
   char state;
   if (!open_paren || !close_paren || close_paren < open_paren ||
       sscanf(close_paren + 1, " %c %ld", &state, &long_ppid) != 2)
      return NULL;
   *close_paren = '\0';
   char *command = open_paren + 1;
   char *word;
   // awk's $NF: the last whitespace separated word
   while (*command == ' ' || *command == '\t')
      command++;
   for (word = command + strlen(command); word > command &&
	(word[-1] == ' ' || word[-1] == '\t'); word--)
      word[-1] = '\0';
   word = strrchr(command, ' ');
   if (word)
      command = word + 1;

   snprintf(path, sizeof(path), "/proc/%ld/status", (long)_pid);
   if (!(fp = fopen(path, "r")))
      return NULL;
   long long_uid = -1;
   char line[256];
   while (fgets(line, sizeof(line), fp))
      if (!strncmp(line, "Uid:", 4))
       {
	 long real_uid;
	 if (sscanf(line + 4, "%ld %ld", &real_uid, &long_uid) != 2)
	    long_uid = -1;
	 break;
       }
   fclose(fp);
   if (long_uid == -1)
      return NULL;

   pid = _pid;
   ppid = (pid_t)long_ppid;
   uid = (uid_t)long_uid;
   snprintf(proc_line, sizeof(proc_line), "%ld %ld %ld %s", (long)_pid,
	    long_ppid, long_uid, command);
   return proc_line;
}
#endif

char *Process::GetByPid(pid_t _pid)
{
   int i;

   if (last_pid == _pid)
      return last_proc;
#ifdef __linux__
   if (use_proc)
    {
      if (_pid <= 0 || !ReadProc(_pid))
	 return NULL;
      last_pid = _pid;
      last_proc = proc_line;
      return proc_line;
    }
#endif
   if (_pid)
      for (i = 0; i < NumProcs; i++)
       {
	 long long_pid, long_ppid, long_uid;
	 sscanf(pprocs[i], "%ld %ld %ld", &long_pid, &long_ppid, &long_uid);
	 pid = (pid_t)long_pid;
	 ppid = (pid_t)long_ppid;
	 uid = (uid_t)long_uid;

         if (_pid == pid)
	  {
	     last_pid = _pid;
	     last_proc = pprocs[i];
	     return pprocs[i];
	  }
       }
   return NULL;
}

pid_t Process::Parent(pid_t pid)
{
   char *proc = GetByPid(pid);
   if (proc)
      return ppid;
   else
      return (pid_t)-1;
}

uid_t Process::UID(pid_t pid)
{
   char *proc = GetByPid(pid);
   if (proc)
      return uid;
   else
      return (uid_t)-1;
}

char *Process::Command(pid_t _pid)
{
   char *proc = GetByPid(_pid);
   if (proc)
    {
      char *s;
      // Find first field
      for (s = proc; *s == ' '; s++)
	 ;
      for ( ; *s != ' '; s++)
	 ;
      // Find second field
      for ( ; *s == ' '; s++)
	 ;
      for ( ; *s != ' '; s++)
	 ;
      // Find third field
      for ( ; *s == ' '; s++)
	 ;
      for ( ; *s != ' '; s++)
	 ;
      // Find fourth field
      for ( ; *s == ' '; s++)
	 ;
      return s;
    }
   else
      return NULL;
}
