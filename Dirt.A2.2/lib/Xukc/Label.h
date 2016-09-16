/* From: Xukc: Label.h,v 1.6 91/12/17 10:02:08 rlh2 Rel */
/* SCCSID: @(#)Label.h	1.2 (UKC) 5/10/92 */

/*
 * Derived from
 * MIT/Athena Label Widget
 *
 * Enhancements and improvements by Richard Hesketh : rlh2@ukc.ac.uk
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

#ifndef _XukcLabel_h
#define _XukcLabel_h

/***********************************************************************
 *
 * UKCLabel Widget
 *
 *
 ***********************************************************************/

#include <X11/Xaw/Simple.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 bitmap		     Pixmap		Pixmap		None
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 cursor		     Cursor		Cursor		None
 destroyCallback     Callback		XtCallbackList	NULL
 font		     Font		XFontStruct*	XtDefaultFont
 foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	text height
 insensitiveBorder   Insensitive	Pixmap		Gray
 internalHeight	     Height		Dimension	2
 internalWidth	     Width		Dimension	4
 justify	     Justify		XtJustify	XtJustifyCenter
 label		     Label		String		NULL
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 resize		     Resize		Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	text width
 x		     Position		Position	0
 y		     Position		Position	0

* Xukc additions *

XtNleftBitmap        LeftBitmap         Bitmap          None              
XtNrightBitmap       RightBitmap        Bitmap          None              
XtNpixmap	     Pixmap		Pixmap		None
XtNleftPixmap        LeftPixmap         Pixmap          None              
XtNrightPixmap       RightPixmap        Pixmap          None              
XtNoverflowRight     OverflowRight      Boolean         FALSE             
XtNoverflowTop	     OverflowTop	Boolean		FALSE
XtNverticalJustify   VerticalJustify    integer		XukcJustifyCenter
XtNeditable          Editable           Boolean         TRUE              
XtNbarColour         Foreground         Pixel           XtExtdefaultforeground
XtNediting           Editing            Boolean         FALSE             
XtNeditCallback      Callback           Callback        (XtPointer)NULL   
XtNgrabKeyboard      GrabKeyboard       Boolean         FALSE             
XtNoldLabel	     OldLabel		String		NULL

*/

#include <X11/Xmu/Converters.h>

#ifndef XtNbitmap
#define XtNbitmap "bitmap"
#endif /* XtNbitmap */

#ifndef XtNpixmap
#define XtNpixmap "pixmap"
#endif /* XtNpixmap */

#ifndef XtNforeground
#define XtNforeground "foreground"
#endif /* XtNforeground */

#ifndef XtNlabel
#define XtNlabel "label"
#endif /* XtNlabel */

#ifndef XtNleftBitmap
#define XtNleftBitmap "leftBitmap"
#endif /* XtNleftBitmap */

#ifndef XtNleftPixmap
#define XtNleftPixmap "leftPixmap"
#endif /* XtNleftPixmap */

#ifndef XtNfont
#define XtNfont "font"
#endif /* XtNfont */

#ifndef XtNinternalWidth
#define XtNinternalWidth "internalWidth"
#endif /* XtNinternalWidth */

#ifndef XtNinternalHeight
#define XtNinternalHeight "internalHeight"
#endif /* XtNinternalHeight */

#ifndef XtNresize
#define XtNresize "resize"
#endif /* XtNresize */

#ifndef XtNrightBitmap
#define XtNrightBitmap "rightBitmap"
#endif /* XtNrightBitmap */

#ifndef XtNrightPixmap
#define XtNrightPixmap "rightPixmap"
#endif /* XtNrightPixmap */


#ifndef XtNeditable
#define XtNeditable "editable"
#endif /* XtNeditable */

#ifndef XtNediting
#define XtNediting "editing"
#endif /* XtNediting */

#ifndef XtNbarColour
#define XtNbarColour "barColour"
#endif /* XtNbarColour */

#ifndef XtNeditCallback
#define XtNeditCallback "editCallback"
#endif /* XtNeditCallback */

#ifndef XtNoverflowRight
#define XtNoverflowRight "overflowRight"
#endif /* XtNoverflowRight */

#ifndef XtNgrabKeyboard
#define XtNgrabKeyboard "grabKeyboard"
#endif /* XtNgrabKeyboard */

#ifndef XtNoverflowTop
#define XtNoverflowTop "overflowTop"
#endif /* XtNoverflowTop */

#ifndef XtNverticalJustify
#define XtNverticalJustify "verticalJustify"
#endif /* XtNverticalJustify */

#ifndef XtNoldLabel
#define XtNoldLabel "oldLabel"
#endif /* XtNoldLabel */


#ifndef XtCBitmap
#define XtCBitmap "Bitmap"
#endif /* XtCBitmap */

#ifndef XtCLeftBitmap
#define XtCLeftBitmap "LeftBitmap"
#endif /* XtCLeftBitmap */

#ifndef XtCRightBitmap
#define XtCRightBitmap "RightBitmap"
#endif /* XtCRightBitmap */

#ifndef XtCPixmap
#define XtCPixmap "Pixmap"
#endif /* XtCPixmap */

#ifndef XtCLeftPixmap
#define XtCLeftPixmap "LeftPixmap"
#endif /* XtCLeftPixmap */

#ifndef XtCRightPixmap
#define XtCRightPixmap "RightPixmap"
#endif /* XtCRightPixmap */

#ifndef XtCResize
#define XtCResize "Resize"
#endif /* XtCResize */


#ifndef XtCEditable
#define XtCEditable "Editable"
#endif /* XtCEditable */

#ifndef XtCEditing
#define XtCEditing "Editing"
#endif /* XtCEditing */

#ifndef XtCOverflowRight
#define XtCOverflowRight "OverflowRight"
#endif /* XtCOverflowRight */

#ifndef XtCOverflowTop
#define XtCOverflowTop "OverflowTop"
#endif /* XtCOverflowTop */

#ifndef XtCVerticalJustify
#define XtCVerticalJustify "VerticalJustify"
#endif /* XtCVerticalJustify */

#ifndef XtCGrabKeyboard
#define XtCGrabKeyboard "GrabKeyboard"
#endif /* XtCGrabKeyboard */

#ifndef XtCOldLabel
#define XtCOldLabel "OldLabel"
#endif /* XtCOldLabel */


#ifndef XtRVerticalJustify
#define XtRVerticalJustify "VerticalJustify"
#endif /* XtRVerticalJustify */


#ifndef XukcJustifyTop
#define XukcJustifyTop "top"
#endif /* XukcJustifyTop */

#ifndef XukcJustifyCenter
#define XukcJustifyCenter "center"
#endif /* XukcJustifyCenter */

#ifndef XukcJustifyBottom
#define XukcJustifyBottom "bottom"
#endif /* XukcJustifyBottom */


#define XukcEtop 0
#define XukcEcenter 1
#define XukcEbottom 2

/* Class record constants */

extern WidgetClass ukcLabelWidgetClass;

typedef struct _UKCLabelClassRec *UKCLabelWidgetClass;
typedef struct _UKCLabelRec      *UKCLabelWidget;

#endif /* _XukcLabel_h */
/* DON'T ADD STUFF AFTER THIS #endif */
