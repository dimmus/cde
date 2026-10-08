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
/* $XConsortium: HelpXlate.c /main/1 1996/08/22 09:16:03 rswiston $ */
/****************************************************************************
$FILEBEG$:    HelpXlate.c
$PROJECT$:    Cde 1.0
$COMPONENT$:  DtXlate service
$1LINER$:     Implements a translation service
$COPYRIGHT$:
 (c) Copyright 1993, 1994 Hewlett-Packard Company
 (c) Copyright 1993, 1994 International Business Machines Corp.
 (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
$END$
 ****************************************************************************
 ************************************<+>*************************************/

#include <stdlib.h>
#include <string.h>
#include <locale.h>

/*=================================================================
$SHAREDBEG$:  This header appears in all appropriate DtXlate topics
=======================================================$SKIP$======*/
/*$INCLUDE$*/
#include "StringFuncsI.h"
/*$END$*/

void _DtHelpCeGetLcCtype(char **locale, char **lang, char **charset) {
    char *ptr;
    char *mLang = NULL;
    char *mCharset = NULL;
    char *mLocale = setlocale(LC_CTYPE, NULL);

    if (mLocale) mLocale = strdup(mLocale);

    if (_DtHelpCeStrchr(mLocale, ".", 1, &ptr) == 0) {
	*ptr++ = '\0';
	if (mLocale != NULL && *mLocale != '\0') mLang = strdup(mLocale);
	if (ptr != NULL && *ptr != '\0') mCharset = strdup(ptr);
    }

    if (lang) *lang = strdup(mLang ? mLang : "C");
    if (charset) *charset = strdup(mCharset ? mCharset : "UTF-8");
    if (locale) *locale = strdup(mLocale ? mLocale : "C.UTF-8");

    free(mLang);
    free(mCharset);
    free(mLocale);
}
