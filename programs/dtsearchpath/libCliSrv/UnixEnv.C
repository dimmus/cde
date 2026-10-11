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
/* $TOG: UnixEnv.C /main/11 1998/12/14 17:06:02 mgreess $ */
/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

#include "Environ.h"
#include "DirIterator.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#if defined(sun)
#include <regexpr.h>
#else
#include <regex.h>
#endif
#include <errno.h>

#define UID_NO_CHANGE ((uid_t) -1)
#define GID_NO_CHANGE ((gid_t) -1)

UnixEnvironment::UnixEnvironment()
{
    dtMountPoint = getEnvironmentVariable("DTMOUNTPOINT");
    if (dtMountPoint.isNull())
#if defined(sun)
	dtMountPoint = "/net/";
#else
	dtMountPoint = "/nfs/";
#endif

    CString temp = getEnvironmentVariable("MANPATH");
    if (temp.isNull())
#if defined(sun)
	manpath = "/usr/share/man";
#elif defined(_AIX)
	manpath = "/usr/share/man:/usr/lpp/info";
#elif defined(__linux__)
        manpath = "/usr/share/man/%L:/usr/share/man:/usr/contrib/man/%L:/usr/contrib/man:/usr/local/man/%L:/usr/local/man";
#elif defined(__OpenBSD__)
	manpath = "/usr/share/man:/usr/X11R6/man:/usr/local/man:/usr/ports/infrastructure/man";
#elif defined(__FreeBSD__)
	manpath = "/usr/share/man:/usr/local/man";
#elif defined(__NetBSD__)
	manpath = "/usr/share/man:/usr/X11R6/man:/usr/X11R7/man:/usr/local/man:/usr/pkg/man";
#endif
    else
	manpath = temp;

    lang = getEnvironmentVariable("LANG");
    if (lang.isNull())
	lang = "C";

    localHost = "localhost";

    CString sh(getEnvironmentVariable("SHELL"));

    cshFormat = 0;
    if (isCsh(sh)) {
        shell_ = new CShell();
        cshFormat = 1;
    }
    else
        shell_ = new KShell();
        
    uid_ = getuid();
    gid_ = getgid();
}


UnixEnvironment::~UnixEnvironment()
{
	delete shell_;
}

int UnixEnvironment::FileExists 
	(
	const CString & filename
	) const
{
struct stat file;
int result = stat((const char *)filename.data(),&file) == 0;

    if (envRecorder)
	envRecorder->test('e', filename.data(), result);
    return result;
}

void UnixEnvironment::MakeDirectory 
	(
	const CString & dirname,
	mode_t          permissions
	)
{
    if (envRecorder)
	envRecorder->spoil();
    if (mkdir (dirname.data(), permissions) == -1) {
	CString errorStr("MakeDirectory: " + dirname);
	perror(errorStr.data());
    }	
}

CString UnixEnvironment::getEnvironmentVariable 
	(
	const char * envvar
	)
{
  char * value = getenv(envvar);
  if (envRecorder)
    envRecorder->env(envvar, value ? value : "");
  if (value == 0)
    return CString("");

  return CString(value);
}

int UnixEnvironment::isDirectory 
	(
	const CString & directory
	)
{
struct stat file;
int result = stat((const char *)directory.data(),&file) == 0 &&
	     S_ISDIR(file.st_mode);

    if (envRecorder)
	envRecorder->test('d', directory.data(), result);
    return result;
}

int UnixEnvironment::isFile
	(
	const CString & filespec
	)
{
struct stat file;
int result = stat((const char *)filespec.data(),&file) == 0 &&
	     S_ISREG(file.st_mode);

    if (envRecorder)
	envRecorder->test('f', filespec.data(), result);
    return result;
}

int UnixEnvironment::isLink
	(
	const CString & filespec
	)
{
struct stat file;
int result = lstat((const char *)filespec.data(),&file) == 0 &&
	     S_ISLNK(file.st_mode);

    if (envRecorder)
	envRecorder->test('L', filespec.data(), result);
    return result;
}

void UnixEnvironment::symbolicLink
	(
	const CString & linkto,
	const CString & linkee
	)
{
    if (envRecorder)
	envRecorder->spoil();
    if (symlink (linkto.data(), linkee.data()) == -1) {
	CString errorStr("symbolicLink: " + linkee + " -> " + linkto + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::setUserId
	(
	const char * name
	)
{
    uid_t uid = UID_NO_CHANGE;

    if (name && name[0]) {
	struct passwd * pwent = getpwnam(name);
	uid = pwent->pw_uid;
    }
    else if (name == 0)
	uid = uid_;

    if (setuid(uid) == -1) {
	CString errorStr("setUserId: ");
	errorStr += name;
	errorStr += "| ";
	perror(errorStr.data());
    }
}

void UnixEnvironment::changePermissions
	(
	const CString & filespec,
	mode_t mode
	)
{
    if (envRecorder)
	envRecorder->spoil();
    if (chmod (filespec.data(), mode) == -1) {
	CString errorStr("changePermissions: " + filespec + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::changeOwnerGroup
	(
	const CString & filespec,
	const char *    owner,
	const char *    group
	)
{
    uid_t uid = UID_NO_CHANGE;
    gid_t gid = GID_NO_CHANGE;

    if (envRecorder)
	envRecorder->spoil();

    if (owner) {
	if (owner[0]) {
	    struct passwd * pwent = getpwnam(owner);
	    uid = pwent->pw_uid;
	}
	else
	    uid = getuid();
    }
    if (group) {
	if (group[0]) {
	    struct group * grent = getgrnam(group);
	    gid = grent->gr_gid;
	}
	else
	    gid = getgid();
    }

    if (chown(filespec.data(),uid,gid) == -1) {
	CString errorStr("changeOwnerGroup: " + filespec + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::removeDirectory 
	(
	const CString & dirspec
	)
{
#ifdef sun
    removeFiles(dirspec, ".~*");
    removeFiles(dirspec, "*");
#else
    removeFiles(dirspec, "[.]~*");
    removeFiles(dirspec, "[.]*");
#endif
    if (rmdir (dirspec.data()) == -1) {
	CString errorStr("removeDirectory: " + dirspec + "| ");
	perror(errorStr.data());
    }
}

void UnixEnvironment::removeFiles 
	(
	const CString & dirspec,
	const CString & filespec
	)
{
#if defined(sun)
char buffer[100];
    sprintf(buffer,"rm -f %s/%s", dirspec.data(),filespec.data());
    system(buffer);
#else

    /*
     * The pattern is compiled once, not once per entry (and is freed:
     * the compiled patterns used to leak, one per directory entry).
     */
    regex_t re;
    if (regcomp (&re, filespec.data(), 0) != 0)
	return;

    DirectoryIterator dir(dirspec);
    struct dirent * direntry;
    while ((direntry = dir())) {
        /*# ifdef should_be_sun_but_this_dont_work*/
	if (regexec (&re, direntry->d_name, 0, NULL, 0) == 0) {
	    if (strcmp(direntry->d_name,".") == 0 ||
		strcmp(direntry->d_name,"..") == 0)
		continue;
	    removeFile(dirspec + "/" + direntry->d_name);
	}
    }
    regfree (&re);
#endif
}

void UnixEnvironment::removeFile 
	(
	const CString & filespec
	)
{
    if (envRecorder)
	envRecorder->spoil();
    if (isDirectory(filespec) && !isLink(filespec))
	removeDirectory(filespec);
    else {
        if (unlink (filespec.data()) == -1) {
	    CString errorStr("removeFile(unlink): " + filespec + "| ");
	    perror(errorStr.data());
	}
    }
}

void UnixEnvironment::removeDeadLinks 
	(
	const CString & dirspec
	)
{
    DIR * dir = opendir(dirspec.data());
    struct dirent * direntry;
    if (dir == NULL)
	return;
    while ((direntry = readdir(dir))) {
	CString path(dirspec + "/" + direntry->d_name);
	struct stat file;

	/* A link to neither a plain file nor a directory (one stat). */
	if (isLink(path))
	    if (stat(path.data(), &file) != 0 ||
		(!S_ISREG(file.st_mode) && !S_ISDIR(file.st_mode)))
	        removeFile(path);
    }
    closedir(dir);
}



/*
 * EnvRecorder
 */
EnvRecorder * envRecorder = 0;

void EnvRecorder::env
	(
	const char * name,
	const char * value
	)
{
    if (ignored_ && strcmp(name, ignored_) == 0)
	return;
    for (size_t i = 0; i < envs_.size(); i++)
	if (envs_[i].first == name)
	    return;		// the environment does not change
    envs_.push_back(std::make_pair(std::string(name), std::string(value)));
}

void EnvRecorder::test
	(
	char op,
	const char * path,
	int result
	)
{
    std::string t(result ? "" : "! ");
    t += '-';
    t += op;
    t += ' ';
    t += kshQuote(path);
    for (size_t i = 0; i < tests_.size(); i++)
	if (tests_[i] == t)
	    return;
    tests_.push_back(t);
}

std::string EnvRecorder::kshQuote
	(
	const std::string & str
	)
{
    std::string q("'");
    for (size_t i = 0; i < str.size(); i++)
	if (str[i] == '\'')
	    q += "'\\''";
	else
	    q += str[i];
    q += '\'';
    return q;
}

/*
 * A variable counts as it was read only if it is exported (the shell
 * may have variables of the same name that the program never sees):
 * $dtsp_x must hold the names of the exported variables, one per line,
 * with a newline before and after ("$(typeset +x)").  An unset and an
 * empty variable read the same.
 */
std::string EnvRecorder::kshCondition () const
{
    std::string c;

    for (size_t i = 0; i < envs_.size(); i++) {
	const std::string & name = envs_[i].first;
	const std::string & value = envs_[i].second;
	std::string exported("$dtsp_x == *\"\n" + name + "\n\"*");

	if (!c.empty())
	    c += " &&\n   ";
	if (value.empty())
	    c += "[[ ! ( " + exported + " ) || -z ${" + name + "-} ]]";
	else
	    c += "[[ " + exported + " && ${" + name + "-} == " +
		 kshQuote(value) + " ]]";
    }
    for (size_t i = 0; i < tests_.size(); i++) {
	if (!c.empty())
	    c += " &&\n   ";
	c += "[[ " + tests_[i] + " ]]";
    }
    if (c.empty())
	c = "true";
    return c;
}
