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
** $TOG: TermPrimBufferP.h /main/2 1997/04/17 18:05:11 samborn $
*/
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef   _Dt_TermPrimBufferP_h
#define   _Dt_TermPrimBufferP_h

#include  "TermPrimBuffer.h"
#include  "TermPrimSelect.h"

#ifdef	   __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define VALID_ROW(tb, r)    (((r) >= 0) && ((r) < (tb)->term_buffer.rows))
#define VALID_COL(tb, c)    (((c) >= 0) && ((c) < (tb)->term_buffer.cols))
#define VALID_LENGTH(tb, l) (((l) >= 0) && ((l) <= (tb)->term_buffer.cols))

/*
** Make it easier to access members of the Terminal Buffer
*/
/*
** The lines of a buffer are a ring of MAX_ROWS(tb) line pointers in
** LINES_BASE(tb), starting at slot LINES_FIRST(tb), so that moving the top
** lines of a full buffer to its bottom (what scrolling does) is a change
** of LINES_FIRST.  Use LINE_OF_TBUF() for a row, never LINES_BASE()[row].
*/
#define LINES_BASE(tb)     ((tb)->term_buffer.linesBase)
#define LINES_FIRST(tb)    ((tb)->term_buffer.linesFirst)
#define LINE_OF_TBUF(tb, row)  (*_DtTermPrimLineSlot((tb), (row)))
#define ROWS(tb)           ((tb)->term_buffer.rows)
#define COLS(tb)           ((tb)->term_buffer.cols)
#define MAX_ROWS(tb)       ((tb)->term_buffer.maxRows)
#define MAX_COLS(tb)       ((tb)->term_buffer.maxCols)
#define TABS(tb)           ((tb)->term_buffer.tabs)
#define NUM_ENH_FIELDS(tb) ((tb)->term_buffer.numFields)
#define SIZE_OF_BUFFER(tb) ((tb)->term_buffer.sizeOfBuffer)
#define SIZE_OF_LINE(tb)   ((tb)->term_buffer.sizeOfLine)
#define SIZE_OF_ENH(tb)    ((tb)->term_buffer.sizeOfEnh)
#define VALUE_LIST(tb)     ((tb)->term_buffer.valueList)
#define WIDGET(tb)         ((tb)->term_buffer.widget)

/*
** Make it easy to access the term buffer methods
*/
#define BUFFER_CREATE(tb)  ((tb)->term_buffer.buffer_create_proc)
#define BUFFER_FREE(tb)    ((tb)->term_buffer.buffer_free_proc)
#define BUFFER_RESIZE(tb)  ((tb)->term_buffer.buffer_resize_proc)
#define ENH_PROC(tb)       ((tb)->term_buffer.enh_proc)
#define CLEAR_ENH(tb)      ((tb)->term_buffer.clear_enh)
#define INSERT_ENH(tb)     ((tb)->term_buffer.insert_enh)
#define DELETE_ENH(tb)     ((tb)->term_buffer.delete_enh)
#define SET_ENH(tb)        ((tb)->term_buffer.set_enh)
#define GET_ENH(tb)        ((tb)->term_buffer.get_enh)
#define SET_LINE_WIDTH(tb) ((tb)->term_buffer.set_line_width)
#define SET_LINE_LENGTH(tb)((tb)->term_buffer.set_line_len)
#define CLEAR_LINE(tb)     ((tb)->term_buffer.clear_line)
#define NEXT_BUFFER(tb)    ((tb)->term_buffer.nextBuffer)
#define PREV_BUFFER(tb)    ((tb)->term_buffer.prevBuffer)
#define BUFFER_SELECT_INFO(tb) ((tb)->term_buffer.selectInfo)
#define WIDTH(l)           ((l)->term_line.width)
#define LENGTH(l)          ((l)->term_line.length)
#define BUFFER(l)          ((l)->term_line.buffer)
#define WRAPPED(l)         ((l)->term_line.wrapped)
#define	IN_SELECTION(l)	   ((l)->term_line.inSelection)
#define	IS_IN_SELECTION(l,s,e) \
		   (!(((l)->term_line.inSelection.selectionStart > e) || \
		   ((l)->term_line.inSelection.selectionEnd < s)))
#define START_SELECTION(l) ((l)->term_line.inSelection.selectionStart)
#define END_SELECTION(l)   ((l)->term_line.inSelection.selectionEnd)
#define	MIN_SELECTION_COL  (0)
#define	MAX_SELECTION_COL  (32700)
#define	NON_SELECTION_COL  (-1)
#define BYTES_PER_CHAR(tb) ((tb)->term_buffer.bytesPerChar)

/*
** In a multibyte locale (BYTES_PER_CHAR(tb) == sizeof(wchar_t)), the
** buffer of a line is an array of cells, one per column: a character
** that is 2 columns wide is stored in the cell of its first column, and
** the cell of its second column holds TermCELL_CONT.  So column c of a
** line is always cell c, LENGTH(line) == WIDTH(line), and a cell holds
** TermCELL_CONT exactly when the cell before it starts a 2 column
** character.  (wchar_t -1 is no character: the input decoders and
** mbtowc() produce 0 .. 0x7fffffff.)  Text handed to or returned by the
** buffer as characters (inserts, overflow, deleted text) never contains
** TermCELL_CONT; an insert drops the ones that it is given, so that the
** cells of one line can be inserted into another.
*/
#define	TermCELL_CONT		((wchar_t) -1)
#define	CELLS(l)		((wchar_t *) BUFFER(l))
/* does the character in column col of line l take 2 columns? */
#define	IS_WIDE_START(l, col) \
	(((col) + 1 < WIDTH(l)) && (CELLS(l)[(col) + 1] == TermCELL_CONT))

/*
** The number of columns a wide character takes in the buffer: 2 for a
** double width character, else 1 (non-printing and zero width characters
** take a column of their own).  Printable ASCII is 1 in every locale, so
** it doesn't need a call to wcwidth().
*/
extern int _DtTermPrimWcCellWidth(wchar_t wc);
#define	CELL_WIDTH(wc) \
	((((unsigned long) (wc)) - 0x20UL < 0x5fUL) ? 1 : \
	_DtTermPrimWcCellWidth(wc))

/* 
** declare the private enhancement information
*/
typedef struct _TermEnhPart
{
    int foo;    /* placate the compiler */
} TermEnhPart;

typedef struct _TermEnhRec
{
    TermEnhPart term_enh;
} TermEnhRec, *TermEnh;

typedef struct _TermLineSelectionRec
{
    short	    selectionStart;
    short	    selectionEnd;
} TermLineSelectionRec;

/* 
** Lines have a column length, and a character length, these two numbers are
** not always equal, since, in some locales (i.e. Japanese) the character
** glyphs can be two columns wide.  We also want to know if this line wrapped
** to the following line.
*/
typedef struct _TermLinePart
{
    short           width;   /* single width columns */
    short           length;  /* in characters        */
    TermLineSelectionRec
		    inSelection;
    Boolean         wrapped; /* did this line wrap to the following line? */
    termChar       *buffer;
} TermLinePart;

typedef struct _TermLineRec
{
    TermLinePart term_line;
} TermLineRec, *TermLine;

/* 
** This structure defines the buffer instance part
*/
typedef struct _TermBufferPart
{
    int                 rows;  /* height (in chars) of character buffer      */
    short               cols;  /* width  (in single width cols) of buffer    */
    int                 maxRows;  /* maximum height (before any resize)      */
    short               maxCols;  /* maximum width  (before any resize)      */
    short               bytesPerChar; /* bytes per character */
    Boolean            *tabs;         /* array of flags showing tab positions*/
    unsigned char       numFields;    /* number of fields in an enhancement  */
    short               sizeOfBuffer; /* bytes per buffer record             */
    short               sizeOfLine;   /* bytes per enhancement record        */
    short               sizeOfEnh;    /* bytes per line record               */
    enhValues           valueList;    /* local storage for enhancement values*/
    TermBuffer          nextBuffer;   /* next term buffer in list            */
    TermBuffer          prevBuffer;   /* previous term buffer in list        */
    TermSelectInfo      selectInfo;   /* current select info record          */
    BufferCreateProc    buffer_create_proc;
    BufferResizeProc    buffer_resize_proc;
    BufferFreeProc      buffer_free_proc;
    TermEnhProc         enh_proc;
    TermEnhClear        clear_enh;
    TermEnhInsert       insert_enh;
    TermEnhDelete       delete_enh;
    TermEnhSet          set_enh;
    TermEnhGet          get_enh;
    TermLineSetLen      set_line_len;
    TermLineSetWidth    set_line_width;
    TermLineClear       clear_line;
    Widget		widget;	      /* the widget that created this buffer */
    TermLine           *linesBase;    /* the ring of lines (see LINES_BASE)  */
    int                 linesAlloc;   /* number of TermLines allocated       */
    int                 linesFirst;   /* slot of row 0                       */
} TermBufferPart;

typedef struct _TermBufferRec
{
    TermBufferPart term_buffer;
} TermBufferRec;

/* the slot of the line pointer of a row (see LINES_BASE)... */
static inline TermLine *
_DtTermPrimLineSlot(const struct _TermBufferRec *tb, int row)
{
    int i = tb->term_buffer.linesFirst + row;

    if (i >= tb->term_buffer.maxRows)
    {
	i -= tb->term_buffer.maxRows;
    }
    else if (i < 0)
    {
	i += tb->term_buffer.maxRows;
    }
    return(&tb->term_buffer.linesBase[i]);
}

extern
short
_DtTermPrimBufferInsertWc
(
    const TermBuffer  tb,
    const int         row,
    const short       col,
          wchar_t    *newChars,
          short       numChars,
          Boolean     insertFlag,
          termChar  **returnChars,
          short      *returnLength
);

extern
short
_DtTermPrimBufferInsertWcWidths
(
    const TermBuffer  tb,
    const int         row,
    const short       col,
          wchar_t    *newChars,
    const signed char *rawWidths,
          short       numChars,
          Boolean     insertFlag,
          termChar  **returnChars,
          short      *returnLength
);

extern
Boolean
_DtTermPrimBufferClearLineWc
(
    const TermBuffer  tb,
    const int         row,
          short       newWidth
);

#ifdef	__cplusplus
} /* close scope of 'extern "C"'... */
#endif	/* __cplusplus */

#endif /* _Dt_TermPrimBufferP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
