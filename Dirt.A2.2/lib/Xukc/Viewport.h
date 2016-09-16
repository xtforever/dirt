/*
 * SCCSID: @(#)Viewport.h	1.2 (UKC) 5/10/92 
 * Public definitions for UKCViewport widget
 */

/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

********************************************************/

#ifndef _UKCViewport_h
#define _UKCViewport_h

#include <X11/Xaw/Form.h>
#include <X11/Xukc/Scrollbar.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 allowHoriz	     Boolean		Boolean		False
 allowVert	     Boolean		Boolean		False
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 forceBars	     Boolean		Boolean		False
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 useBottom	     Boolean		Boolean		False
 useRight	     Boolean		Boolean		False
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0
XtNhorizScroll       ReadOnly           Widget          NULL              
XtNvertScroll        ReadOnly           Widget          NULL              
XtNclipWidget        ReadOnly           Widget          NULL              
XtNscrolledChild     ReadOnly           Widget          NULL              
XtNhorizAutoScroll   AutoScroll		Boolean		FALSE
XtNvertAutoScroll    AutoScroll		Boolean		FALSE

*/

/* fields added to Form */
#ifndef XtNforceBars
#define XtNforceBars "forceBars"
#endif /* XtNforceBars */

#ifndef XtNallowHoriz
#define XtNallowHoriz "allowHoriz"
#endif /* XtNallowHoriz */

#ifndef XtNallowVert
#define XtNallowVert "allowVert"
#endif /* XtNallowVert */

#ifndef XtNuseBottom
#define XtNuseBottom "useBottom"
#endif /* XtNuseBottom */

#ifndef XtNuseRight
#define XtNuseRight "useRight"
#endif /* XtNuseRight */


#ifndef XtNhorizAutoScroll
#define XtNhorizAutoScroll "horizAutoScroll"
#endif /* XtNhorizAutoScroll */

#ifndef XtNvertAutoScroll
#define XtNvertAutoScroll "vertAutoScroll"
#endif /* XtNvertAutoScroll */

#ifndef XtNhorizScroll
#define XtNhorizScroll "horizScroll"
#endif /* XtNhorizScroll */

#ifndef XtNvertScroll
#define XtNvertScroll "vertScroll"
#endif /* XtNvertScroll */

#ifndef XtNclipWidget
#define XtNclipWidget "clipWidget"
#endif /* XtNclipWidget */

#ifndef XtNscrolledChild
#define XtNscrolledChild "scrolledChild"
#endif /* XtNscrolledChild */


#ifndef XtCAutoScroll
#define XtCAutoScroll "AutoScroll"
#endif /* XtCAutoScroll */


extern WidgetClass ukcViewportWidgetClass;

typedef struct _UKCViewportClassRec *UKCViewportWidgetClass;
typedef struct _UKCViewportRec	 *UKCViewportWidget;

/* Public Routines */
void XukcVScrollSetValues(/* w, horiz_scroll, args, num_args */);
/*   Widget w;
     Boolean horiz_scroll;
     ArgList args;
     Cardinal num_args;
*/

Widget XukcViewScrollbar(/* w, horiz_scroll */);

#endif _UKCViewport_h
