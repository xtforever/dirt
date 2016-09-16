#ifndef _UKCScroll_h
#define _UKCScroll_h

/* SCCSID: @(#)Scrollbar.h	1.1 (UKC) 5/10/92 */

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

/****************************************************************
 *
 * UKCScrollbar Widget with Thumb markings
 *
 ****************************************************************/


/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		White
 border		     BorderColor	Pixel		Black
 borderWidth	     BorderWidth	Dimension	1
 centre		     Centre		Boolean		False
 destroyCallback     Callback		Function	NULL
 fillMarks	     FillMarks		Boolean		TRUE
 foreground	     Color		Pixel		Black
 height		     Height		Dimension	length or thickness
 jumpProc	     Callback		Function	NULL
 length		     Length		Dimension	1
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 markColor	     Color		Pixel		Black
 markSize	     Thickness		Dimension	7
 orientation	     Orientation	XtOrientation	XtorientVertical
 reverseVideo	     ReverseVideo	Boolean		False
 scrollDCursor	     Cursor		Cursor		XC_sb_down_arrow
 scrollHCursor	     Cursor		Cursor		XC_sb_h_double_arrow
 scrollLCursor	     Cursor		Cursor		XC_sb_left_arrow
 scrollProc	     Callback		Function	NULL
 scrollRCursor	     Cursor		Cursor		XC_sb_right_arrow
 scrollUCursor	     Cursor		Cursor		XC_sb_up_arrow
 scrollVCursor	     Cursor		Cursor		XC_sb_v_double_arrow
 sensitive	     Sensitive		Boolean		True
 shown		     Shown		float		0.0
 thickness	     Thickness		Dimension	14
 thumb		     Thumb		Pixmap		Grey
 thumbProc	     Callback		Function	NULL
 top		     Top		float		0.0
 width		     Width		Dimension	thickness or length
 x		     Position		Position	0
 y		     Position		Position	0

*/

#include <X11/Xmu/Xmu.h>

#ifndef XtNfillMarks
#define XtNfillMarks "fillMarks"
#endif /* XtNfillMarks */

#ifndef XtNjumpProc
#define XtNjumpProc "jumpProc"
#endif /* XtNjumpProc */

#ifndef XtNlength
#define XtNlength "length"
#endif /* XtNlength */

#ifndef XtNmarkColor
#define XtNmarkColor "markColor"
#endif /* XtNmarkColor */

#ifndef XtNmarkSize
#define XtNmarkSize "markSize"
#endif /* XtNmarkSize */

#ifndef XtNorientation
#define XtNorientation "orientation"
#endif /* XtNorientation */

#ifndef XtNscrollProc
#define XtNscrollProc "scrollProc"
#endif /* XtNscrollProc */

#ifndef XtNscrollDCursor
#define XtNscrollDCursor "scrollDCursor"
#endif /* XtNscrollDCursor */

#ifndef XtNscrollHCursor
#define XtNscrollHCursor "scrollHCursor"
#endif /* XtNscrollHCursor */

#ifndef XtNscrollLCursor
#define XtNscrollLCursor "scrollLCursor"
#endif /* XtNscrollLCursor */

#ifndef XtNscrollRCursor
#define XtNscrollRCursor "scrollRCursor"
#endif /* XtNscrollRCursor */

#ifndef XtNscrollUCursor
#define XtNscrollUCursor "scrollUCursor"
#endif /* XtNscrollUCursor */

#ifndef XtNscrollVCursor
#define XtNscrollVCursor "scrollVCursor"
#endif /* XtNscrollVCursor */

#ifndef XtNshown
#define XtNshown "shown"
#endif /* XtNshown */

#ifndef XtNthickness
#define XtNthickness "thickness"
#endif /* XtNthickness */

#ifndef XtNthumb
#define XtNthumb "thumb"
#endif /* XtNthumb */

#ifndef XtNthumbProc
#define XtNthumbProc "thumbProc"
#endif /* XtNthumbProc */

#ifndef XtNtop
#define XtNtop "top"
#endif /* XtNtop */

#ifndef XtNcentre
#define XtNcentre "centre"
#endif /* XtNcentre */


#ifndef XtCFillMarks
#define XtCFillMarks "FillMarks"
#endif /* XtCFillMarks */

#ifndef XtCShown
#define XtCShown "Shown"
#endif /* XtCShown */

#ifndef XtCTop
#define XtCTop "Top"
#endif /* XtCTop */

#ifndef XtCCentre
#define XtCCentre "Centre"
#endif /* XtCCentre */


#ifndef XtRFloat
#define XtRFloat "Float"
#endif /* XtRFloat */


typedef struct _UKCScrollbarRec	  *UKCScrollbarWidget;
typedef struct _UKCScrollbarClassRec *UKCScrollbarWidgetClass;

extern WidgetClass ukcScrollbarWidgetClass;

void XukcScrollbarSetThumb(); /* scrollBar, top, shown */
/* Widget scrollBar; */
/* float top, shown; */

typedef struct _publicmark {
	double	start_percent;
	double	end_percent;
} *PubMark;

PubMark XukcScrollbarAddMark();
PubMark XukcScrollbarUpdateMark(/* w, mark, start, end */);
void XukcScrollbarDisplayMarks();
void XukcScrollbarRemoveMark();
void XukcScrollbarResetMarks();
void XukcScrollbarSetThumb();

#endif _UKCScroll_h
