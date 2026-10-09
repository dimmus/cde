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

/*
 * In-process replacement for "compress -f" and "compress -d".
 *
 * Writes and reads the compress(1) ".Z" format (magic 0x1f 0x9d, block
 * mode, 9 to 16 bit LZW codes) that lib/DtHelp/decompress.c reads at
 * run time.  The encoder follows compress 4.0, including the padding to
 * a full group of n_bits bytes when the code width changes and the CLEAR
 * code it emits when the compression ratio drops, so any compress(1)
 * compatible decoder reads its output.
 */

#include <stdio.h>
#include <string.h>

#include "lzw.h"

#define LZW_BITS        16
#define LZW_HSIZE       69001           /* 95% occupancy for 16 bits */
#define LZW_INIT_BITS   9
#define LZW_BIT_MASK    0x1f
#define LZW_BLOCK_MASK  0x80
#define LZW_FIRST       257             /* first free entry */
#define LZW_CLEAR       256             /* table clear output code */
#define LZW_CHECK_GAP   10000           /* ratio check interval */
#define LZW_MAXCODE(n)  ((1L << (n)) - 1)

static const unsigned char lmask[9] =
    { 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 };
static const unsigned char rmask[9] =
    { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

/* encoder state */
static long           htab[LZW_HSIZE];
static unsigned short codetab[LZW_HSIZE];
static unsigned char  outBuf[LZW_BITS];
static int            nBits, offset, clearFlag;
static long           maxCode, freeEnt, bytesOut;
static const long     maxMaxCode = 1L << LZW_BITS;

/* decoder state */
static unsigned short prefixTab[1L << LZW_BITS];
static unsigned char  suffixTab[1L << LZW_BITS];
static unsigned char  deStack[1L << LZW_BITS];


static void ClearHash(void)
{
long i;

for (i = 0; i < LZW_HSIZE; i++)
    htab[i] = -1;
}


/* Output the given code; a negative code flushes the last bits. */
static int Output(FILE *out, long code)
{
int            rOff = offset, bits = nBits;
unsigned char *bp = outBuf;

if (code >= 0)
    {
    bp += rOff >> 3;
    rOff &= 7;
    *bp = (*bp & rmask[rOff]) | ((code << rOff) & lmask[rOff]);
    bp++;
    bits -= 8 - rOff;
    code >>= 8 - rOff;
    if (bits >= 8)
	{
	*bp++ = code;
	code >>= 8;
	bits -= 8;
	}
    if (bits)
	*bp = code;
    offset += nBits;
    if (offset == (nBits << 3))
	{
	if (fwrite(outBuf, 1, nBits, out) != (size_t) nBits)
	    return -1;
	bytesOut += nBits;
	offset = 0;
	memset(outBuf, 0, sizeof(outBuf));
	}

    /* The decoder only notices a width change after it has read the
     * whole group, so write the whole group now.
     */
    if (freeEnt > maxCode || clearFlag)
	{
	if (offset > 0)
	    {
	    if (fwrite(outBuf, 1, nBits, out) != (size_t) nBits)
		return -1;
	    bytesOut += nBits;
	    }
	offset = 0;
	memset(outBuf, 0, sizeof(outBuf));
	if (clearFlag)
	    {
	    maxCode = LZW_MAXCODE(nBits = LZW_INIT_BITS);
	    clearFlag = 0;
	    }
	else
	    {
	    nBits++;
	    if (nBits == LZW_BITS)
		maxCode = maxMaxCode;
	    else
		maxCode = LZW_MAXCODE(nBits);
	    }
	}
    }
else
    {
    if (offset > 0)
	{
	if (fwrite(outBuf, 1, (offset + 7) / 8, out) !=
						(size_t) ((offset + 7) / 8))
	    return -1;
	bytesOut += (offset + 7) / 8;
	}
    offset = 0;
    }
return 0;
}


int LzwCompress(FILE *in, FILE *out)
{
static const unsigned char header[3] =
    { 0x1f, 0x9d, LZW_BITS | LZW_BLOCK_MASK };
long  inCount, checkpoint, ratio, rat, fcode, i, disp, ent;
int   c, hshift;

if (fwrite(header, 1, 3, out) != 3)
    return -1;

memset(outBuf, 0, sizeof(outBuf));
offset     = 0;
bytesOut   = 3;
clearFlag  = 0;
ratio      = 0;
inCount    = 1;
checkpoint = LZW_CHECK_GAP;
maxCode    = LZW_MAXCODE(nBits = LZW_INIT_BITS);
freeEnt    = LZW_FIRST;

if ((ent = getc(in)) == EOF)
    return ferror(in) ? -1 : 0;

hshift = 0;
for (fcode = LZW_HSIZE; fcode < 65536L; fcode *= 2L)
    hshift++;
hshift = 8 - hshift;
ClearHash();

while ((c = getc(in)) != EOF)
    {
    inCount++;
    fcode = ((long) c << LZW_BITS) + ent;
    i = ((long) c << hshift) ^ ent;
    if (htab[i] == fcode)
	{
	ent = codetab[i];
	continue;
	}
    if (htab[i] >= 0)
	{
	disp = LZW_HSIZE - i;
	if (i == 0)
	    disp = 1;
	for (;;)
	    {
	    if ((i -= disp) < 0)
		i += LZW_HSIZE;
	    if (htab[i] == fcode)
		break;
	    if (htab[i] < 0)
		break;
	    }
	if (htab[i] == fcode)
	    {
	    ent = codetab[i];
	    continue;
	    }
	}

    /* no match */
    if (Output(out, ent) < 0)
	return -1;
    ent = c;
    if (freeEnt < maxMaxCode)
	{
	codetab[i] = freeEnt++;
	htab[i] = fcode;
	}
    else if (inCount >= checkpoint)
	{
	/* table full: start over if the compression ratio drops */
	checkpoint = inCount + LZW_CHECK_GAP;
	if (inCount > 0x007fffffL)
	    {
	    rat = bytesOut >> 8;
	    rat = (rat == 0) ? 0x7fffffffL : inCount / rat;
	    }
	else
	    rat = (inCount << 8) / bytesOut;
	if (rat > ratio)
	    ratio = rat;
	else
	    {
	    ratio = 0;
	    ClearHash();
	    freeEnt = LZW_FIRST;
	    clearFlag = 1;
	    if (Output(out, LZW_CLEAR) < 0)
		return -1;
	    }
	}
    }
if (ferror(in))
    return -1;

if (Output(out, ent) < 0 || Output(out, -1) < 0)
    return -1;
return 0;
}


/* decoder: read one code of width nBits, or -1 at end of input */
static unsigned char inBuf[LZW_BITS];
static int           inOffset, inSize;
static long          deMaxCode, deFreeEnt;
static int           deBits, deMaxBits, deClear;

static long GetCode(FILE *in)
{
long           code;
int            rOff, bits;
unsigned char *bp = inBuf;

if (deClear || inOffset >= inSize || deFreeEnt > deMaxCode)
    {
    /* new group: pick up a pending change of the code width first */
    if (deFreeEnt > deMaxCode)
	{
	deBits++;
	if (deBits == deMaxBits)
	    deMaxCode = 1L << deMaxBits;
	else
	    deMaxCode = LZW_MAXCODE(deBits);
	}
    if (deClear)
	{
	deMaxCode = LZW_MAXCODE(deBits = LZW_INIT_BITS);
	deClear = 0;
	}
    inSize = fread(inBuf, 1, deBits, in);
    if (inSize <= 0)
	return -1;
    inOffset = 0;
    /* round down to an integral number of codes */
    inSize = (inSize << 3) - (deBits - 1);
    }
rOff = inOffset;
bits = deBits;

bp += rOff >> 3;
rOff &= 7;
code = *bp++ >> rOff;
bits -= 8 - rOff;
rOff = 8 - rOff;
if (bits >= 8)
    {
    code |= (long) *bp++ << rOff;
    rOff += 8;
    bits -= 8;
    }
code |= (long) (*bp & rmask[bits]) << rOff;
inOffset += deBits;
return code;
}


int LzwDecompress(FILE *in, FILE *out)
{
unsigned char  hdr[3];
unsigned char *sp;
long           code, oldcode, incode;
int            finchar;

if (fread(hdr, 1, 3, in) != 3 || hdr[0] != 0x1f || hdr[1] != 0x9d)
    return -1;
deMaxBits = hdr[2] & LZW_BIT_MASK;
if (deMaxBits < 12 || deMaxBits > LZW_BITS || !(hdr[2] & LZW_BLOCK_MASK))
    return -1;

deMaxCode = LZW_MAXCODE(deBits = LZW_INIT_BITS);
deFreeEnt = LZW_FIRST;
deClear   = 0;
inOffset  = inSize = 0;
for (code = 255; code >= 0; code--)
    {
    prefixTab[code] = 0;
    suffixTab[code] = (unsigned char) code;
    }

finchar = oldcode = GetCode(in);
if (oldcode == -1)
    return 0;
if (oldcode > 255 || putc(finchar, out) == EOF)
    return -1;
sp = deStack;

while ((code = GetCode(in)) > -1)
    {
    if (code == LZW_CLEAR)
	{
	deFreeEnt = LZW_FIRST - 1;
	deClear = 1;
	continue;
	}
    incode = code;
    if (code >= deFreeEnt)
	{
	if (code > deFreeEnt)
	    return -1;          /* corrupt input */
	*sp++ = finchar;
	code = oldcode;
	}
    while (code >= 256)
	{
	*sp++ = suffixTab[code];
	code = prefixTab[code];
	}
    *sp++ = finchar = suffixTab[code];
    do
	if (putc(*--sp, out) == EOF)
	    return -1;
    while (sp > deStack);

    if (deFreeEnt < (1L << deMaxBits))
	{
	prefixTab[deFreeEnt] = (unsigned short) oldcode;
	suffixTab[deFreeEnt] = finchar;
	deFreeEnt++;
	}
    oldcode = incode;
    }
return ferror(in) ? -1 : 0;
}
