#ifndef _XukcValue_h
#define _XukcValue_h

/* SCCSID: @(#)Value.h	1.1 (UKC) 5/10/92 */

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

/***********************************************************************
 *
 * UKCValue Widget (subclass of UKC LayoutClass)
 *
 ***********************************************************************/

/************************  Default Widget Resources  *****************

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNborderWidth       BorderWidth        Dimension        0                 
XtNvalueCallback     ValueCallback      Callback         (XtPointer)NULL   
XtNarrowsPosition    ArrowsPosition     ArrowsPosition   EAST              
XtNnoBorder          NoBorder           Boolean          FALSE             
XtNincArrow          Arrow              Widget           NULL              
XtNdecArrow          Arrow              Widget           NULL              
XtNvalueText         ValueText          Widget           NULL              
XtNvalue             Value              Int              0                 
XtNmin               Min                Int              0                 
XtNmax               Max                Int              100               
*/

#include <X11/Xukc/Layout.h>

/* uses children of the following classes */
#include <X11/Xukc/Arrow.h>
#include <X11/Xaw/AsciiText.h>

#ifndef XtNarrowsPosition
#define XtNarrowsPosition "arrowsPosition"
#endif /* XtNarrowsPosition */

#ifndef XtNnoBorder
#define XtNnoBorder "noBorder"
#endif /* XtNnoBorder */

#ifndef XtNincArrow
#define XtNincArrow "incArrow"
#endif /* XtNincArrow */

#ifndef XtNdecArrow
#define XtNdecArrow "decArrow"
#endif /* XtNdecArrow */

#ifndef XtNvalueText
#define XtNvalueText "valueText"
#endif /* XtNvalueText */

#ifndef XtNvalue
#define XtNvalue "value"
#endif /* XtNvalue */

#ifndef XtNmin
#define XtNmin "min"
#endif /* XtNmin */

#ifndef XtNmax
#define XtNmax "max"
#endif /* XtNmax */

#ifndef XtNvalueCallback
#define XtNvalueCallback "valueCallback"
#endif /* XtNvalueCallback */


#ifndef XtCArrowsPosition
#define XtCArrowsPosition "ArrowsPosition"
#endif /* XtCArrowsPosition */

#ifndef XtCNoBorder
#define XtCNoBorder "NoBorder"
#endif /* XtCNoBorder */

#ifndef XtCValue
#define XtCValue "Value"
#endif /* XtCValue */

#ifndef XtCMin
#define XtCMin "Min"
#endif /* XtCMin */

#ifndef XtCMax
#define XtCMax "Max"
#endif /* XtCMax */

#ifndef XtCValueCallback
#define XtCValueCallback "ValueCallback"
#endif /* XtCValueCallback */


#ifndef XtRArrowsPosition
#define XtRArrowsPosition "ArrowsPosition"
#endif /* XtRArrowsPosition */


/* Class record constants */

extern WidgetClass ukcValueWidgetClass;

typedef struct _UKCValueClassRec *UKCValueWidgetClass;
typedef struct _UKCValueRec      *UKCValueWidget;

#endif _XukcValue_h
