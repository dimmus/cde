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
/*******************************************************************
 *
 *  File:              dtappgather.C
 *
 *  Purpose:           Create the Application Manager
 *                     the desktop components.
 *
 *  Description:       dtappgather is invoked by the Xsession file at
 *                     user login.  It collects the application groups
 *                     and subdirectories that are to appear in the
 *                     user's Application Manager view.
 *
 *                     It is also invoked by the "ReloadApps" action.
 *
 *  Product:           @(#)Common Desktop Environment 1.0          
 *
 *  Revision:          $TOG: dtappgather.C /main/9 1998/12/14 17:38:54 mgreess $
 *
 ********************************************************************/

/********************************************************************
 *
 *  Environment variables used by this utility
 *
 *  DTMOUNTPOINT
 *     Default may be vendor specific:  /nfs
 *
 *  DTAPPSEARCHPATH
 *     Default:  $HOME/.dt,/etc/dt/appconfig,/usr/dt/appconfig
 *     Set at login by the dtsearchpath utility.
 *
 *  HOME
 *     No default.
 *     Set by the dtlogin program.
 *
 *  DISPLAY
 *     No default.
 *     Set by the dtlogin program.
 *
 *  TARGET_APPMAN_DIR
 *     Default:	/var/dt/appconfig/appmanager/$DTUSERSESSION
 *     Set within this utility.
 *
 ****************************************************************************/
#include "Options.h"
#include "dtappgather.h"
#include "DirIterator.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

Options * options;

void LegacyCleanUp
	(
	CDEEnvironment * user
	)
{
    //XXX:
    // For now, remove the old ApplicationManager subdirectory
    // so that we don't leave old CDE users with extra baggage
    // under $HOME.

    CString targetAppmanDir (user->HOME() + ".dt/ApplicationManager");

    if (user->OS()->isDirectory(targetAppmanDir) &&
        0 == user->OS()->isLink(targetAppmanDir)) {

	user->OS()->changePermissions(targetAppmanDir,0777);
	user->OS()->removeDirectory(targetAppmanDir);
    }
}

AppManagerDirectory::AppManagerDirectory
	(
	CDEEnvironment * user,
	const CString &  app
	) : user_(user),
	    appsp_(app),
	    langVersionFound(0),
	    update_(!options->Retain())
{
    // Set the users Application Manager subdirectory

    CString userhostdir = user->UserHostDir();

    if (userhostdir.isNull())
	userhostdir = "generic-display-0";

    // else if dir is non-NULL and there is a / in DTUSERSESSION, 
    // dtappgather creates /var/dt/appconfig/appmanager/DTUSERSESSION.
    // This is a possible security hole, so
    // prevent creation of directories of the form
    // /var/dt/appconfig/appmanager/directory1/directory2 
    //

    else if ((char *)strstr(userhostdir.data(),"/"))
	userhostdir = "generic-display-0";

    dirname_ = CDE_LOGFILES_TOP "/appconfig/appmanager/";
    dirname_ += userhostdir;

    if (user->OS()->isDirectory(dirname_) &&
        0 == user->OS()->isLink(dirname_)) {

	user->OS()->changeOwnerGroup(dirname_,"","");
	user->OS()->changePermissions(dirname_,0755);

	// With -r, keep what is there but backups and dead links.
	// Otherwise Update() makes the directory hold just the links
	// gathered, changing only what differs: it used to remove
	// everything here and link it all again, so at every login the
	// Application Manager directory was empty for a moment and
	// changed (its mtime) even when nothing else had.
	if (options->Retain()) {
#ifdef sun
	    user->OS()->removeFiles(dirname_, ".~*");
#else
	    user->OS()->removeFiles(dirname_, "[.]~*");
#endif
	    user->OS()->removeDeadLinks(dirname_);
	}
    }
    else {

	// Make /var/dt/appconfig/appmanager directories if not present
	// 
	// Make user session subdirectory under /var/dt/appconfig/appmanager

	CString dir(dirname_);
	dir.replace("/" + userhostdir,"");
	if (!user->OS()->isDirectory(dir)) {	// does appmanager exist?
	    dir.replace("/appmanager","");
	    if (!user->OS()->isDirectory(dir)) {   // does appconfig exist?
		dir.replace("/appconfig","");
	        if (!user->OS()->isDirectory(dir)) {   // does dt exist?
		    dir.replace("/dt","");
		    if (!user->OS()->isDirectory(dir)) {  // does /var exist?
			user->OS()->MakeDirectory(dir,0755);
			user->OS()->changeOwnerGroup(dir,"root","bin");
			user->OS()->changePermissions(dir,0755);
		    }
		    dir += "/dt";
		    user->OS()->MakeDirectory(dir,0755);
		    user->OS()->changeOwnerGroup(dir,"root","bin");
		    user->OS()->changePermissions(dir,0755);
		}
		dir += "/appconfig";
		user->OS()->MakeDirectory(dir,0755);
		user->OS()->changeOwnerGroup(dir,"bin","bin");
		user->OS()->changePermissions(dir,0755);
	    }
	    dir += "/appmanager";
	    user->OS()->MakeDirectory(dir,0755);
	    user->OS()->changeOwnerGroup(dir,"bin","bin");
	    user->OS()->changePermissions(dir,0755);
	}
	user->OS()->MakeDirectory(dirname_,0755);
	user->OS()->changeOwnerGroup(dirname_,"","");
    }

    // Make /var/dt/tmp directory if not present

    CString tmp(CDE_LOGFILES_TOP "/tmp/");
    if (!user->OS()->isDirectory(tmp)) {  // does tmp exist?
	user->OS()->MakeDirectory(tmp,0755);
	user->OS()->changeOwnerGroup(tmp,"root","bin");
	user->OS()->changePermissions(tmp,0755);
    }

    tmp += userhostdir;
    if (!user->OS()->isDirectory(tmp)) {  // does tmp/$DTUSERSESSION exist?
	user->OS()->MakeDirectory(tmp, 0755);
	user->OS()->changeOwnerGroup(tmp,"","");
	user->OS()->changePermissions(tmp,0755);
    }
}


/********************************************************************
 * 
 *  TraversePath()
 *
 * 	Parse a given search path, using comma (,) and colon (:) as
 *      delimiters.  Pass each path element to another function.
 ********************************************************************/

void AppManagerDirectory::TraversePath()
{
    if (!appsp_.isNull()) {
	CTokenizedString subpath(appsp_,":");
	CString dir = subpath.next();
	while (!dir.isNull()) {
	    GatherAppsFromASearchElement (dir);
	    dir = subpath.next();
	    if (langVersionFound && dir == CDE_DATA_TOP "/appconfig/appmanager/C")
		dir = subpath.next();
	}
    }
}

/********************************************************************
 * 
 *  goodFile()
 *
 * 	Ignore '.' and '..' and make sure file doesn't exist.
 *
 ********************************************************************/

int AppManagerDirectory::goodFile
	(
	const CString & path,
	const CString & fname
	) const
{
    if (fname == "." || fname == "..")
	return 0;

    CString filnam(path + "/" + fname);

    if (user_->OS()->FileExists(filnam))
	return 0;

    return 1;
}


/*********************************************************************
 *
 *  GatherAppsFromASearchElement()
 *
 * 	Given search path element (host name or path name), construct
 *      the appropriate path for the various desktop subsystems.
 *
 *      A path is constructed so that each element is appended to
 *      the tail.
 *********************************************************************/
void AppManagerDirectory::GatherAppsFromASearchElement
	(
	const CString & path
	) 
{
    if (path.length()) {
	CString source(path);
	if (source.contains("/%L")) {
	    if (user_->OS()->LANG() != "C") {
		source.replace("%L",user_->OS()->LANG());
		if (!user_->OS()->isDirectory(source))
		     return;
		if (source.contains(CDE_DATA_TOP "/appconfig/appmanager/"))
		    langVersionFound = 1;
	    }
	    else
		return;
	}

	else if (!user_->OS()->isDirectory(source))
	    return;

	DirectoryIterator iter (source);
	struct dirent * direntry;

	user_->OS()->setUserId();

	while ((direntry = iter())) {
	    CString dname(direntry->d_name);
	    CString spath(source + "/" + dname);
	    struct stat file;
	    int wanted;

	    // A directory or a plain file (stat() follows links).  d_type
	    // tells without a stat() unless the entry is a link or of a
	    // type the file system does not report; it used to take one
	    // stat() per test.
#ifdef DT_DIR
	    if (direntry->d_type == DT_DIR || direntry->d_type == DT_REG)
		wanted = 1;
	    else if (direntry->d_type != DT_LNK &&
		     direntry->d_type != DT_UNKNOWN)
		wanted = 0;
	    else
#endif
		wanted = stat(spath.data(), &file) == 0 &&
			 (S_ISDIR(file.st_mode) || S_ISREG(file.st_mode));
	    if (!wanted)
		continue;
	    if (update_) {
		// The first of a name wins, as below.
		if (dname != "." && dname != ".." &&
		    wantedIndex_.find(dname.data()) == wantedIndex_.end()) {
		    wantedIndex_[dname.data()] = wanted_.size();
		    wanted_.push_back(std::make_pair(std::string(dname.data()),
						     std::string(spath.data())));
		}
	    }
	    else if (goodFile(dirname_, dname))
		user_->OS()->symbolicLink (spath, dirname_ + "/" + dname);
	}

	user_->OS()->setUserId();
    }
}

/*********************************************************************
 *
 *  Update()
 *
 *	Without -r: make the directory hold the links gathered and
 *	nothing else.  A link that is there already, to the same
 *	target, is kept; anything else is removed (as it was before
 *	gathering) and the missing links are made.
 *
 *********************************************************************/
void AppManagerDirectory::Update()
{
    std::vector<std::string> names;
    std::vector<char> present(wanted_.size(), 0);

    if (!update_)
	return;

    {
	DirectoryIterator dir(dirname_);
	struct dirent * direntry;

	while ((direntry = dir()))
	    if (strcmp(direntry->d_name, ".") != 0 &&
		strcmp(direntry->d_name, "..") != 0)
		names.push_back(direntry->d_name);
    }

    for (size_t i = 0; i < names.size(); i++) {
	std::map<std::string, size_t>::const_iterator w =
	    wantedIndex_.find(names[i]);
	CString path(dirname_ + "/" + names[i].c_str());

	if (w != wantedIndex_.end()) {
	    const std::string & target = wanted_[w->second].second;
	    std::vector<char> buf(target.size() + 2);
	    ssize_t n = readlink(path.data(), &buf[0], buf.size());

	    if (n == (ssize_t) target.size() &&
		target.compare(0, target.size(), &buf[0], n) == 0) {
		present[w->second] = 1;
		continue;
	    }
	}
	user_->OS()->removeFile(path);
    }

    for (size_t i = 0; i < wanted_.size(); i++)
	if (!present[i])
	    user_->OS()->symbolicLink(wanted_[i].second.c_str(),
				      dirname_ + "/" + wanted_[i].first.c_str());
}

int main (int argc, char **argv)
{
    options = new Options(argc, argv);

    UnixEnvironment * os = new UnixEnvironment;

    CDEEnvironment * user = new CDEEnvironment(0, os);

    LegacyCleanUp(user);

    CString app(getenv("DTAPPSEARCHPATH"));

    AppManagerDirectory * appman = new AppManagerDirectory(user, app);
    appman->TraversePath();
    appman->Update();

    user->OS()->changePermissions((*appman)(),0555);

    return 0;
}
