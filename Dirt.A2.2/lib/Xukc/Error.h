#ifndef _XukcError_h
#define _XukcError_h

/* SCCSID: @(#)Error.h	1.1 (UKC) 5/10/92 */

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
 * UKCError Widget (subclass of Athena Form WidgetClass)
 *
 ***********************************************************************/

#include <X11/Xaw/Form.h>

/* uses children of the following classes */
#include <X11/Xaw/AsciiText.h>
#include <X11/Xukc/Command.h>
#include <X11/Xukc/Label.h>

/************************  Default Widget Resources  *****************

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNerrorTitle        ErrorTitle         String           NULL              
XtNerrorCallback     Callback           Callback         (XtPointer)NULL   
XtNhandleXtErrors    Boolean            Boolean          TRUE              
XtNhandleXlibErrors  Boolean            Boolean          TRUE              
XtNhistoryList       Boolean            Boolean          TRUE              
XtNpauseOnError      Boolean            Boolean          FALSE             
XtNbeepOnError       Boolean            Boolean          TRUE              
XtNbeepVolume        Volume             Int              -50               
*/

#ifndef XtNerrorCallback
#define XtNerrorCallback "errorCallback"
#endif /* XtNerrorCallback */

#ifndef XtNhandleXtErrors
#define XtNhandleXtErrors "handleXtErrors"
#endif /* XtNhandleXtErrors */

#ifndef XtNhandleXlibErrors
#define XtNhandleXlibErrors "handleXlibErrors"
#endif /* XtNhandleXlibErrors */

#ifndef XtNhistoryList
#define XtNhistoryList "historyList"
#endif /* XtNhistoryList */

#ifndef XtNpauseOnError
#define XtNpauseOnError "pauseOnError"
#endif /* XtNpauseOnError */

#ifndef XtNbeepOnError
#define XtNbeepOnError "beepOnError"
#endif /* XtNbeepOnError */

#ifndef XtNbeepVolume
#define XtNbeepVolume "beepVolume"
#endif /* XtNbeepVolume */

#ifndef XtNerrorTitle
#define XtNerrorTitle "errorTitle"
#endif /* XtNerrorTitle */

#ifndef XtNcurrentHandler
#define XtNcurrentHandler "currentHandler"
#endif /* XtNcurrentHandler */


#ifndef XtCVolume
#define XtCVolume "Volume"
#endif /* XtCVolume */

#ifndef XtCErrorTitle
#define XtCErrorTitle "ErrorTitle"
#endif /* XtCErrorTitle */


/* Class record constants */

extern WidgetClass ukcErrorWidgetClass;

typedef struct _UKCErrorClassRec *UKCErrorWidgetClass;
typedef struct _UKCErrorRec      *UKCErrorWidget;

/* Public functions */

extern void XukcGiveErrorMessage(/* w, message, append */);
/* UKCErrorWidget w;		- error widget
 * String message;		- message to output
 * Boolean append;		- if TRUE message is added to existing ones
 */

#endif _XukcError_h
