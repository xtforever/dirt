#ifndef _XukcMouseH_h
#define _XukcMouseH_h

/* From: Xukc: Mouse.h,v 1.4 91/12/17 10:01:29 rlh2 Rel */
/* SCCSID: @(#)Mouse.h	1.2 (UKC) 5/10/92 */

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

/*****************************************************************************
 *  	     UKCMouse Hole Widget (subclass of simplewidgetClass)	     *
 *****************************************************************************/

/************************  Default Widget Resources  *****************

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNfont              Font               FontStruct       XtDefaultfont  
XtNforeground        Foreground         Pixel            XtExtdefaultforeground
XtNbuttonDown        ButtonDown         Int              0                 
XtNupdateRate        UpdateRate         Int              DEFAULT_RATE      
XtNcursor            Cursor             Cursor           "mouse"           
*/

/*
 * New Resources/Parameters :
 */ 

#define MH_No_Function		" "
#define MH_Left_Button		1
#define MH_Middle_Button	2
#define MH_Right_Button		4

#ifndef XtNbuttonDown
#define XtNbuttonDown "buttonDown"
#endif /* XtNbuttonDown */

#ifndef XtNupdateRate
#define XtNupdateRate "updateRate"
#endif /* XtNupdateRate */


#ifndef XtCButtonDown
#define XtCButtonDown "ButtonDown"
#endif /* XtCButtonDown */

#ifndef XtCUpdateRate
#define XtCUpdateRate "UpdateRate"
#endif /* XtCUpdateRate */


/* Class record pointer */

extern WidgetClass ukcMouseWidgetClass;

/* C Widget Type definition */

typedef	struct _UKCMouseRec *UKCMouseWidget;
typedef struct _UKCMouseClassRec *UKCMouseWidgetClass;

/* Functions added by UKCMouse Widget */

extern void XukcMouseAddClient(/* mw, w, left, middle, right */);
/* UKCMouseWidget mw;
 * Widget w;
 * String left, middle, right;
 */
extern void XukcMouseRemoveClient(/* mw, w, remove_children */);
/* UKCMouseWidget mw;
 * Widget w;
 * Boolean remove_children;
 */

#define XukcMouseBlankClient(mw, w) \
    XukcMouseAddClient(mw, w, MH_No_Function, MH_No_Function, MH_No_Function)

#endif	_XukcMouseH_h
