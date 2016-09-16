#ifndef _UKCArrow_h
#define _UKCArrow_h

/* SCCSID: @(#)Arrow.h	1.1 (UKC) 5/10/92 */

/* 
 * Copyright 1990 Richard Hesketh / rlh2@ukc.ac.uk
 *                Computing Lab. University of Kent at Canterbury, UK
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Richard Hesketh and The University of
 * Kent at Canterbury not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Richard Hesketh and The University of Kent at Canterbury make no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * Richard Hesketh AND THE UNIVERSITY OF KENT AT CANTERBURY DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL Richard Hesketh OR THE
 * UNIVERSITY OF KENT AT CANTERBURY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author:  Richard Hesketh / rlh2@ukc.ac.uk, 
 *                Computing Lab. University of Kent at Canterbury, UK
 */

/****************************************************************
 *
 * UKCArrow widget
 *
 ****************************************************************/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#ifndef XtNthickness
#define XtNthickness "thickness"
#endif /* XtNthickness */

#ifndef XtNorientation
#define XtNorientation "orientation"
#endif /* XtNorientation */

#define XtNfilled "filled"
#define XtNstartTimeout "startTimeout"
#define XtNrepeatTimeout "repeatTimeout"

#ifndef XtCThickness
#define XtCThickness "Thickness"
#endif /* XtCThickness */

#ifndef XtCOrientation
#define XtCOrientation "Orientation"
#endif /* XtCOrientation */

#ifndef XtCFilled
#define XtCFilled "Filled"
#endif /* XtCFilled */

#ifndef XtCTimeout
#define XtCTimeout "Timeout"
#endif /* XtCTimeout */

#ifndef XtRArrowOrientation
#define XtRArrowOrientation "ArrowOrientation"
#endif /* XtRArrowOrientation */

#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

#ifndef XtENorth
#define XtENorth "North"
#endif /* XtENorth */

#ifndef XtESouth
#define XtESouth "South"
#endif /* XtESouth */

#ifndef XtEEast
#define XtEEast "East"
#endif /* XtEEast */

#ifndef XtEWest
#define XtEWest "West"
#endif /* XtEWest */

/* Actions defined in UKCArrowWidget class :
 *
 * arrow() - call routines on XtNarrowCallback list
 * invert() - invert the colour of the arrow (only lasts until a exposure
 *	      event)
 */

/* declare specific UKCArrowWidget class and instance datatypes */

typedef struct _UKCArrowClassRec *UKCArrowWidgetClass;
typedef struct _UKCArrowRec	 *UKCArrowWidget;

/* declare the class constant */

extern WidgetClass ukcArrowWidgetClass;

#endif  _UKCArrow_h
