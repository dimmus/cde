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
/* $XConsortium: RFCMIME.h /main/1 1995/11/03 10:17:13 rswiston $ */
/*
 *   COMPONENT_NAME: desktop
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 119
 *
 *   OBJECT CODE ONLY SOURCE MATERIALS
 */

#pragma once

#include <EUSCompat.h>

typedef enum {
	MIME_7BIT,
	MIME_8BIT,
	MIME_QPRINT,
	MIME_BASE64
	} Encoding;

typedef enum {
	CURRENT_TO_INTERNET,
	INTERNET_TO_CURRENT
	} Direction;


/* RFCMIME.c */
void DtXlateOpToStdLocale(char *operation, char *opLocale, char **ret_stdLocale, char **ret_stdLang, char **ret_stdSet);
void DtXlateStdToOpLocale(char *operation, char *stdLocale, char *stdLang, char *stdCodeSet, char *dflt_opLocale, char **ret_opLocale);
char *targetTagName(void);
void getCharSet(char *charset);
void md5PlainText(const char *bp, const unsigned long len, unsigned char *digest);
int CvtStr(char *charSet, void *from, unsigned long from_len, void **to, unsigned long *to_len, Direction dir);
unsigned int base64size(const unsigned long len);
Encoding getEncodingType(const char *body, const unsigned int len, boolean_t strict_mime);
void writeContentHeaders(char *hdr_buf, const char *type, const Encoding enc, const char *digest, int isAllASCII);
void writeBase64(char *buf, const char *bp, const unsigned long len);
void writeQPrint(char *buf, const char *bp, const unsigned long bp_len, int is_Special);
void rfc1522cpy(char *buf, const char *value);
