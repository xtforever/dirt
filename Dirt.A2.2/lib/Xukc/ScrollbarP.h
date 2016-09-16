#ifndef _UKCScrollP_h
#define _UKCScrollP_h

/* SCCSID: @(#)ScrollbarP.h	1.1 (UKC) 5/10/92 */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <X11/Xukc/Scrollbar.h>
#include <X11/CoreP.h>

typedef struct _marklist {
	double	start_percent;
	double	end_percent;
	Boolean	dead;
	Boolean	changed;
	struct _marklist *next;
} MarkListRec;

typedef struct _marklist *MarkList;

typedef struct _ukcScrollbarPart {
     /* public */
    Pixel	  foreground;	/* thumb foreground color */
    XtOrientation orientation;	/* horizontal or vertical */
    XtCallbackList scrollProc;	/* proportional scroll */
    XtCallbackList thumbProc;	/* jump (to position) scroll */
    XtCallbackList jumpProc;	/* same as thumbProc but pass data by ref */
    Pixmap	  thumb;	/* thumb color */
    Cursor	  upCursor;	/* scroll up cursor */
    Cursor	  downCursor;	/* scroll down cursor */
    Cursor	  leftCursor;	/* scroll left cursor */
    Cursor	  rightCursor;	/* scroll right cursor */
    Cursor	  verCursor;	/* scroll vertical cursor */
    Cursor	  horCursor;	/* scroll horizontal cursor */
    float	  top;		/* What percent is above the win's top */
    float	  shown;	/* What percent is shown in the win */
    Dimension	  length;	/* either height or width */
    Dimension	  thickness;	/* either width or height */
    Boolean	  centre;	/* whether to centre the scrollbar on cursor */
     /* private */
    Cursor	  inactiveCursor; /* The normal cursor for scrollbar */
    char	  direction;	/* a scroll has started; which direction */
    GC		  gc;		/* a (shared) gc */
    Position	  topLoc;	/* Pixel that corresponds to top */
    Dimension	  shownLength;	/* Num pixels corresponding to shown */
    Boolean	  reverse_video; /* display in reverse video */

    /* added resources for thumb marks */
    /* public */
    Dimension	  markSize;	/* size of the marks */
    Pixel	  markColor;	/* colour of the marks */
    Boolean	  fillMarks;	/* TRUE = filled marks, FALSE = hollow */

    /* private */
    GC		  markGC;	/* GC for the thumb marks */
    GC		  removeGC;	/* GC to remove the thumb marks */
    MarkList	  markList;	/* linked list of thumb marks */
    Boolean	  marks_changed; /* mark list has been updated */
} UKCScrollbarPart;


typedef struct _UKCScrollbarRec {
    CorePart		core;
    UKCScrollbarPart	scrollbar;
} UKCScrollbarRec;


typedef struct {int empty;} UKCScrollbarClassPart;


typedef struct _UKCScrollbarClassRec {
    CoreClassPart		core_class;
    UKCScrollbarClassPart		scrollbar_class;
} UKCScrollbarClassRec;

extern UKCScrollbarClassRec ukcScrollbarClassRec;

#endif _UKCScrollP_h
