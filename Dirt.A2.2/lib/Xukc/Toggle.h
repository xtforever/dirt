#ifndef _XukcToggleH_h
#define _XukcToggleH_h

/* SCCSID: @(#)Toggle.h	1.1 (UKC) 5/10/92 */

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

/************************************************************************
 *                            UKCToggle Widget                          *
 *                                                                      *
 *    Subclass of ukcCommandWidgetClass : produces a toggle button      *
 ************************************************************************/

/* superclasses resources */

#include <X11/Xukc/Command.h>

/* resources */

/************************  Default Widget Resources  *****************

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNtoggleAddress     ToggleAddress      ToggleAddress    NULL              
XtNstate             Boolean            Boolean          FALSE             
XtNradioToggle       Widget             Widget           NULL              
XtNradioValue        RadioValue         Int              0                 
XtNinformWhenOff     Boolean		Boolean		 FALSE
XtNonLeft            OnLeft             Boolean          TRUE              
XtNonBitmap          OnBitmap           Bitmap           None              
XtNoffBitmap         OffBitmap          Bitmap           None              
XtNcursor            Cursor             Cursor           "hand1"           
*/

#ifndef XtRWidget /* this should be in StringDefs coz its defined in Form */
#define XtRWidget "Widget"
#endif  /* XtRWidget */


#ifndef XtRToggleAddress
#define XtRToggleAddress "ToggleAddress"
#endif /* XtRToggleAddress */


#ifndef XtNradioToggle
#define XtNradioToggle "radioToggle"
#endif /* XtNradioToggle */

#ifndef XtNradioValue
#define XtNradioValue "radioValue"
#endif /* XtNradioValue */

#ifndef XtNstate
#define XtNstate "state"
#endif /* XtNstate */

#ifndef XtNinformWhenOff
#define XtNinformWhenOff "informWhenOff"
#endif /* XtNinformWhenOff */

#ifndef XtNtoggleAddress
#define XtNtoggleAddress "toggleAddress"
#endif /* XtNtoggleAddress */

#ifndef XtNonLeft
#define XtNonLeft "onLeft"
#endif /* XtNonLeft */

#ifndef XtNonBitmap
#define XtNonBitmap "onBitmap"
#endif /* XtNonBitmap */

#ifndef XtNoffBitmap
#define XtNoffBitmap "offBitmap"
#endif /* XtNoffBitmap */


#ifndef XtCToggleAddress
#define XtCToggleAddress "ToggleAddress"
#endif /* XtCToggleAddress */

#ifndef XtCRadioValue
#define XtCRadioValue "RadioValue"
#endif /* XtCRadioValue */

#ifndef XtCOnLeft
#define XtCOnLeft "OnLeft"
#endif /* XtCOnLeft */

#ifndef XtCOnBitmap
#define XtCOnBitmap "OnBitmap"
#endif /* XtCOnBitmap */

#ifndef XtCOffBitmap
#define XtCOffBitmap "OffBitmap"
#endif /* XtCOffBitmap */

#ifndef XtCWidget
#define XtCWidget "Widget"
#endif /* XtCWidget */


/* Class record pointer */

extern WidgetClass	ukcToggleWidgetClass;

/* Widget Type definition */

typedef	struct _UKCToggleRec	*UKCToggleWidget;
typedef struct _UKCToggleClassRec	*UKCToggleWidgetClass;

/* public routines defined in the toggle class */

/* toggle current state */
void XukcDoToggle(/* w, do_calls */);
/* Widget w;
   Boolean do_calls; */     /* Callback is called if TRUE */

/* explicitly set state */
void XukcSetToggle(/* w, do_calls */);
/* Widget w;
   Boolean do_calls; */     /* Callback is called if TRUE */

/* explicitly reset state */
void XukcUnsetToggle(/* w, do_calls */);
/* Widget w;
   Boolean do_calls; */     /* Callback is called if TRUE */

void XukcAddRadioToggle(/* w1, w2 */);
/* Widget w1, w2; */	/* widgets to join as radio toggle buttons */

void XukcRemoveRadioToggle(/* w */);
/* Widget w; */

UKCToggleWidget XukcGetRadioSet(/* w */);
/* Widget w; */			/* returns the radio toggle currently set */
#endif	_XukcToggleH_h
