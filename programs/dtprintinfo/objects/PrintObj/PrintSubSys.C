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
/* $XConsortium: PrintSubSys.C /main/4 1996/01/17 18:02:47 lehors $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "PrintSubSys.h"
#include "Queue.h"
#include "Invoke.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef aix
const char *LIST_QUEUES = "lsallq | grep -v '^bsh$' | sort";
#else
// "device for NAME: URI" lines; the names are extracted and sorted in
// ListPrintQueues() rather than by a nawk | sort pipeline.
const char *LIST_QUEUES = "LANG=C LC_ALL=C lpstat -v";

static int CompareQueueNames(const void *a, const void *b)
{
   return strcoll(*(char * const *)a, *(char * const *)b);
}
#endif

// Object Class Name
const char *PRINTSUBSYSTEM = "PrintSubSystem";

char *ListPrintQueues(int *status, char **device_list)
{
   char *std_out;
   Invoke cmd(LIST_QUEUES, &std_out);
   if (device_list)
      *device_list = NULL;
#ifdef aix
   if (status)
      *status = cmd.status;
   return std_out;
#else
   // The old "lpstat | nawk | sort" pipeline reported the status of sort,
   // so only a failure to run the command counts, not lpstat's status.
   if (status)
      *status = (cmd.status == -1) ? -1 : 0;
   if (device_list)
      *device_list = strdup(std_out);
   // Same result as
   //   nawk '$2 == "for" { x = match($3, /:/); print substr($3, 1, x-1) }'
   //   | sort
   // (sort in the user's collation order), one name per line.
   size_t n_names = 0, max_names = 16;
   char **names = (char **) malloc(max_names * sizeof(char *));
   char *line, *next;
   for (line = std_out; names && line && *line; line = next)
    {
      if ((next = strchr(line, '\n')))
	 *next++ = '\0';
      char *field[3];
      int nf = 0;
      char *s = line;
      while (nf < 3)
       {
	 while (*s && isspace((unsigned char)*s))
	    s++;
	 if (!*s)
	    break;
	 field[nf++] = s;
	 while (*s && !isspace((unsigned char)*s))
	    s++;
	 if (*s)
	    *s++ = '\0';
       }
      if (nf < 3 || strcmp(field[1], "for"))
	 continue;
      char *colon = strchr(field[2], ':');
      if (!colon || colon == field[2])
	 continue;
      *colon = '\0';
      if (n_names == max_names)
       {
	 char **tmp = (char **) realloc(names, 2 * max_names * sizeof(char *));
	 if (!tmp)
	    break;
	 names = tmp;
	 max_names *= 2;
       }
      names[n_names++] = field[2];
    }
   size_t i, len = 0;
   for (i = 0; i < n_names; i++)
      len += strlen(names[i]) + 1;
   char *result = (char *) malloc(len + 1);
   if (result)
    {
      char *r = result;
      if (n_names)
	 qsort(names, n_names, sizeof(char *), CompareQueueNames);
      for (i = 0; i < n_names; i++)
       {
	 size_t l = strlen(names[i]);
	 memcpy(r, names[i], l);
	 r += l;
	 *r++ = '\n';
       }
      *r = '\0';
    }
   free(names);
   free(std_out);
   return result ? result : strdup("");
#endif
}

PrintSubSystem::PrintSubSystem(BaseObj *parent)
	: BaseObj(parent, "PrintSubSystem")
{
   _device_list = NULL;
   _displayName = strdup(MESSAGE(PrinterMenuL));
   _details = strdup("Status   Number   Owner      Date       Time       Size");
}

PrintSubSystem::~PrintSubSystem()
{
   // Delete the queues while the device list they may use still exists
   DeleteChildren();
   free(_device_list);
}

void PrintSubSystem::InitChildren()
{
   int status;
   free(_device_list);
   char *std_out = ListPrintQueues(&status, &_device_list);
   if (status)
    {
      Error("InitChildren method could not list queues.");
    }
   else
    {
      char *queue = strtok(std_out, " \n");
      while (queue && *queue)
       {
         new Queue(this, queue);
         queue = strtok(NULL, " \n");
       }
    }
   free(std_out);
}
