#ifndef _XukcPassiveH_h
#define _XukcPassiveH_h

/* SCCSID: @(#)Passive.h	1.1 (UKC) 5/10/92 */

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

/**********************************************************************
 *                          UKCPassive Widget                         *
 *                                                                    *
 * Subclass of compositeWidgetClass : lets children have any geometry *
 * Calls callback routines for <expose> and <resize> events.          *
 **********************************************************************/

/************************  Default Widget Resources  *****************

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNresizeCallback    Callback           Callback         (XtPointer)NULL   
XtNexposeCallback    Callback           Callback         (XtPointer)NULL   
XtNforeground        Foreground         Pixel            XtExtdefaultforeground
XtNcursor            Cursor             Cursor           None              
XtNuseBackingStore   UseBackingStore    Boolean          TRUE
*/

/* resources */

#ifndef XtNexposeCallback
#define XtNexposeCallback "exposeCallback"
#endif /* XtNexposeCallback */

#ifndef XtNresizeCallback
#define XtNresizeCallback "resizeCallback"
#endif /* XtNresizeCallback */

#ifndef XtNforeground
#define XtNforeground "foreground"
#endif /* XtNforeground */

#ifndef XtNcursor
#define XtNcursor "cursor"
#endif /* XtNcursor */

#ifndef XtNuseBackingStore
#define XtNuseBackingStore "useBackingStore"
#endif /* XtNuseBackingStore */


#ifndef XtCUseBackingStore
#define XtCUseBackingStore "UseBackingStore"
#endif /* XtCUseBackingStore */


/* Class record pointer */

extern WidgetClass	ukcPassiveWidgetClass;

/* Widget Type definition */

typedef	struct _UKCPassiveRec	*UKCPassiveWidget;
typedef struct _UKCPassiveClassRec	*UKCPassiveWidgetClass;

#endif	_XukcPassiveH_h
