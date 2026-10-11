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
/*                                                                      *
 * (c) Copyright 1993, 1994, 1996 Hewlett-Packard Company               *
 * (c) Copyright 1993, 1994, 1996 International Business Machines Corp. *
 * (c) Copyright 1993, 1994, 1996 Sun Microsystems, Inc.                *
 * (c) Copyright 1993, 1994, 1996 Novell, Inc.                          *
 * (c) Copyright 1996 Digital Equipment Corporation.			*
 * (c) Copyright 1996 FUJITSU LIMITED.					*
 * (c) Copyright 1996 Hitachi.						*
 */

#define	USE_MEMCPY	/* use memcpy for line movement... */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <Xm/Xm.h>
#include "TermHeader.h"       /* for MIN/MAX */
#include "TermPrim.h"
#include "TermPrimOSDepI.h"
#include "TermPrimBufferP.h"
#include "TermPrimDebug.h"


/*
** The line buffers of a multibyte locale are cell arrays, one cell per
** column (see TermPrimBufferP.h).  Column lookups are therefore O(1),
** and the only widths ever computed are those of new characters.
*/

static void
_termBufferValidateLineWc
(
    const TermBuffer tb,
    const int row
);

static wchar_t *
_dropContCells
(
    wchar_t  *chars,
    short    *numChars,
    wchar_t **scratch
);

static void
_patchUpChar
(
    TermBuffer   tb,
    int        row,
    short       *col
);

static void
_countWidth
(
          wchar_t *wcBuffer,
    const signed char *rawWidths,
          short    length,
          short    maxWidth,
          short   *retCount,
          short   *retWidth,
          unsigned char *widths
);

static void
_putChars
(
          wchar_t *cells,
          short    col,
    const wchar_t *chars,
          short    numChars,
          short    width,
    const unsigned char *widths
);

static void
_primBufferInsertWc
(
    const TermBuffer  tb,
    const int       row,
          short      *col,
          wchar_t    *newChars,
    const signed char *rawWidths,
          short       numChars,
          short      *widthInc,
          short      *widthInsert,  /* width of inserted characters           */
          unsigned char *widths,    /* room for the width of each character  */
          termChar   *returnChars,  /* pointer to overflow buffer             */
          short      *returnCount   /* count of characters in overflow buffer */
);

static void
_primBufferOverwriteWc
(
    const TermBuffer  tb,
    const int       row,
          short      *col,
          wchar_t    *newChars,
    const signed char *rawWidths,
          short       numChars,
          short      *widthInc,
          short      *widthInsert,  /* width of inserted characters           */
          unsigned char *widths,    /* room for the width of each character  */
          termChar   *returnChars,  /* pointer to overflow buffer             */
          short      *returnCount   /* count of characters in overflow buffer */
);

#ifdef USE_SUN_WCWIDTH_PATCH
#include <limits.h>
/*
**  A small workaround for systems that don't have wcwidth...
*/
int
sun_wcwidth
(
    const wchar_t wc
)
{
    int status;
    char mbstr[MB_LEN_MAX + 1];

    status = wctomb(mbstr, wc);

    if (status > 0)
       return(euccol(mbstr));
    else
       return(status);
}
#endif /*  USE_SUN_WCWIDTH_PATCH */

/*
** the number of cells (columns) a character takes: 2 or 1 (see
** CELL_WIDTH())...
*/
int
_DtTermPrimWcCellWidth(wchar_t wc)
{
    return((wcwidth(wc) > 1) ? 2 : 1);
}

/*
** Characters handed to an insert may be the cells of another line (the
** history and resize code copy lines that way).  Drop the second-column
** cells of 2 column characters from them; a stray one (one that does not
** follow a 2 column character) becomes a space.  Returns the characters
** to insert: chars itself if it has no such cells, else a copy in
** *scratch, which the caller frees.
*/
static wchar_t *
_dropContCells
(
    wchar_t  *chars,
    short    *numChars,
    wchar_t **scratch
)
{
    wchar_t *out;
    short    i;
    short    n;

    *scratch = NULL;
    for (i = 0; i < *numChars; i++)
    {
	if (chars[i] == TermCELL_CONT)
	{
	    break;
	}
    }
    if (i >= *numChars)
    {
	return(chars);
    }

    out = (wchar_t *) XtMalloc(*numChars * sizeof(wchar_t));
    n = 0;
    for (i = 0; i < *numChars; i++)
    {
	if (chars[i] != TermCELL_CONT)
	{
	    out[n++] = chars[i];
	}
	else if ((i == 0) || (chars[i - 1] == TermCELL_CONT) ||
		 (CELL_WIDTH(chars[i - 1]) != 2))
	{
	    out[n++] = L' ';
	}
    }
    *numChars = n;
    *scratch = out;
    return(out);
}

/*
** this is a helper function, that replaces a 2 column character with
** two spaces if "col" falls on column 2 of 2 (and then decrements *col)...
*/
static void
_patchUpChar
(
    TermBuffer   tb,
    int        row,
    short       *col
)
{
    TermLine  line;
    wchar_t  *cells;

    line  = LINE_OF_TBUF(tb, row);
    cells = CELLS(line);

    if ((*col > 0) && (*col < WIDTH(line)) &&
	(cells[*col] == TermCELL_CONT))
    {
	/*
	** we are on the second column of a two column character,
	** replace the character with 2 spaces before we proceed
	*/
	cells[*col - 1] = L' ';
	cells[*col]     = L' ';

	/*
	** remember the old starting column for later...
	*/
	(*col)--;
    }
}

/*
** determine how many wide characters in the buffer we can have before the
** maxWidth is exceeded...
*/
static void
_countWidth
(
          wchar_t *wcBuffer, /* buffer of wide characters                     */
    const signed char *rawWidths, /* their wcwidth()s if known, else NULL     */
          short    wcLen,    /* number of wide chars in buffer                */
          short    maxWidth, /* maximum width desired                         */
          short   *length,   /* max num of chars with total width <= maxWidth */
          short   *width,    /* actual width of the retCount wide chars       */
          unsigned char *widths /* the width of each of them                 */
)
{
          wchar_t *pwc;
          short    charWidth;

    *width = 0;
    for (pwc = wcBuffer; pwc < wcBuffer + wcLen; pwc++)
    {
	if (rawWidths)
	{
	    charWidth = rawWidths[pwc - wcBuffer];
	}
	else if (((unsigned long) *pwc) - 0x20UL < 0x5fUL)
	{
	    /* printable ASCII, the common case... */
	    charWidth = 1;
	}
	else
	{
	    charWidth = wcwidth(*pwc);
	}
        switch (charWidth)
        {
          case -1:
            /* 
            ** invalid character, (but this should have been handled earlier)
            */
	    /* replace the character with a space... */
	    *pwc = L' ';
	    /* and set the width to 1... */
	    charWidth = 1;
            break;
          case 0:
            /* 
            ** its a null character, but is still has a width for our
            ** purposes...
            */
            charWidth = 1;
	    break;
          default:
	    charWidth = (charWidth > 1) ? 2 : 1;
            break;
        }    
	if (*width + charWidth > maxWidth)
	{
	    break;
	}
	else
	{
	    *width += charWidth;
	    widths[pwc - wcBuffer] = charWidth;
	}
    }
    *length = pwc - wcBuffer;
}

/*
** store numChars characters, width columns wide in all, into the cells
** starting at col...
*/
static void
_putChars
(
          wchar_t *cells,
          short    col,
    const wchar_t *chars,
          short    numChars,
          short    width,
    const unsigned char *widths
)
{
    short i;

    if (width == numChars)
    {
	/* no 2 column characters... */
	(void) memcpy(cells + col, chars, numChars * sizeof(wchar_t));
	return;
    }
    for (i = 0; i < numChars; i++)
    {
	cells[col++] = chars[i];
	if (widths[i] == 2)
	{
	    cells[col++] = TermCELL_CONT;
	}
    }
}

/*
** This is a helper function that inserts characters into the specified
** buffer in insert mode.
*/
static void
_primBufferInsertWc
(
    const TermBuffer  tb,
    const int       row,
          short      *col,
          wchar_t    *newChars,
    const signed char *rawWidths,
          short       numChars,
          short      *widthInc,
	  short      *widthInsert,  /* width of inserted characters      */
          unsigned char *widths,    /* room for the width of each character  */
          termChar   *returnChars,  /* pointer to overflow buffer             */
          short      *returnLength  /* count of characters in overflow buffer */
)
{
    short             lengthInsert;
    short             overflowWidth;
    short             localCol;
    short             keep;
    short             c;
    TermLine          line;
    wchar_t          *cells;
    wchar_t          *overflow = (wchar_t *) returnChars;

    /*
    ** make a copy of *col because it may be modified by
    ** _patchUpChar()
    */
    localCol = *col;

    /*
    ** first decide how many characters we can insert before
    ** running off the end of the line...
    */
    _countWidth(newChars, rawWidths, numChars, COLS(tb) - localCol,
	        &lengthInsert, widthInsert, widths);

    /*
    ** return any extra characters...
    */
    *returnLength = numChars - lengthInsert;

    if (*returnLength > 0)
    {
        /*
        ** we have some overflow...
	*/
	memcpy(overflow, newChars + lengthInsert,
	       *returnLength * sizeof(wchar_t));
    }

    /*
    ** make sure we are not trying to overwrite the second column
    ** of a two column character.
    */
    _patchUpChar(tb, row, col);

    /*
    ** Decide how many characters we can insert before running off the
    ** end of the buffer...
    */
    line  = LINE_OF_TBUF(tb, row);
    cells = CELLS(line);
    keep  = WIDTH(line);
    if (WIDTH(line) + *widthInsert <= COLS(tb))
    {
	/*
	** there is no overflow, we can insert all "lengthInsert" characters...
	*/
	*widthInc = *widthInsert;
    }
    else
    {
	/*
	** inserting the new characters will overflow the line buffer,
	** remove as many of the characters at the end of the line as
	** necessary to prevent buffer overflow (whole characters, so
	** this may remove one column more than necessary)...
	*/
	overflowWidth = WIDTH(line) + *widthInsert - COLS(tb);
	while ((overflowWidth > 0) && (keep > localCol))
	{
	    if ((keep >= 2) && (cells[keep - 1] == TermCELL_CONT))
	    {
		keep          -= 2;
		overflowWidth -= 2;
	    }
	    else
	    {
		keep--;
		overflowWidth--;
	    }
	}
	*widthInc = *widthInsert - (WIDTH(line) - keep);

	/*
	** copy the displaced characters from the line to the
	** overflow buffer...
	*/
	for (c = keep; c < WIDTH(line); c++)
	{
	    if (cells[c] != TermCELL_CONT)
	    {
		overflow[(*returnLength)++] = cells[c];
	    }
	}
    }

    /*
    ** Any overflow has been taken care of, now it's time to make
    ** room for the new characters...
    */
    memmove(cells + localCol + *widthInsert, cells + localCol,
	    (keep - localCol) * sizeof(wchar_t));

    /*
    ** copy the new characters into the buffer
    */
    _putChars(cells, localCol, newChars, lengthInsert, *widthInsert, widths);
}

/*
** This is a helper function that inserts characters into the specified
** buffer in overwrite mode.
*/
static void
_primBufferOverwriteWc
(
    const TermBuffer  tb,
    const int       row,
          short      *col,
          wchar_t    *newChars,
    const signed char *rawWidths,
          short       numChars,
          short      *widthInc,
          short      *widthInsert,  /* width of inserted characters           */
          unsigned char *widths,    /* room for the width of each character  */
          termChar   *returnChars,  /* pointer to overflow buffer             */
          short      *returnLength  /* count of characters in overflow buffer */
)
{
           short        lengthInsert;
           short        localCol;
           short        endCol;
           TermLine     line;
           wchar_t     *cells;

    line  = LINE_OF_TBUF(tb, row);
    cells = CELLS(line);

    /*
    ** make a copy of *col because it may be modified by
    ** _patchUpChar()
    */
    localCol = *col;

    /*
    ** first decide how many characters we can overwrite before
    ** running off the end of the line.
    */
    _countWidth(newChars, rawWidths, numChars, COLS(tb) - localCol,
	        &lengthInsert, widthInsert, widths);

    /*
    ** We are overwriting:
    **      - determine the width increment
    **      - put any extra new characters into the overflow buffer
    */
    if (localCol == WIDTH(line))
    {
	/*
	** we are appending to the end of the line, this is easy...
	*/
	*widthInc  = *widthInsert;
    }
    else
    {
	/*
	** we are overwriting characters in the middle of the line...
	**
	** make sure we are not trying to overwrite the second column
	** of a two column character.
	*/
	_patchUpChar(tb, row, col);

	/*
	** see if we have to deal with the end of the line...
	*/
	endCol = localCol + *widthInsert;
	if (endCol < WIDTH(line))
	{
	    /*
	    ** the line width will remain constant...
	    */
	    *widthInc = 0;

	    /*
	    ** if we overwrite column 1 of a 2 column character, its
	    ** column 2 becomes a space...
	    */
	    if (cells[endCol] == TermCELL_CONT)
	    {
		cells[endCol] = L' ';
	    }
	}
	else
	{
	    /*
	    ** the line may get wider...
	    */
	    *widthInc  = endCol - WIDTH(line);
	}
    }

    /*
    ** now insert the new characters...
    */
    _putChars(cells, localCol, newChars, lengthInsert, *widthInsert, widths);

    /*
    ** put any overflow into the overflow buffer
    */
    *returnLength = numChars - lengthInsert;
    if (*returnLength > 0)
    {
	memcpy(returnChars, newChars + lengthInsert,
	       *returnLength * sizeof(wchar_t));
    }
}

/*
** Insert as many characters as possible at the specified row,col
** return a count of the number of characters bumped off the end of the line
** in 'returnLength' and a pointer to a buffer containing those characters
** 'returnChars'.
**
** The the new column width of the line is returned as the value of the
** function.
**
** NOTES:
**      We are trying to implement mechanism and not policy.  This
**      routine does a minimum of checking for boundary conditions.
*/
short
_DtTermPrimBufferInsertWc
(
    const TermBuffer  tb,
    const int       row,
    const short       col,
          wchar_t    *newChars,
          short       numChars,
          Boolean     insertFlag,   /* if TRUE, insert, else overwrite        */
          termChar  **returnChars,  /* pointer to overflow buffer             */
          short      *returnLength  /* count of characters in overflow buffer */
)
{
    return(_DtTermPrimBufferInsertWcWidths(tb, row, col, newChars, NULL,
	    numChars, insertFlag, returnChars, returnLength));
}

/*
** The same, for a caller that already knows the wcwidth() of each of the
** characters (rawWidths[i] for newChars[i]), so that they need not be
** computed again.
*/
short
_DtTermPrimBufferInsertWcWidths
(
    const TermBuffer  tb,
    const int       row,
    const short       col,
          wchar_t    *newChars,
    const signed char *rawWidths,
          short       numChars,
          Boolean     insertFlag,   /* if TRUE, insert, else overwrite        */
          termChar  **returnChars,  /* pointer to overflow buffer             */
          short      *returnLength  /* count of characters in overflow buffer */
)
{
           short    widthInc = 0;       /* incremental change in line width  */
           short    widthInsert;    /* column width of chars inserted    */
	   short    localCol;
           TermLine line;
           wchar_t *scratch;
           unsigned char widthBuf[256];
           unsigned char *widths;

    if (!VALID_ROW(tb, row) || !VALID_COL(tb, col))
    {
        *returnLength = 0;
        return(0);
    }

    if (isDebugFSet('i', 1)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	(void) _termBufferValidateLineWc(tb, row);
    }

    line     = LINE_OF_TBUF(tb, row);
    localCol = col;
    newChars = _dropContCells(newChars, &numChars, &scratch);
    if (scratch)
    {
	/* the widths no longer match the characters... */
	rawWidths = NULL;
    }
    widths   = (numChars <= (short) sizeof(widthBuf)) ? widthBuf :
	       (unsigned char *) XtMalloc(numChars);

    if (WIDTH(line) < col)
    {
        /*
        ** We're adding characters past the current end of line,
        ** pad it out.
        */
        _DtTermPrimBufferPadLineWc(tb, row, col);
    }

    /*
    ** It doesn't matter if we're overwriting, or inserting at the end
    ** of the line, the result is the same...
    */
    if (insertFlag == False || col == WIDTH(line))
    {
	_primBufferOverwriteWc(tb, row, &localCol, newChars, rawWidths,
			       numChars,
			       &widthInc, &widthInsert, widths,
			       *returnChars, returnLength);
    }
    else
    {
	_primBufferInsertWc(tb, row, &localCol, newChars, rawWidths,
			    numChars,
			    &widthInc, &widthInsert, widths,
			    *returnChars, returnLength);
    }

    /*
    ** Everything's ready:
    **     - adjust the line width (and length, which is the same)
    **     - update the enhancements
    */
    WIDTH(line)  += widthInc;
    LENGTH(line)  = WIDTH(line);

    /*
    ** fix up the enhancments...
    */
    if (INSERT_ENH(tb))
    {
        (*INSERT_ENH(tb))(tb, row, col, widthInsert, insertFlag);
    }

    if (isDebugFSet('i', 1)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	(void) _termBufferValidateLineWc(tb, row);
    }

    if (scratch)
    {
	XtFree((char *) scratch);
    }
    if (widths != widthBuf)
    {
	XtFree((char *) widths);
    }
    return(WIDTH(line));
}

void
_DtTermPrimBufferDeleteWc
(
    TermBuffer  tb,
    int      *row,
    short      *col,
    short      *width,
    termChar  **returnChars,  /* pointer to delete buffer        */
    short      *returnCount  /* count of bytes in delete buffer */
)
{
    TermLine  line;
    wchar_t  *cells;
    wchar_t  *pwc;
    int     localRow;
    short     localCol;
    short     stopCol;
    short     c;
    short     n;

    if (!VALID_ROW(tb, *row) || !VALID_COL(tb, *col))
    {
        if (returnChars)
	{
	    *returnChars = NULL;
	    *returnCount = 0;
	}
        return;
    }

    /*
    ** save some local copies, to originals may get modified
    */
    localRow = *row;
    localCol = *col;
    line     = LINE_OF_TBUF(tb, *row);
    cells    = CELLS(line);
    *width   = MAX(0, MIN(WIDTH(line) - localCol, *width));

    /*
    ** there are 3 cases of deleting a character from a line:
    **     Case 1:
    **       the cursor is at or past the end of the line
    **
    **     Case 2:
    **       the cursor is in the middle of the line
    **          - move the remaining characters to the left
    **          - deleteEnhancement
    **          - adjust WIDTH and LENGTH
    **
    **     Case 3:
    **       the deleted columns reach the end of the line
    **          - deleteEnhancement
    **          - adjust WIDTH and LENGTH
    */
    if (localCol >= WIDTH(line) || *width == 0)
    {
        /*
        ** Handle Case 1...
        */
        if (returnChars)
	{
	    *returnChars = NULL;
	    *returnCount = 0;
	}
        return;
    }

    /*
    ** make any necessary adjustments to 2 column characters (the
    ** deleted columns can not start or end in the middle of one)...
    */
    _patchUpChar(tb, *row, col);

    stopCol = MIN(WIDTH(line), localCol + *width);
    if ((stopCol < WIDTH(line)) && (cells[stopCol] == TermCELL_CONT))
    {
	/*
	** do not try to delete column 1 of 2...
	**
	** replace the 2 column character with two one column spaces...
	*/
	cells[stopCol - 1] = L' ';
	cells[stopCol]     = L' ';
    }

    /*
    ** Save the current characters before we overwrite them
    ** (if returnChars is non-NULL 0)
    */
    if (returnChars != NULL)
    {
	for (n = 0, c = localCol; c < stopCol; c++)
	{
	    if (cells[c] != TermCELL_CONT)
	    {
		n++;
	    }
	}
	*returnCount = n * sizeof(wchar_t);
	*returnChars = (termChar *)XtMalloc(*returnCount);
	for (pwc = (wchar_t *) *returnChars, c = localCol; c < stopCol; c++)
	{
	    if (cells[c] != TermCELL_CONT)
	    {
		*pwc++ = cells[c];
	    }
	}
    }

    /*
    ** Cases 2 and 3 require that we delete the enhancement...
    */
    if (DELETE_ENH(tb))
    {
        (*DELETE_ENH(tb))(tb, localRow, localCol, *width);
    }

    if (stopCol < WIDTH(line))
    {
        /*
        ** handle case 2
        */
        memmove(cells + localCol, cells + stopCol,
                (WIDTH(line) - stopCol) * sizeof(wchar_t));
    }

    /*
    ** Cases 2 and 3 require that we decrement the line width...
    */
    WIDTH(line)  -= *width;
    LENGTH(line)  = WIDTH(line);

    /*
    ** update info we need to return...
    */
    *width = stopCol - localCol;
}

/*
** Pad the requested row from the current width to 'newWidth' with spaces...
*/
void
_DtTermPrimBufferPadLineWc
(
    const TermBuffer  tb,
    const int       row,
    const short       width
)
{
    short       i;
    short       widthInc;
    TermLine    line;
    wchar_t    *pwc;

    line = LINE_OF_TBUF(tb, row);

    if (isDebugFSet('i', 1)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	(void) _termBufferValidateLineWc(tb, row);
    }

    /*
    ** if this line is part of the selection, disown the selection...
    */
    if (IS_IN_SELECTION(line, MIN(width, WIDTH(line)),
			MAX(width, WIDTH(line))))
    {
	(void) _DtTermPrimSelectDisown(WIDGET(tb));
    }

    widthInc = MIN(COLS(tb), width) - WIDTH(line);

    for (i = 0, pwc = CELLS(line) + WIDTH(line); i < widthInc; i++, pwc++)
    {
        *pwc = L' ';
    }
    if (CLEAR_ENH(tb))
    {
        (*CLEAR_ENH(tb))(tb, row, WIDTH(line), widthInc);
    }
    _DtTermPrimBufferSetLineWidth(tb, row, WIDTH(line) + widthInc);
    if (isDebugFSet('i', 1)) {
#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
	_termBufferValidateLineWc(tb, row);
    }
}

/*
** Clear the line to the new width (just reset the line width).
*/
Boolean
_DtTermPrimBufferClearLineWc
(
    const TermBuffer  tb,
    const int       row,
          short       newWidth
)
{
    TermLine        line;

    /*
    ** Some simple bounds checking.
    */
    if (!VALID_ROW(tb, row))
    {
        return(False);
    }

    /*
    ** force the width to the desired value
    **
    ** (We take the direct approach because _DtTermPrimBufferSetLineWidth
    **  doesn't allow the line width to decrease.)
    */
    line = LINE_OF_TBUF(tb, row);

    /*
    ** if this line is part of the selection, disown the selection...
    */
    if (IS_IN_SELECTION(line, MIN(newWidth, WIDTH(line)),
			MAX(newWidth, WIDTH(line))))
    {
	(void) _DtTermPrimSelectDisown(WIDGET(tb));
    }

    /*
    ** Clip the new width to the buffer width.
    */
    newWidth = MIN(newWidth, COLS(tb));

    if (newWidth < WIDTH(line))
    {
	/*
	** handle the case of clearing the second column of a two column
	** character: its first column becomes a space...
	*/
	if ((newWidth > 0) && IS_WIDE_START(line, newWidth - 1))
	{
	    CELLS(line)[newWidth - 1] = L' ';
	}

	/*
	** Call the helper function if it exists
	*/
	if (CLEAR_LINE(tb))
	{
	    (*CLEAR_LINE(tb))(tb, row, newWidth);
	}
	WRAPPED(line) = False;
	WIDTH(line)   = newWidth;
	LENGTH(line)  = newWidth;
    }
    return(True);
}

/*
** replace all characters from startCol upto stopCol with spaces,
**
** NOTE:
**    we are dealing with double width characters: a 2 column character
**    that is partly in the erased columns becomes 2 spaces.
*/
void
_DtTermPrimBufferEraseWc
(
    TermBuffer tb,
    int      row,
    short      startCol,
    short      stopCol
)
{
    TermLine              line;
    wchar_t              *cells;
    short                 localCol;
    short                 c;

    line  = LINE_OF_TBUF(tb, row);
    cells = CELLS(line);

    /*
    ** only erase columns that are in the line...
    */
    stopCol = MIN(stopCol, WIDTH(line) - 1);
    if (startCol > stopCol)
    {
	return;
    }

    /*
    ** make sure we are not trying to erase the second column
    ** of a two column character.
    */
    localCol = startCol;
    _patchUpChar(tb, row, &localCol);

    /*
    ** make sure we are not trying to erase the first column
    ** of a two column character.
    */
    if (IS_WIDE_START(line, stopCol))
    {
	(void) stopCol++;
    }

    /*
    ** replace the characters with spaces...
    */
    for (c = startCol; c <= stopCol; c++)
    {
	cells[c] = L' ';
    }
}

#ifdef	BBA
#pragma BBA_IGNORE
#endif	/*BBA*/
static void
_termBufferValidateLineWc
(
    const TermBuffer tb,
    const int row
)
{
    wchar_t    *pwc;
    TermLine    line;

    line = LINE_OF_TBUF(tb, row);
    if (LENGTH(line) != WIDTH(line))
    {
	fprintf(stderr, "_termBufferValidateLineWc: length %d != width %d\n",
		LENGTH(line), WIDTH(line));
    }
    for (pwc = CELLS(line); pwc < CELLS(line) + WIDTH(line); pwc++)
    {
	if (*pwc == TermCELL_CONT)
	{
	    if ((pwc == CELLS(line)) || (pwc[-1] == TermCELL_CONT) ||
		(CELL_WIDTH(pwc[-1]) != 2))
	    {
		fprintf(stderr, "_termBufferValidateLineWc: stray cell\n");
	    }
	    continue;
	}
	if ((CELL_WIDTH(*pwc) == 2) &&
	    ((pwc + 1 >= CELLS(line) + WIDTH(line)) ||
	     (pwc[1] != TermCELL_CONT)))
	{
	    fprintf(stderr, "_termBufferValidateLineWc: missing cell\n");
	}
        if (wcwidth(*pwc) == -1)
        {
            fprintf(stderr, "_termBufferValidateLineWc: invalid wide char\n");
	    /* replace the character with a space... */
	    *pwc = L' ';
            break;
        }
    }
}

#if (defined(TEST) || defined(__CODECENTER__) || defined(DEBUG))
static void
_termBufferPrintLine
(
    const TermBuffer tb,
    const int row
)
{
    TermLine    line;
    wchar_t   *pChar;
    short       j;

    printf("Line: %d\n", row);

    line = LINE_OF_TBUF(tb, row);
    printf("    length: %3d\n", WIDTH(line));
    if (WIDTH(line) > 0)
    {
        printf("    buffer: <");
        pChar = BUFFER(line);
        for (j = 0; j < WIDTH(line); j++)
        {
            printf("%X", *pChar++);
        }
        printf(">\n");
    }
}

/*
** Print the contents of the TermBuffer.
*/
static void
_termBufferPrintBuffer
(
    const TermBuffer tb
)
{
    short i;
    short j;
    short k;

    if (tb == NULL) {
        printf("TermBuffer has been freed.\n");
        return;
    }

    printf("TermBuffer dimensions:\n");
    printf("    rows: %d\n", ROWS(tb));
    printf("    cols: %d\n", COLS(tb));

    for (i = 0; i < ROWS(tb); i++)
    {
        _termBufferPrintLine(tb, i);
    }
}
#endif /* (defined(TEST) || defined(__CODECENTER__)) */

#ifdef TEST
/*
** Some simple tests of the termBuffer.
*/
/* the following is to allow for a single main function in the code... */
#define       termBufMain     main
termBufMain()
{
    const TermBuffer  myTB;

    printf("Sizeof termEnhRec    : %d\n", sizeof(struct _termEnhRec));
    printf("Sizeof termBufferRec : %d\n", sizeof(struct _TermBufferRec));

    myTB = _DtTermPrimBufferCreateBuffer(12, 80);
    _termBufferPrintBuffer(myTB);

    printf("[0,0] %d\n", _DtTermPrimBufferSetEnhancement(myTB, 0, 0, enhVideo, BLINK));
    _termBufferPrintEnhancement(myTB, 0, 0);
    printf("[0,1] %d\n", _DtTermPrimBufferSetEnhancement(myTB, 0, 1, enhVideo, INVERSE));
    _termBufferPrintEnhancement(myTB, 0, 0);
    _termBufferPrintEnhancement(myTB, 0, 1);
    printf("[0,9] %d\n", _DtTermPrimBufferSetEnhancement(myTB, 0, 9, enhVideo, UNDERLINE));
    _termBufferPrintEnhancement(myTB, 0, 0);
    _termBufferPrintEnhancement(myTB, 0, 1);
    _termBufferPrintEnhancement(myTB, 0, 9);
    printf("[0,6] %d\n", _DtTermPrimBufferSetEnhancement(myTB, 0, 6, enhVideo, HALF_BRIGHT));
    _termBufferPrintEnhancement(myTB, 0, 0);
    _termBufferPrintEnhancement(myTB, 0, 1);
    _termBufferPrintEnhancement(myTB, 0, 6);
    _termBufferPrintEnhancement(myTB, 0, 9);
    _termBufferPrintBuffer(myTB);

    _DtTermPrimBufferSetEnhancement(myTB, 10, 10, enhVideo, BLINK);
    _DtTermPrimBufferSetEnhancement(myTB, 10, 20, enhColor, 3);
    _termBufferPrintBuffer(myTB);

    _DtTermPrimBufferResizeBuffer(&myTB,  6, 40);
    _termBufferPrintBuffer(myTB);

    _DtTermPrimBufferSetEnhancement(myTB, 10, 10, enhVideo, BLINK);
    _DtTermPrimBufferResizeBuffer(&myTB, 12, 80);
    _termBufferPrintBuffer(myTB);

    _DtTermPrimBufferFreeBuffer(myTB);
}
#endif /* TEST */

