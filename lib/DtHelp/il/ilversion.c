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
/* $XConsortium: ilversion.c /main/3 1995/10/23 16:02:54 rswiston $ */
/*  /ilc/ilversion.c : Contains the IL version string
    (which should really be dynamically generated).
*/

#include "ilint.h"

/*
   Image Library version <external>[ <baselevel>], for <OS> <OS version> '<library name>'
   WARNING: FIXED FORMAT EXPECTED BY /ilc/iltiffwrite.c !!!
   See notes below "ilVersionName".
*/

    IL_PRIVATE
   char _ilVersionString[] = "@(#)HP IL v 2.1  bl06; Image Library for Domain/OS SR10.X 'il'";
/*                                nnnnn vvvvvv bbbb       see above
                                  xxxxxxxxxxxxxxxxx       total size = 17 chars
*/
