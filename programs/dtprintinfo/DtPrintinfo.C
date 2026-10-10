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
/* $TOG: DtPrintinfo.C /main/6 1998/07/24 16:10:28 mgreess $ */
// User Interface headers
#include "DtApp.h"
#include "DtPrinterIcon.h"
#include "Invoke.h"

#ifndef NO_CDE
extern "C" {
  #include <Dt/EnvControlP.h>
}
#endif


// Object headers
#include "PrintSubSys.h"

// Message header
#include "dtprintinfomsg.h"
nl_catd dtprintinfo_cat = NULL;

#include <stdlib.h> // This is for the getenv function
#include <unistd.h> // This is for the getuid function
#include <string.h> 
#include <errno.h> 

#if defined(aix)
extern "C" { extern int seteuid(uid_t); }
#endif

int main(int argc, char **argv)
{
#ifndef NO_CDE
   _DtEnvControl(DT_ENV_SET);
#endif

// run as user's UID
   seteuid(getuid());

   setlocale(LC_ALL, "");

   char *lang = getenv("LANG");
   if (lang && strcmp(lang, "C"))
    {
      errno = 0;

#ifdef NL_CAT_LOCALE
      dtprintinfo_cat = CATOPEN("dtprintinfo", NL_CAT_LOCALE);
#else
      dtprintinfo_cat = CATOPEN("dtprintinfo", 0);
#endif

      if (errno)
         dtprintinfo_cat = (nl_catd) -1;
    }

   if (dtprintinfo_cat == NULL) {
      dtprintinfo_cat = (nl_catd) -1;
   }

   if (!STRCMP(argv[1], "-help"))
    {
      // Same as piping the queue list through awk '{print "\t", $1}'
      char *queues = ListPrintQueues();
      size_t n_lines = 0;
      char *s;
      for (s = queues; (s = strchr(s, '\n')); s++)
         n_lines++;
      char *output = (char *) malloc(strlen(queues) + 2 * n_lines + 1);
      char *o = output;
      char *line, *next;
      for (line = queues; output && line && *line; line = next)
       {
         if ((next = strchr(line, '\n')))
            *next++ = '\0';
         char *end;
         while (*line == ' ' || *line == '\t')
            line++;
         for (end = line; *end && *end != ' ' && *end != '\t'; end++)
            ;
         *o++ = '\t';
         *o++ = ' ';
         memcpy(o, line, end - line);
         o += end - line;
         *o++ = '\n';
       }
      if (output)
         *o = '\0';
      printf(MESSAGE(CommandLineHelpL), output ? output : "");
      printf("\n");
      free(output);
      free(queues);
      return 0;
    }

   char *progname = strrchr(argv[0], '/');
   if (progname)
      progname++;
   else
      progname = argv[0];
   if (!STRCMP(argv[1], "-populate"))
    {
      if (getuid() != 0)
       {
	 fprintf(stderr, MESSAGE(RootUserL), progname, "-populate");
	 fprintf(stderr, "\n");
	 return 1;
       }

      PrintSubSystem *prt = new PrintSubSystem(NULL);
      int n_queues = prt->NumChildren();
      // Get Print Subsystem children, (these are queues)
      Queue **queues = (Queue **)prt->Children();
      int i;
      for (i = 0; i < n_queues; i++)
       {
         DtPrinterIcon *icon = new DtPrinterIcon(NULL, NULL, queues[i],
						 INITIALIZE_PRINTERS);
	 icon->CreateActionFile();
	 delete icon;
       }
      return 0;
    }

   DtApp *app = new DtApp(progname, &argc, argv);
   app->Visible(true);
   app->Run();

   return 0;
}
