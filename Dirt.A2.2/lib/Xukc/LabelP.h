/*
* Derived from
* XConsortium: LabelP.h,v 1.22 89/03/30 16:05:35 jim Exp
*
* $Xukc: LabelP.h,v 1.9 91/12/17 10:01:33 rlh2 Rel $
*/


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

/* 
 * LabelP.h - Private definitions for UKCLabel widget
 * 
 */

#ifndef _XukcLabelP_h
#define _XukcLabelP_h

/***********************************************************************
 *
 * UKCLabel Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SimpleP.h>
#include <X11/Xukc/Label.h>

typedef void (*XukcUpdateBitmapProc)(
#if NeedFunctionPrototypes
	Widget,  /* w */
	Pixmap,  /* new_bitmap */
	Boolean  /* is_left */
#endif
);

#define XtInheritUpdateBitmap ((XukcUpdateBitmapProc) _XtInherit)

/* New fields for the UKCLabel widget class record */

typedef struct _UKCLabelClassPart{
	XukcUpdateBitmapProc	update_bitmap;
} UKCLabelClassPart;

/* Full class record declaration */
typedef struct _UKCLabelClassRec {
	CoreClassPart	core_class;
	SimpleClassPart	simple_class;
	UKCLabelClassPart	label_class;
} UKCLabelClassRec;

extern UKCLabelClassRec ukcLabelClassRec;

/* New fields for the UKCLabel widget record */
typedef struct _UKCLabelPart {
    /* public resources */
    Pixel	foreground;
    XFontStruct	*font;
    String	label;
    String	old_label;
    XtJustify	justify;
    Dimension	internal_width;
    Dimension	internal_height;
    Pixmap	bitmap;
    Pixmap	left_bitmap;
    Pixmap	right_bitmap;
    Pixmap	pixmap;
    Pixmap	left_pixmap;
    Pixmap	right_pixmap;
    Boolean	resize;

    /* resources controlling editing of label */
    Boolean	is_editable;
    Boolean	editing;
    Boolean	overflow_right;
    Boolean	overflow_top;
    Boolean	grab_keyboard;
    Pixel	bar_colour;
    XtCallbackList	edit_callback;
    int		vert_justify;

    /* private state */
    GC		normal_GC;
    GC          gray_GC;
    GC          stipple_GC;
    GC		bar_GC;
    Position	bar_x;
    Position	bar_y;
    Cardinal	bar_char;
    Position	label_x;
    Position	label_y;
    Dimension	label_width;
    Dimension	label_height;
    Dimension	label_len;
    Dimension	left_width;
    Dimension	left_height;
    Dimension	left_depth;
    Region	left_region;
    Dimension	right_width;
    Dimension	right_height;
    Dimension	right_depth;
    Region	right_region;
} UKCLabelPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _UKCLabelRec {
    CorePart	core;
    SimplePart	simple;
    UKCLabelPart	label;
} UKCLabelRec;

#endif /* _XukcLabelP_h */
