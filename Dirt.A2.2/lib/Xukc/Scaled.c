#ifndef lint
/* From: Xukc: Scaled.c,v 1.6 91/12/17 10:01:41 rlh2 Rel */
static char sccsid[] = "@(#)Scaled.c	1.2 (UKC) 5/10/92";
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
 *       UKCScaled Composite Widget - scales all children on resize   *
 **********************************************************************/

#define DEFAULT_WIDTH 500
#define DEFAULT_HEIGHT 300

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "ScaledP.h"

/* Full Class Record Constant */

static XtGeometryResult Geometry_handler();
static void Resize();
static void Initialize();
static void constraint_Initialize();

UKCScaledClassRec ukcScaledClassRec = {
  {
    /* core_class fields  */	
    /* superclass	  */	(WidgetClass) &constraintClassRec,
    /* class_name	  */	"UKCScaled",
    /* widget_size	  */	sizeof(UKCScaledRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	XtInheritRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	NULL,
    /* num_resources	  */	0,
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	Resize,
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	NULL,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	XtInheritAcceptFocus,
    /* version		  */	XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table		  */	XtInheritTranslations,
    /* query_geometry	  */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL
  },{
/* composite_class fields */
    /* geometry_handler	  */	Geometry_handler,
    /* change_managed	  */	XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL
  },{
/* constraint_class fields */
    /* resources	  */	NULL,
    /* num_resources	  */	0,
    /* constraint_size	  */	sizeof(UKCScaledConstraintRecord),
    /* initialize	  */	constraint_Initialize,
    /* destroy		  */	NULL,
    /* set_values	  */	NULL,
    /* extension	  */	NULL
  }
};

WidgetClass ukcScaledWidgetClass = (WidgetClass)&ukcScaledClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void
Initialize(request, new)
Widget request, new;
{
	UKCScaledWidget nw = (UKCScaledWidget)new;

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
static XtGeometryResult
Geometry_handler(w, request, reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
	UKCScaledConstraint conns = (UKCScaledConstraint)w->core.constraints;
	UKCScaledWidget sw = (UKCScaledWidget)XtParent(w);

	if (!(request->request_mode & XtCWQueryOnly)) {
		if (request->request_mode & CWX) {
			w->core.x = request->x;
			conns->scaled.x = (double)w->core.x/(double)sw->core.x;
		}
		if (request->request_mode & CWY) {
			w->core.y = request->y;
			conns->scaled.y = (double)w->core.y/(double)sw->core.y;
		}
		if (request->request_mode & CWWidth) {
			w->core.width = request->width;
			conns->scaled.width =
				(double)w->core.width/(double)sw->core.width;
		}
		if (request->request_mode & CWHeight) {
			w->core.height = request->height;
			conns->scaled.height =
				(double)w->core.height/(double)sw->core.height;
		}
		if (request->request_mode & CWBorderWidth)
			w->core.border_width = request->border_width;
	}
	return(XtGeometryYes);
}


/* catch any resize events */
static void
Resize(w)
Widget w;
{
#define CHILD (sw->composite.children[i])
	/* resize all the children proportionally .. if they are too
	 * small to see then unmap then and only map them again then
	 * they can be resized big enough.
	 */
	UKCScaledWidget sw = (UKCScaledWidget)w;
	Cardinal i;

	if (sw->composite.num_children == 0)
		return;

	if (XtIsRealized(w))
		XtUnmapWidget(w);

	for (i = 0; i < sw->composite.num_children; i++) {
		if (XtIsManaged(CHILD)) {
			UKCScaledConstraint conns =
				(UKCScaledConstraint)CHILD->core.constraints;
			Position new_width, new_height;

			new_width = (Position)(conns->scaled.width *
						(double)sw->core.width + 0.5);
			new_height = (Position)(conns->scaled.height *
						(double)sw->core.height + 0.5);

			if (new_width <= 0 || new_height <= 0) {
				if (XtIsRealized(w))
					XtUnmapWidget(CHILD);
			} else {
				if (XtIsRealized(w))
					XtMapWidget(CHILD);

				XtMoveWidget(CHILD, (Position)(conns->scaled.x
					* (double)sw->core.width + 0.5),
						    (Position)(conns->scaled.y
					* (double)sw->core.height + 0.5));
				XtResizeWidget(CHILD,
						(Dimension)new_width,
						(Dimension)new_height,
						CHILD->core.border_width);
			}
		}
	}
	if (XtIsRealized(w))
		XtMapWidget(w);
}


/* ARGSUSED */
static void
constraint_Initialize(request, new)
Widget request, new;
{
	UKCScaledConstraint conns = (UKCScaledConstraint)new->core.constraints;
	Widget p = XtParent(new);

	conns->scaled.width = (double)new->core.width/(double)p->core.width;
	conns->scaled.height = (double)new->core.height/(double)p->core.height;
	conns->scaled.x = (double)new->core.x/(double)p->core.width;
	conns->scaled.y = (double)new->core.y/(double)p->core.height;
}
