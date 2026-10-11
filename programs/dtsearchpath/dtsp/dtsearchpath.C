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
/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

/*****************************************************************************
 *  File:              dtsearchpath
 *
 *  Purpose:           Set up the application search paths for 
 *                     the desktop components.
 *
 *  Description:       This client is invoked by the Xsession file
 *                     at user login.  It sets the local and remote locations
 *                     that the desktop components should look, for application
 *                     icons, help files, and other application configuration
 *                     information on behalf of the user.
 *
 *  Invoked by:        Xsession after the scripts in /etc/dt/Xsession.d
 *                     are sourced in.
 *
 *  Product:           @(#)Common Desktop Environment 1.0          
 *
 *  Revision:          $XConsortium: dtsearchpath.C /main/9 1996/07/10 16:38:27 rswiston $
 *
 ****************************************************************************/

/****************************************************************************
 *
 *  DTMOUNTPOINT 
 *     Description:  The directory where this system mounts remote discs.
 *                   Used for constructing pathnames to remote files, for
 *                   systems that are not using a global file name space.
 *     Default:      Vendor specific:  /nfs
 *
 *  DTAPPSEARCHPATH
 *     Description:  The directories where we will look for applications.
 *     Default:      $HOME/.dt/appmanager,
 *                   /etc/dt/appconfig/appmanager,
 *                   /usr/dt/appconfig/appmanager
 *
 *  DTDATABASESEARCHPATH
 *     Description:  The directories where we will look for actions/filetypes.
 *     Default:      Derived from DTAPPSEARCHPATH:
 *                       $HOME/.dt/types,
 *                       /etc/dt/appconfig/types/%L,
 *                       /usr/dt/appconfig/types/%L
 *
 *  DTHELPSEARCHPATH
 *     Description:  The directories where we will look for online help.
 *     Default:      Derived from $DTAPPSEARCHPATH:
 *                       $HOME/.dt/help/[%H | %H.hv]
 *                       /etc/dt/appconfig/help/%L/[%H | %H.sdl | %H.hv],
 *                       /usr/dt/appconfig/help/%L/[%H | %H.sdl | %H.hv]
 *
 *  DTINFOLIBSEARCHPATH
 *     Description:  The directories where we will look for online information.
 *     Default:      Derived from $DTAPPSEARCHPATH:
 *                       $HOME/.dt/infolib/%L/%I.dti,
 *                       /etc/dt/appconfig/infolib/%L/%I.dti,
 *                       /usr/dt/appconfig/infolib/%L/%I.dti
 *
 *  DTICONSEARCHPATH
 *     Description:  The directories where we will look for icons.
 *     Default:      Derived from $DTAPPSEARCHPATH:
 *                       $HOME/.dt/icons/%B,
 *                       /etc/dt/appconfig/icons/%L/[%B | %B.pm | %B.bm]
 *                       /usr/dt/appconfig/icons/%L/[%B | %B.pm | %B.bm]
 ****************************************************************************/

/****************************************************************************
 *    If you specify a value for one of the search paths, then that value
 *    will be added to the default search path after the sys-admin config-
 *    uration location but before the factory-default location.  For ex-
 *    ample, if you specify a value for the DTSPSYSAPPHOSTS of /foo, then
 *    the search path will be:
 *
 *      $HOME/.dt:/etc/dt/appconfig:/foo:/usr/dt/appconfig
 *
 *    Remember that you can also set the USER equivalents of these
 *    search paths.  They are useful for specifying locations other
 *    than the defaults.   Their names are:
 *
 *  DTSPUSERAPPHOSTS
 *     Default:  <null>
 *
 *  DTSPUSERDATABASEHOSTS
 *     Default:  <null>
 *
 *  DTSPUSERHELP
 *     Default:  <null>
 *
 *  DTSPUSERINFOLIB
 *     Default:  <null>
 *
 *  DTSPUSERICON
 *     Default:  <null>
 *
 *    These values will be prepended to the system paths, and hence will
 *    will take precedence.
 *
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "SearchPath.h"
#include "Options.h"
#include "Environ.h"

const char * DTINFOLIBDEFAULT_NAME = "DTINFOLIBDEFAULT";
const char * DTINFOLIBDEFAULT_VALUE = "cde";

Options * options;

/*
 * dtsearchpath -ksh -cache <file>: the output also goes to <file>, as a
 * ksh93 script for Xsession that sets dtsp_out to it if the exported
 * environment variables read and the results of the file tests made
 * are still the same.  Xsession then does "eval $dtsp_out", which is
 * what it does with the output of dtsearchpath, without running it.
 */
static EnvRecorder recorder;
static FILE * capture;
static int savedStdout = -1;

static void StartCapture()
{
  if (!options->cacheFile())
    return;
  // Only the plain ksh output, and only when it depends on nothing
  // but the environment and the file tests.
  if (!options->useKshEnv() || options->Verbose() ||
      options->CheckingUser() || options->getHomeDir())
    return;
  fflush(stdout);
  if (!(capture = tmpfile()))
    return;
  if ((savedStdout = dup(1)) < 0 || dup2(fileno(capture), 1) < 0) {
    if (savedStdout >= 0)
      close(savedStdout);
    savedStdout = -1;
    fclose(capture);
    capture = 0;
    return;
  }
  recorder.ignore("SHELL");	// set by -ksh
  envRecorder = &recorder;
}

static void WriteCache(const char * file, const std::string & out)
{
  std::string tmp(file);
  std::string dir(file, strrchr(file, '/') - file);
  std::string script;
  std::string value(out);
  size_t len;
  int fd;

  // As in $(...): no trailing newlines.
  while (!value.empty() && value[value.size() - 1] == '\n')
    value.erase(value.size() - 1);

  script = "# Written by dtsearchpath -cache: its -ksh output, for "
	   "Xsession (ksh93).\n"
	   "dtsp_out=\n"
	   "dtsp_x=\"\n$(typeset +x)\n\"\n"
	   "if " + recorder.kshCondition() + "\n"
	   "then\n"
	   "  dtsp_out=" + EnvRecorder::kshQuote(value) + "\n"
	   "fi\n"
	   "unset dtsp_x\n";

  if (!dir.empty() && mkdir(dir.c_str(), 0700) != 0 && errno != EEXIST)
    return;
  tmp += ".XXXXXX";
  if ((fd = mkstemp(&tmp[0])) < 0)
    return;
  len = script.size();
  if (write(fd, script.data(), len) != (ssize_t) len ||
      close(fd) != 0 || rename(tmp.c_str(), file) != 0)
    unlink(tmp.c_str());
}

static void EndCapture(char ** argv)
{
  std::string out;
  char buf[4096];
  size_t n;
  int failed;

  if (!capture)
    return;
  envRecorder = 0;
#ifdef IOSTREAMSWORKS
  std::cout.flush();
#endif
  failed = fflush(stdout) != 0 || ferror(stdout);
  dup2(savedStdout, 1);
  close(savedStdout);
  clearerr(stdout);
  rewind(capture);
  while (!failed && (n = fread(buf, 1, sizeof(buf), capture)) > 0)
    out.append(buf, n);
  failed = failed || ferror(capture);
  fclose(capture);
  capture = 0;
  if (failed) {
    // The temporary file could not hold the output (a full /tmp, say):
    // run again without -cache, writing straight to stdout.
    char ** args;
    int i, j;

    for (i = 0; argv[i]; i++)
      ;
    args = new char * [i + 1];
    for (i = j = 0; argv[i]; i++)
      if (strcmp(argv[i], "-cache") == 0 && argv[i + 1])
	i++;
      else
	args[j++] = argv[i];
    args[j] = 0;
    execv(args[0], args);
    execvp(args[0], args);
    perror(args[0]);
    exit(1);
  }
  if (!out.empty() && write(1, out.data(), out.size()) != (ssize_t) out.size())
    return;
  if (!recorder.spoiled() && !out.empty())
    WriteCache(options->cacheFile(), out);
}

int main (int argc, char **argv)
{
  CString newShell("SHELL="); // Keep in scope for duration of program.

  options = new Options(argc, argv);
  StartCapture();

  if ( options->useKshEnv() ) {
    newShell += UnixEnvironment::kshString();
    putenv(newShell.data());
  }
  else if ( options->useCshEnv() ) {
    newShell += UnixEnvironment::cshString();
    putenv(newShell.data());
  }

  UnixEnvironment * os = new UnixEnvironment;

  CDEEnvironment * user = new CDEEnvironment(options->getHomeDir(), os);
  user->CreateHomeAppconfigDir();

  if (options->removeAutoMountPoint())
    options->setAutoMountPoint(os->getEnvironmentVariable("DTAUTOMOUNTPOINT"));

  InfoLibSearchPath infolib (user);

  AppSearchPath app (user);
  DatabaseSearchPath database (user);

  user->setDTAPPSP(app.GetSearchPath());

  IconSearchPath icon (user);
  IconSearchPath iconbm (user, "XMICONBM", ".bm", ".pm");

  HelpSearchPath help (user);

  ManSearchPath man(user);

  app.FixUp();

  if (options->Verbose() || options->CheckingUser()) {
    CString mp(user->OS()->MountPoint());
#ifdef IOSTREAMSWORKS
    cout << "DTMOUNTPOINT: " << mp << "\n" << endl;
    cout << app << endl;
    cout << database << endl;
    cout << icon << endl;
    cout << iconbm << endl;
    cout << help << endl;
    cout << infolib << endl;
    cout << DTINFOLIBDEFAULT_NAME << ": " << DTINFOLIBDEFAULT_VALUE << "\n" << endl;
#else
    printf("DTMOUNTPOINT: %s\n",mp.data());
    app.Print();
    database.Print();
    icon.Print();
    iconbm.Print();
    help.Print();
    infolib.Print();
    printf ("%s: %s\n", DTINFOLIBDEFAULT_NAME, DTINFOLIBDEFAULT_VALUE);
#endif
    man.Print();
  }

  if (!options->CheckingUser() && !options->Verbose()) {
    app.ExportPath();
    database.ExportPath();
    icon.ExportPath();
    iconbm.ExportPath();
    help.ExportPath();
    infolib.ExportPath();

#ifdef IOSTREAMSWORKS
    if ( os->cshFormat ) {
      cout << "setenv " << DTINFOLIBDEFAULT_NAME << " " << DTINFOLIBDEFAULT_VALUE << endl;
    } else {
      cout << DTINFOLIBDEFAULT_NAME << "=" << DTINFOLIBDEFAULT_VALUE << endl;
      cout << "export " << DTINFOLIBDEFAULT_NAME << ";" << endl;
    }
#else
    if ( os->cshFormat ) {
      printf("setenv %s %s;\n", DTINFOLIBDEFAULT_NAME, DTINFOLIBDEFAULT_VALUE);
    } else {
      printf("%s=%s;\n", DTINFOLIBDEFAULT_NAME, DTINFOLIBDEFAULT_VALUE);
      printf("export %s;\n", DTINFOLIBDEFAULT_NAME);
    }
#endif /* IOSTREAMSWORKS */

    man.ExportPath();

  }

  EndCapture(argv);
  return 0;
}
