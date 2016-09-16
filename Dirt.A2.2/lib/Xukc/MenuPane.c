#ifndef lint
/* From:  Xukc: MenuPane.c,v 1.5 91/12/17 10:01:46 rlh2 Rel */
static char sccsid[] = "@(#)MenuPane.c	1.2 (UKC) 5/10/92";
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
 *	                   UKCMenuPaneWidget			      *
 *                                                                    *
 * Subclass of constraintWidgetClass :looks after its children either *
 *                               horizontally or vertically arranged  *
 **********************************************************************/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "MenuPaneP.h"

#define REFERENCE_DIMENSION(w) \
	(((MPConstraint)w->core.constraints)->menuPane.dimension)
#define NEW_DIMENSION(w) \
	(((MPConstraint)w->core.constraints)->menuPane.new_dimension)

#ifdef DEBUG
static Dimension
reference_dimension(w)
Widget w;
{
	Dimension d;
	MPConstraint mpconns = ((MPConstraint)w->core.constraints);
	d = mpconns->menuPane.dimension;
	return (d);
}
#endif /* DEBUG */

static void Initialize();
static void Resize();
static XtGeometryResult Geometry_handler();
static void Change_managed();
static XtGeometryResult MenuPane_geometry();
static void constraint_Initialize();
static Boolean constraint_SetValues();
static Boolean SetValues();
static Dimension preferred_size();

/*
 * New resources added by this subclass :-
 *
 *  XtNvertPane - if TRUE pane is vertically arranged, FALSE = horizontal.
 *  XtNaskChild - if TRUE the child is asked what size it prefers.
 */
static XtResource resources[] = {
#define offset(field) XtOffset(UKCMenuPaneWidget, menuPane.field)
	{ XtNvertPane, XtCVertPane, XtRBoolean, sizeof(Boolean),
		offset(vert_pane), XtRString, "TRUE" },
	{ XtNaskChild, XtCAskChild, XtRBoolean, sizeof(Boolean),
		offset(ask_child), XtRString, "TRUE" },
	{ XtNresize, XtCResize, XtRBoolean, sizeof(Boolean),
		offset(resize), XtRImmediate, (XtPointer)FALSE },
#undef offset
};

UKCMenuPaneClassRec ukcMenuPaneClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	(WidgetClass) &constraintClassRec,
    /* class_name	  */	"UKCMenuPane",
    /* widget_size	  */	sizeof(UKCMenuPaneRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	XtInheritRealize,
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
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_private   */	NULL,
    /* tm_table           */	NULL,
    /* query_geometry	  */	MenuPane_geometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL
  },
  {
/* composite_class fields */
    /* geometry_handler	  */	Geometry_handler,
    /* change_managed	  */	Change_managed,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL
  },
  {
/* constraint_class fields */
    /* resources          */    NULL,
    /* num_resources      */    0,
    /* constraint_size    */    sizeof(MPConstraintRecord),
    /* initialize         */    constraint_Initialize,
    /* destroy            */    NULL,
    /* set_values         */	constraint_SetValues,
    /* extension	  */	NULL
  },
/* menuPane_class fields (NO NEW ONES) */
};

/* class record */

WidgetClass ukcMenuPaneWidgetClass = (WidgetClass)&ukcMenuPaneClassRec;

/****************************************************************
 *
 * Standard Widget Procedures
 *
 ****************************************************************/

#define SIZE(mpw, cw) \
	(mpw->menuPane.vert_pane ? cw->core.height : cw->core.width)
#define POSITION(mpw, cw) \
	(mpw->menuPane.vert_pane ? cw->core.y : cw->core.x)

#ifdef DEBUG
#undef SIZE
#undef POSITION
static Dimension
SIZE(mpw, cw)
UKCMenuPaneWidget mpw;
Widget cw;
{
	Dimension d =  (mpw->menuPane.vert_pane ? cw->core.height
						: cw->core.width);
	return (d);
}

static Position
POSITION(mpw, cw)
UKCMenuPaneWidget mpw;
Widget cw;
{
	Position p = (mpw->menuPane.vert_pane ? cw->core.y : cw->core.x);
	return (p);
}
#endif /* DEBUG */

/* > Initialize < */
/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCMenuPaneWidget mpw = (UKCMenuPaneWidget)new;

	/* inherit our parent's size if we are given one */
	if (mpw->core.width == 0)
		mpw->core.width = XtParent(mpw)->core.width;

	if (mpw->core.height == 0)
		mpw->core.height = XtParent(mpw)->core.height;
}


/* ARGSUSED */
static void
constraint_Initialize(request, new)
Widget request, new;
{
	UKCMenuPaneWidget mpw = (UKCMenuPaneWidget)XtParent(new); 

	REFERENCE_DIMENSION(new) = SIZE(mpw, new);
}


/* ARGSUSED */
static Boolean
constraint_SetValues(current, request, new)
Widget current, request, new;
{
	UKCMenuPaneWidget mpw = (UKCMenuPaneWidget)XtParent(new); 

	if (SIZE(mpw, current) != SIZE(mpw, new))
		REFERENCE_DIMENSION(new) = SIZE(mpw, new);
	return (FALSE);
}


/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
	UKCMenuPaneWidget npw = (UKCMenuPaneWidget)new; 
	UKCMenuPaneWidget cpw = (UKCMenuPaneWidget)current; 

	if (npw->menuPane.vert_pane != cpw->menuPane.vert_pane)
		Resize(new);
	return (FALSE);
}


/*	> Resize <
 * resize any children
 */
static void Resize(w)
Widget w;
{
	UKCMenuPaneWidget mpw = (UKCMenuPaneWidget) w;
	Cardinal Kids;
	Position offset, spaceleft;
	Dimension oldsize, last_border;

#ifdef DEBUG
	printf("Resize(menuPane)\n");
#undef REFERENCE_DIMENSION
#define REFERENCE_DIMENSION reference_dimension
#endif
	if (mpw->composite.num_children == 0)
		return;
#ifndef DEBUG
	if (XtIsRealized(w))		/* we don't want to see any moving */
		XtUnmapWidget(w);
#endif DEBUG

	oldsize = preferred_size(mpw);
	offset = 0;
	last_border = 0;
	/* resize kids */
	for (Kids = 0; Kids < mpw->composite.num_children; Kids++) {
		Widget kw = mpw->composite.children[Kids];

		if (!XtIsManaged(kw))
			continue;

		NEW_DIMENSION(kw) = (((double)SIZE(mpw, mpw)
				/ (double)oldsize)
				* ((double)REFERENCE_DIMENSION(kw)
				   + kw->core.border_width) + 0.5 -
				   kw->core.border_width);

		last_border = kw->core.border_width;
		offset += NEW_DIMENSION(kw) + last_border;
	}

	if (offset == 0)
		/* no children to move */
		return;

	offset -= last_border;
	spaceleft = SIZE(mpw, mpw) - offset;
	while (spaceleft != 0) {
		Kids = 0;
		while (spaceleft != 0 && Kids < mpw->composite.num_children) {
			Widget kw = mpw->composite.children[Kids];
			Dimension inc;

			if (!XtIsManaged(kw)) {
				Kids++;
				continue;
			}
			inc = spaceleft > 0 ? 1 : -1;
			NEW_DIMENSION(kw) += inc;
			spaceleft -= inc;
			Kids++;
		}
	}

	for (Kids = 0; Kids < mpw->composite.num_children; Kids++)
		if (XtIsManaged(mpw->composite.children[Kids]))
			break;

	offset = - mpw->composite.children[Kids]->core.border_width;
	for (Kids = 0; Kids < mpw->composite.num_children; Kids++) {
		Widget kw = mpw->composite.children[Kids];

		if (!XtIsManaged(kw))
			continue;

		if (NEW_DIMENSION(kw) <= 0)
			NEW_DIMENSION(kw) = 1;

		if (mpw->menuPane.vert_pane) {
			XtResizeWidget(kw, mpw->core.width, NEW_DIMENSION(kw),
					kw->core.border_width);
			XtMoveWidget(kw, -(kw->core.border_width), offset);
			offset += kw->core.height;
		} else {
			XtResizeWidget(kw, NEW_DIMENSION(kw), mpw->core.height,
					kw->core.border_width);
			XtMoveWidget(kw, offset, -(kw->core.border_width));
			offset += kw->core.width;
		}
		offset += kw->core.border_width;
	}
#ifndef DEBUG
	if (XtIsRealized(w))
		XtMapWidget(w);
#endif DEBUG
}


/* ARGSUSED */
static XtGeometryResult Geometry_handler(w, request, reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
	/* Any requests by the children are just refused, the parent
	 * knows best.
	 * If the resource XtNaskChild is set to TRUE then when a
	 * `change managed' event occurs the children are asked their preferred
	 * size.
	 */
	return(XtGeometryNo);
}


static Dimension
preferred_size(mpw)
UKCMenuPaneWidget mpw;
{
	Widget cw = NULL;
	Dimension managed_size = 0;
	Cardinal i;

	/* calculate the total height or width of the children */
	for (i = 0; i < mpw->composite.num_children; i++) {
		if (XtIsManaged(mpw->composite.children[i])) {
			cw = mpw->composite.children[i];
			managed_size += REFERENCE_DIMENSION(cw)
						+ cw->core.border_width;
		}
	}

	if ((managed_size == 0) || (cw == NULL))
		return (0);

	/* remove the last child's border width */
	managed_size -= cw->core.border_width;
	return (managed_size);
}


static void Change_managed(w)
Widget w;
{
	UKCMenuPaneWidget mpw = (UKCMenuPaneWidget)w;
	Dimension new_size = preferred_size(mpw);

#ifdef DEBUG
	printf("Change_managed(menuPane, %s)\n",w->core.name);
#endif
	/* we want the parent to change size so that the children remain
	 * the same size.
	 */
	if (mpw->menuPane.resize) {
		XtGeometryResult result;
		Dimension new_width, new_height;

		new_width = mpw->menuPane.vert_pane ? mpw->core.width
						    : new_size;
		new_height = mpw->menuPane.vert_pane ? new_size
						     : mpw->core.height;
		result = XtMakeResizeRequest(w, new_width, new_height,
				(Dimension *)NULL, (Dimension *)NULL);
	      /* warning: result set but not used in function Change_managed */
	}
	Resize(w);
}


/* > MenuPane_geometry <
 * return the preferred geometry for the menuPane widget
 * The preferred geometry is the width/height of the menuPane and
 * the combined height/width of the children.
 */
static XtGeometryResult MenuPane_geometry(w, request, preferred_return)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *preferred_return;
{
	UKCMenuPaneWidget mpw = (UKCMenuPaneWidget)w;

	preferred_return->request_mode = CWWidth | CWHeight;

	if (mpw->menuPane.vert_pane) {
		preferred_return->height = preferred_size(mpw);
		preferred_return->width = mpw->core.width;
	} else {
		preferred_return->height = mpw->core.height;
		preferred_return->width = preferred_size(mpw);
	}

	/* the size given the caller is not exactly what we want */
	if (((request->request_mode & CWWidth) &&
	   (request->width != preferred_return->width)) ||
	   ((request->request_mode & CWHeight) &&
	   (request->height != preferred_return->height)) ||
	   (preferred_return->request_mode != request->request_mode))
		return (XtGeometryAlmost);

	/* we are already the perfect size */
	if ((mpw->core.height == preferred_return->height) &&
	    (mpw->core.width == preferred_return->width))
		return (XtGeometryNo);

	return (XtGeometryYes);
}
