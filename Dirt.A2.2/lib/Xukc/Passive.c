#ifndef lint
/* From: Xukc: Passive.c,v 1.6 91/12/17 10:01:50 rlh2 Rel */
static char sccsid[] = "@(#)Passive.c	1.2 (UKC) 5/10/92";
#endif /* !lint */

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
 *       UKCPassive Composite Widget - with added YES geometry!       *
 **********************************************************************/

#define DEFAULT_WIDTH 500
#define DEFAULT_HEIGHT 300

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "PassiveP.h"

/* Full Class Record Constant */

static void Initialize();
static void Realize();
static XtGeometryResult Geometry_handler();
static void Resize();
static void Redisplay();
static Boolean SetValues();

static XtResource resources[] = {
#define offset(field) XtOffset(UKCPassiveWidget, passive.field)
	{ XtNresizeCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(resize_callbacks), XtRCallback, (XtPointer)NULL},
	{ XtNexposeCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(expose_callbacks), XtRCallback, (XtPointer)NULL},
	{ XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	  offset(foreground), XtRString, (XtPointer)XtExtdefaultforeground},
	{ XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
	  offset(cursor), XtRImmediate, (XtPointer)None},
	{ XtNuseBackingStore, XtCUseBackingStore, XtRBoolean,
	  sizeof(Boolean), offset(use_bs), XtRImmediate, (XtPointer)TRUE},
};

UKCPassiveClassRec ukcPassiveClassRec = {
  {
    /* core_class fields  */	
    /* superclass	  */	(WidgetClass) &compositeClassRec,
    /* class_name	  */	"UKCPassive",
    /* widget_size	  */	sizeof(UKCPassiveRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	Realize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	Resize,
    /* expose		  */	Redisplay,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	XtInheritAcceptFocus,
    /* version		  */	XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table		  */	XtInheritTranslations,
    /* query_geometry	  */	NULL,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL
  },
  {
/* composite_class fields */
    /* geometry_handler	  */	Geometry_handler,
    /* change_managed	  */	XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL
  }
};

WidgetClass ukcPassiveWidgetClass = (WidgetClass)&ukcPassiveClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void
Initialize(request, new)
Widget request, new;
{
        UKCPassiveWidget nw = (UKCPassiveWidget)new;

        if (nw->core.width == 0) {
                if (nw->core.parent->core.width > 0)
                        nw->core.width = nw->core.parent->core.width;
                else
                        nw->core.width = DEFAULT_WIDTH;
        }
        if (nw->core.height == 0) {
                if (nw->core.parent->core.height > 0)
                        nw->core.height = nw->core.parent->core.height;
                else
                        nw->core.height = DEFAULT_HEIGHT;
        }
}


/* ARGSUSED */
static XtGeometryResult Geometry_handler(w, request, reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
	/* replace this with any particular handler you want */
	if (!(request->request_mode & XtCWQueryOnly))
	{
		if (request->request_mode & CWX)
			w->core.x = request->x;
		if (request->request_mode & CWY)
			w->core.y = request->y;
		if (request->request_mode & CWWidth)
			w->core.width = request->width;
		if (request->request_mode & CWHeight)
			w->core.height = request->height;
		if (request->request_mode & CWBorderWidth)
			w->core.border_width = request->border_width;
	}
	return(XtGeometryYes);
}


/* catch any resize events */
static void Resize(w)
Widget w;
{
	XtCallCallbacks(w, XtNresizeCallback, (XtPointer)NULL);
}

/* catch any exposure events and pass them to redisplay routines registered
 * with the XtNexposeCallback.  The newly exposed (possibly composite)
 * region is passed to the callback routines as call_data.
 */
/* ARGSUSED */
static void
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
	XtCallCallbacks(w, XtNexposeCallback, (XtPointer)region);
}

static void
Realize(w, valueMask, attributes)
Widget w;
XtValueMask *valueMask;
XSetWindowAttributes *attributes;
{
	if ((attributes->cursor = ((UKCPassiveWidget)w)->passive.cursor)
	    != None)
		*valueMask |= CWCursor;

	if (((UKCPassiveWidget)w)->passive.use_bs) {
		/* use backing store and save unders if they are available */
		attributes->backing_store = WhenMapped;
		attributes->save_under = TRUE;
		*valueMask |= CWBackingStore | CWSaveUnder;
	}
	XtCreateWindow(w, InputOutput, (Visual *)CopyFromParent,
					(XtValueMask)*valueMask, attributes);
}


/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
	UKCPassiveWidget old = (UKCPassiveWidget)current;
	UKCPassiveWidget nw = (UKCPassiveWidget)new;

	if ((old->passive.cursor != nw->passive.cursor) && XtIsRealized(new)
	     && nw->passive.cursor != None)
		XDefineCursor(XtDisplay(nw), XtWindow(nw), nw->passive.cursor);
	return FALSE;
}
