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
/*
 * DtsI.h: data typing entry points internal to CDE (not installed).
 */
#ifndef _Dt_DtsI_h
#define _Dt_DtsI_h

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DtDtsDataToDataType(path, NULL, 0, st, link_target, NULL, NULL) for a
 * caller that has already looked at the file (dtfile reading a folder),
 * so that typing does not look again:
 *
 *   dirfd, name  NAME relative to the open directory DIRFD is the same
 *                file as PATH; the file (CONTENT) and a folder's
 *                .DtDirDataType are opened with openat() relative to it.
 *                Pass -1 and NULL to use PATH.
 *   path         the path typed (NAME_PATTERN, PATH_PATTERN), as for
 *                DtDtsDataToDataType.
 *   st           stat() of the file, following links, or NULL.
 *   lst          lstat() of the entry, or NULL.  Unlike the LS argument
 *                of DtDtsDataToDataType, which means "this is a link",
 *                it is a real lstat: MODE l and the LINK_NAME and
 *                LINK_PATH of a file that is not a link are answered
 *                from it, without the lstat() or readlink() typing would
 *                otherwise do.
 *   link_target  the final target of a link (absolute), or NULL.
 *
 * Returns the data type, to be released with DtDtsFreeDataType.
 */
extern char *_DtDtsDataToDataTypeAt(int dirfd,
				    const char *name,
				    const char *path,
				    const struct stat *st,
				    const struct stat *lst,
				    const char *link_target);

#ifdef __cplusplus
}
#endif

#endif /* _Dt_DtsI_h */
