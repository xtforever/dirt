/* SCCSID: @(#)Command.h	1.1 (UKC) 5/10/92 */


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

#ifndef _XukcCommand_h
#define _XukcCommand_h

/***********************************************************************
 *
 * UKCCommand Widget
 *
 ***********************************************************************/

#include <X11/Xukc/Label.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 bitmap		     Pixmap		Pixmap		None
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 callback	     Callback		Pointer		NULL
 cursor		     Cursor		Cursor		"hand2"
 destroyCallback     Callback		Pointer		NULL
 font		     Font		XFontStruct *	XtDefaultFont
 foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	text height
 highlightColor	     HighlightColor	Pixel		XtDefaultForeground
 highlightThickness  Thickness		Dimension	2
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

*/

#ifndef XtNcallback
#define XtNcallback "callback"
#endif /* XtNcallback */

#ifndef XtNhighlightThickness
#define XtNhighlightThickness "highlightThickness"
#endif /* XtNhighlightThickness */

#ifndef XtNhighlightColor
#define XtNhighlightColor "highlightColor"
#endif /* XtNhighlightColor */

#ifndef XtNtranslations
#define XtNtranslations "translations"
#endif /* XtNtranslations */


extern WidgetClass     ukcCommandWidgetClass;

typedef struct _UKCCommandClassRec   *UKCCommandWidgetClass;
typedef struct _UKCCommandRec        *UKCCommandWidget;


#endif _XukcCommand_h
/* DON'T ADD STUFF AFTER THIS */
