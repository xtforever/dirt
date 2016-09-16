#ifndef lint
static char sccsid[] = "@(#)Layout.c	1.2 (UKC) 5/10/92";
#endif /* !lint */

/* ||| Kludge list ||| */
#define SHELL_KLUDGE 1

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
 *			    UKCLayout Widget			      *
 *								      *
 * Subclass of constraintWidgetClass : constrains children by the use *
 * of horizontal and vertical connection lists. These lists are held  *
 * as a tree structure with the UKCLayoutWidget as the root node.     *
 **********************************************************************/

#define	XtStrlen(s)	((s) ? strlen(s) : 0)

#define NOT_RESIZING		1
#define RESIZING		2
#define RESIZING_WITH_FORCE	RESIZING+4
#define FORCE_MANAGED_ON(lw) (lw->layout.resizing == RESIZING_WITH_FORCE)

#define Constraints_On_Child(w, axis) \
	&((UKCLayoutConstraint)w->core.constraints)->layout.connections[axis]
#define CONSTRAINTS_MANAGED(w) \
	(((UKCLayoutConstraint)w->core.constraints)->layout.managed_done)

#define WIDGET_X(w) (w->core.x)
#define WIDGET_Y(w) (w->core.y)
#define WIDGET_WIDTH(w) (w->core.width)
#define WIDGET_HEIGHT(w) (w->core.height)
#define WIDGET_BORDER_WIDTH(w) (w->core.border_width)

#define POS_ONLY(num) (unsigned)((int)(num) < 0 ? 0 : (num))
#define XTOP(w)	(POS_ONLY(WIDGET_X(w)))
#define XBOTTOM(w) \
	(POS_ONLY(WIDGET_X(w) + WIDGET_BORDER_WIDTH(w)*2 + WIDGET_WIDTH(w)))
#define YTOP(w)	(POS_ONLY(WIDGET_Y(w)))
#define YBOTTOM(w) \
	(POS_ONLY(WIDGET_Y(w) + WIDGET_BORDER_WIDTH(w)*2 + WIDGET_HEIGHT(w)))
#define REF_DIMENSION(w, axis) \
	(((UKCLayoutConstraint)w->core.constraints) \
		->layout.connections[axis].dimension)
#define ABS_SIZE(l, a) ((Dimension)(l->dimension * (double)(a) + 0.5))
#define ABS_WIDGET_SIZE(l, a, lw) ((Dimension)(l->dimension * (double) \
		(a == VERTICAL ? lw->core.height : lw->core.width) + 0.5))


/* line drawing definitions - for debugging only so they're not resources */
#define LINE_WIDTH 0
#define IN_DISTANCE 5

#define DEFAULT_WIDTH 500
#define DEFAULT_HEIGHT 300

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Label.h>
#include <X11/Xukc/Convert.h>
#include "LayoutP.h"


/* these translations are mainly used for debugging */

#ifdef DEBUG
static char defaultTranslations[] =
	"<Btn1Down>:	draw_connections() \n\
	 <Btn1Up>:	clear_connections() \n\
	 <Btn2Down>:	print_widget_connections() \n\
	 <Btn3Down>:	print_connections()";
#endif DEBUG

/* two constraint resources settable by the user ... whether the widget is
 * fixed in the Horizontal and/or Vertical directions
 * NOTE: an unmanaged child is classed as being fixed for resize purposes.
 */

static XtResource constraint_resources[] = {
#define offset(field) XtOffset(UKCLayoutConstraint, layout.field)
	{ XtNhorizFixed, XtCHorizFixed, XtRBoolean, sizeof(Boolean),
		offset(connections[HORIZONTAL].fixed), XtRImmediate, FALSE},
	{ XtNvertFixed, XtCVertFixed, XtRBoolean, sizeof(Boolean),
		offset(connections[VERTICAL].fixed), XtRImmediate, FALSE}
#undef offset
};

/* resources defined in this class:
 *
 * 	XtNminWidth = minimum width widget is allowed to be.
 *	XtNminHeight = minimum height widget is allowed to be.
 *	   (if widget is resized smaller than these a Too Small message
 *          is displayed in the center of the widget)
 *      XtNhorizColor = colour that the horizontal connections are drawn in.
 *      XtNvertColor = colour that the vertical connections are drawn in.
 *	XtNgeomStatus = XukcAllowMove | XukcAllowResize | XukcTurnedOff |
 *			XukcRefuseAll | XukcOnlyAskParent
 */

static XtResource resources[] = {
#define offset(field) XtOffset(UKCLayoutWidget, layout.field)
	{ XtNminWidth, XtCMinWidth, XtRDimension, sizeof(Dimension),
	    offset(minimum_width), XtRImmediate, (XtPointer)0},
	{ XtNminHeight, XtCMinHeight, XtRDimension, sizeof(Dimension),
	    offset(minimum_height), XtRImmediate, (XtPointer)0},
	{ XtNhorizColor, XtCForeground, XtRPixel, sizeof(Pixel),
	    offset(draw_horiz_foreground), XtRString, XtExtdefaultforeground},
	{ XtNvertColor, XtCForeground, XtRPixel, sizeof(Pixel),
	    offset(draw_vert_foreground), XtRString, XtExtdefaultforeground},
	{ XtNgeomStatus, XtCGeomStatus, XtRGeomStatus, sizeof(int),
	    offset(geom_status), XtRImmediate, (XtPointer)XukcRefuseAll },
	{ XtNresizeDone, XtCCallback, XtRCallback, sizeof(XtPointer),
	    offset(resize_done), XtRCallback, (XtPointer)NULL },
#undef offset
};

static void DrawConnections();	/* draw lines between connected widgets */
static void ClearConnections(); /* clear any lines drawn */
static void PrintWidgetConnections(); /* print connection list for a child */
static void PrintConnections(); /* print connections list for a layout widget */

static XtActionsRec actionsList[] =
{
	{"draw_connections",		DrawConnections},
	{"clear_connections",		ClearConnections},
	{"print_widget_connections",	PrintWidgetConnections},
	{"print_connections",		PrintConnections},
};

static void ClassInitialize();
static void ClassPartInitialize();
static void Initialize();
static void Realize();
static void Resize();
static void Redraw();
static void Destroy();
static XtGeometryResult Layout_geometry();
static XtGeometryResult Geometry_handler();
static void Change_managed();
static void Insert_child();
static void Delete_child();
static void constraint_Initialize();
static void constraint_Destroy();
static Boolean constraint_SetValues();
static void lwTooSmall();

/* child event handlers */
static void lwEnteredWidget();

static void bubble_all();
static void sort_list_by_unfixed();

static CompositeClassExtensionRec extension_rec = {
	/* next_extension */  NULL,
	/* record_type */     NULLQUARK,
	/* version */         XtCompositeExtensionVersion,
	/* record_size */     sizeof(CompositeClassExtensionRec),
	/* accepts_objects */ TRUE,
};

UKCLayoutClassRec ukcLayoutClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	(WidgetClass) &constraintClassRec,
    /* class_name	  */	"UKCLayout",
    /* widget_size	  */	sizeof(UKCLayoutRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_initialize */ ClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	Realize,
    /* actions		  */	actionsList,
    /* num_actions	  */	XtNumber(actionsList),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	Resize,
    /* expose		  */	Redraw,
    /* set_values	  */	NULL,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_private   */	NULL,
#ifdef DEBUG
    /* tm_table		  */	defaultTranslations,
#else
    /* tm_table           */	NULL,
#endif DEBUG
    /* query_geometry	  */	Layout_geometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL,
  },
  {
/* composite_class fields */
    /* geometry_handler	  */	Geometry_handler,
    /* change_managed	  */	Change_managed,
    /* insert_child	  */	Insert_child,
    /* delete_child	  */	Delete_child,
    /* extension	  */	NULL,
  },
  {
/* constraint_class fields */
    /* resources	  */	constraint_resources,
    /* num_resources	  */	XtNumber(constraint_resources),
    /* constraint_size	  */	sizeof(UKCLayoutConstraintRecord),
    /* initialize	  */	constraint_Initialize,
    /* destroy		  */	constraint_Destroy,
    /* set_values	  */	constraint_SetValues,
    /* extension	  */	NULL,
  }
/* layout_class fields (NO NEW ONES) */
};

/* class record */

WidgetClass ukcLayoutWidgetClass = (WidgetClass)&ukcLayoutClassRec;

Cardinal sort_axis;	/* Global variable used by sort_list_by_unfixed() */

/* The following code can be reused across widgets for bit settings
 * of other flags */

/* resource converters */

static EnumListRec flagList[] = {
	{ XukcEAllowMove, XukcAllowMove },
	{ XukcEAllowResize, XukcAllowResize },
	{ XukcETurnedOff, XukcTurnedOff },
	{ XukcERefuseAll, XukcRefuseAll },
	{ XukcEOnlyAskParent, XukcOnlyAskParent },
};

static XtConvertArgRec convertArgs[] = {
	{ XtAddress,	(XtPointer)flagList, sizeof(EnumList) },
	{ XtImmediate,	(XtPointer)XtNumber(flagList), sizeof(Cardinal) },
	{ XtAddress,	XtRGeomStatus,	sizeof(String) },
};


/****************************************************************
 *
 * Standard Widget Procedures
 *
 ****************************************************************/

/* > ClassInitialize < */
static void
ClassInitialize()
{
	/* add the string to geometry status converter */
	XtSetTypeConverter(XtRString, XtRGeomStatus,
			XukcCvtStringToFlagSettings,
			convertArgs, XtNumber(convertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
	XtSetTypeConverter(XtRGeomStatus, XtRString,
			XukcCvtFlagSettingsToString,
			convertArgs, XtNumber(convertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
}


static void
ClassPartInitialize(wc)
WidgetClass wc;
{
	UKCLayoutWidgetClass lwc = (UKCLayoutWidgetClass)wc;

	extension_rec.next_extension = lwc->composite_class.extension;
	lwc->composite_class.extension = (XtPointer)&extension_rec;
}


/* > Initialize < */
/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCLayoutWidget lw = (UKCLayoutWidget) new;

#ifdef DEBUG
printf("Initialize(layout)\n");
#endif
	/* if no size has been set give it a reasonable default */
	if (lw->core.width == 0) {
		if (lw->layout.minimum_width > 0)
			lw->core.width = lw->layout.minimum_width;
		else if (XtParent(new)->core.width > 0)
			lw->core.width = XtParent(new)->core.width;
		else
			lw->core.width = DEFAULT_WIDTH;
	}

	if (lw->core.height == 0) {
		if (lw->layout.minimum_height > 0)
			lw->core.height = lw->layout.minimum_height;
		else if (XtParent(new)->core.height > 0)
			lw->core.height = XtParent(new)->core.height;
		else
			lw->core.height = DEFAULT_HEIGHT;
	}

	lw->layout.oldwidth = lw->core.width;
	lw->layout.oldheight = lw->core.height;

	/* if our parent is a shell see if its size has not been set */
	if (XtIsSubclass(XtParent(new), shellWidgetClass)) {
		Arg args[2];
		int i = 0;

		if (XtParent(new)->core.height == 0) {
			XtSetArg(args[i], XtNheight, lw->core.height); i++;
		}
		if (XtParent(new)->core.width == 0) {
			XtSetArg(args[i], XtNwidth, lw->core.width); i++;
		}

		if (i > 0)
			XtSetValues(XtParent(new), (ArgList)args, i);
	}

	/* zero the heads of the connection lists */
	lw->layout.connections[HORIZONTAL].next = NULL;
	lw->layout.connections[HORIZONTAL].prev = NULL;
	lw->layout.connections[HORIZONTAL].unfixed_dimension = 0.0;
	lw->layout.connections[HORIZONTAL].fixed = FALSE;

	lw->layout.connections[VERTICAL].next = NULL;
	lw->layout.connections[VERTICAL].prev = NULL;
	lw->layout.connections[VERTICAL].unfixed_dimension = 0.0;
	lw->layout.connections[VERTICAL].fixed = FALSE;

	/* the child we last received an EnterNotify for */
	lw->layout.current_widget = NULL;

	/* labelWidget used for the "Too Small" message */
	lw->layout.too_small = lwTooSmall;
	lw->layout.too_small_widget = NULL;
	lw->layout.too_small_used = FALSE;

	lw->layout.drawing_connections = FALSE;
	lw->layout.resizing = NOT_RESIZING;
	lw->layout.managed_once = FALSE;
}


/* > Destroy < */
static void
Destroy(w)
Widget w;
{
	UKCLayoutWidget lw = (UKCLayoutWidget)w;
	Connections temp;
	int axis;

	/* free memory allocated to root node connection lists */
	for (axis = 0; axis <= 1; axis++) {
		while (lw->layout.connections[axis].next != NULL) {
			temp = lw->layout.connections[axis].next;
			lw->layout.connections[axis].next =
				lw->layout.connections[axis].next->next;
			XtFree((char *)temp);
		}

		/* previous list should always be NULL */
		if (lw->layout.connections[axis].prev != NULL) {
			XtError("Corruption of dying layout widget");
			abort(-1);
		}
	}
}


/* > Child Resize <
 * Recursively resize the siblings on the `axis' connection list of `w'.
 * children who are fixed in the given axis are not resized but may be
 * moved if siblings before them on the connection list were resized.
 * Children currently unmanaged are classed as being fixed
 */
static void
ChildResize(axis, w, new_dimension, old_dimension, changing, lw, offset)
Cardinal axis;
Widget w;
Dimension new_dimension, old_dimension;
double changing;
UKCLayoutWidget lw;
Position offset;
{
	Position bit;
	Position used;
	Boolean resize;
	Wconns conn_list, look_list;
	Connections look_conns;

	if (w == NULL || old_dimension == new_dimension)
		return;

	conn_list = Constraints_On_Child(w, axis);
	look_conns = conn_list->next;
	bit = 0;
	used = 0;
	resize = FALSE;
		
	if (changing == 0) {
		if (!conn_list->fixed &&
		    (CONSTRAINTS_MANAGED(w) || FORCE_MANAGED_ON(lw)))
			changing = conn_list->dimension;

		while(look_conns != NULL) {
			look_list = Constraints_On_Child(
						look_conns->sibling, axis);
			if (!look_list->fixed &&
			    (CONSTRAINTS_MANAGED(look_conns->sibling)
			    || FORCE_MANAGED_ON(lw)))
				changing += look_list->dimension;
			look_conns = look_list->next;
		};
	}

	if (!conn_list->fixed && changing > 0) {
		resize = TRUE;
		bit = (Position)(conn_list->dimension / changing
				  * ((double)new_dimension
				  - (double)old_dimension) +
				  (double)(axis == HORIZONTAL ?
						w->core.width : w->core.height)
				  + 0.5);
	}

	if (!conn_list->resized) {
		if (axis == HORIZONTAL) {
			if (offset)
				XtMoveWidget(w, w->core.x + offset, w->core.y);
			if (resize && !conn_list->fixed &&
			   (CONSTRAINTS_MANAGED(w) || FORCE_MANAGED_ON(lw))) {
				bit = (bit <= 0) ? 1 : bit;
				used = bit - w->core.width;
				XtResizeWidget(w, (Dimension)bit,
							w->core.height,
							w->core.border_width);
			}
		} else {
			if (offset)
				XtMoveWidget(w, w->core.x, w->core.y + offset);
			if (resize && !conn_list->fixed &&
			   (CONSTRAINTS_MANAGED(w) || FORCE_MANAGED_ON(lw))) {
				bit = (bit <= 0) ? 1 : bit;
				used = bit - w->core.height;
				XtResizeWidget(w, w->core.width,
							(Dimension)bit,
							w->core.border_width);
			}
		}

		conn_list->resized = TRUE;
	}

	if (conn_list->next != NULL) {
		Connections child_list;

		child_list = conn_list->next;
		for (; child_list != NULL; child_list = child_list->next)
			ChildResize(axis, child_list->sibling, new_dimension,
				old_dimension, changing, lw, used + offset);
	}
}


/*	> Resize <
 * resize any children on the connection lists which are not fixed and
 * move any required to keep the distances between widgets constant
 */
static void
Resize(w)
Widget w;
{
	UKCLayoutWidget lw = (UKCLayoutWidget) w;
	int Kids = lw->composite.num_children;
	Connections xchild;
	Connections ychild;

#ifdef DEBUG
	printf("Resize(layout)\n");
#endif
	if (Kids == 0) {
		lw->layout.oldwidth = lw->core.width;
		lw->layout.oldheight = lw->core.height;
		return;
	}

	if ((lw->core.width < lw->layout.minimum_width)
	   || (lw->core.height < lw->layout.minimum_height)) {
		(*lw->layout.too_small)(lw,
				lw->core.width < lw->layout.minimum_width,
				lw->core.height < lw->layout.minimum_height);
		return;	
	}

#ifndef DEBUG
	if (!XtIsSubclass(XtParent(w), ukcLayoutWidgetClass) && XtIsRealized(w))
		XtUnmapWidget(w);
#endif !DEBUG

	/* stop change_managed() from doing nasty things! */
	if (!XtIsRealized(w) && !lw->layout.managed_once) {
		lw->layout.resizing = RESIZING_WITH_FORCE;
		Change_managed(w);
		lw->layout.managed_once = FALSE;
	}

	xchild = lw->layout.connections[HORIZONTAL].next;
	ychild = lw->layout.connections[VERTICAL].next;

	/* reset all resized flags */
	while (Kids > 0) {
		Widget w = lw->composite.children[--Kids];
		UKCLayoutConstraint lc;

		lc = (UKCLayoutConstraint)w->core.constraints;
		lc->layout.connections[HORIZONTAL].resized = FALSE;
		lc->layout.connections[VERTICAL].resized = FALSE;
	};

	for (; xchild != NULL; xchild = xchild->next)
		ChildResize(HORIZONTAL, xchild->sibling,
				lw->core.width, lw->layout.oldwidth,
				(double)0, lw, 0);
	for (; ychild != NULL; ychild = ychild->next)
		ChildResize(VERTICAL, ychild->sibling,
				lw->core.height, lw->layout.oldheight,
				(double)0, lw, 0);

	if (!XtIsRealized(w) && !lw->layout.managed_once) {
		lw->layout.resizing = RESIZING;
		Change_managed(w);
	}

	lw->layout.resizing = NOT_RESIZING;

	if (lw->layout.too_small_used) {
		lw->layout.too_small_used = FALSE;
		XtUnmanageChild(lw->layout.too_small_widget);
		if (XtIsSubclass(XtParent(lw), shellWidgetClass))
			XtRemoveGrab(lw->layout.too_small_widget);
	}

#ifndef DEBUG
	if (XtIsRealized(w))
		XtMapWidget(w);
#endif DEBUG
	/* inform any interested parties that all resizing has been done */
	if (XtHasCallbacks(w, XtNresizeDone) == XtCallbackHasSome) {
		XFlush(XtDisplay(w));
		XtCallCallbacks(w, XtNresizeDone, (XtPointer)NULL);
	}
	lw->layout.oldwidth = lw->core.width;
	lw->layout.oldheight = lw->core.height;
}


static void
Realize(w, valueMask, attributes)
Widget w;
XtValueMask *valueMask;
XSetWindowAttributes *attributes;
{
#ifdef DEBUG
	printf("Realize(layout)\n");
#endif

	/* force all the backing store hints to be off .. we want exposures! */
	/* but we still may never get them 8-( */
	*valueMask |= CWBackingStore | CWSaveUnder;
	attributes->backing_store = NotUseful;
	attributes->save_under = False;
	XtCreateWindow(w, InputOutput, (Visual *)CopyFromParent,
			(XtValueMask)*valueMask, attributes);
}


/* > Redraw - on exposure events < */
static void
Redraw(w, event, region)
Widget w;
XEvent *event;
Region region;
{
	UKCLayoutWidget lw = (UKCLayoutWidget) w;
	Cardinal i;

	if (lw->layout.drawing_connections)
		DrawConnections(w, (XEvent *)NULL, (String *)NULL,
							(Cardinal *)NULL);

	for (i = 0; i < lw->composite.num_children; i++) {
		if (XtIsWidget(lw->composite.children[i]))
			continue;

		/* redraw all rectangle objects that have been exposed */
		if (XtIsManaged(lw->composite.children[i]) &&
		    XtIsRectObj(lw->composite.children[i])) {
			RectObj obj = (RectObj)lw->composite.children[i];
			RectObjClass class;

			if (region != NULL)
				switch(XRectInRegion(region,
					(int)obj->rectangle.x,
					(int)obj->rectangle.y,
					(unsigned int)obj->rectangle.width,
					(unsigned int)obj->rectangle.height))
				{
					case RectangleIn:
					case RectanglePart:
						break;
					default:
						continue;
				}
			class = (RectObjClass)XtClass((Widget)obj);
			if (class->rect_class.expose != NULL)
				(*class->rect_class.expose)((Widget)obj,
							event, (Region)NULL);
		}
	}
}


/* > Layout_geometry <
 * return the preferred geometry for the layout widget
 */
static XtGeometryResult
Layout_geometry(w, request, preferred_return)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *preferred_return;
{
	UKCLayoutWidget lw = (UKCLayoutWidget)w;

#if DEBUG
printf("UKCLayout preferred geometry = %d x %d\n", lw->layout.minimum_width,
	lw->layout.minimum_height);
#endif DEBUG
	preferred_return->request_mode = CWWidth | CWHeight;
	preferred_return->height = lw->layout.minimum_height
			? lw->layout.minimum_height : lw->core.height;
	preferred_return->width = lw->layout.minimum_width
			? lw->layout.minimum_width : lw->core.width;

	if (((request->request_mode & CWWidth) &&
	   (request->width != preferred_return->width)) ||
	   ((request->request_mode & CWHeight) &&
	   (request->height != preferred_return->height)) ||
	   (preferred_return->request_mode != request->request_mode))
		return (XtGeometryAlmost);

	if ((lw->core.height == preferred_return->height) &&
	    (lw->core.width == preferred_return->width))
		return (XtGeometryNo);

	return (XtGeometryYes);
}


/**** Geometry Handler Starts Here *****/


/* > unfixed_dimensions <
 * recurse up the connection list to find the size of unfixed children
 * used by Resize() for the row/column that this_child is on
 */
static double
unfixed_dimensions(w, layout, axis)
Widget w;
UKCLayoutWidget layout;
Cardinal axis;
{
	Wconns conn_list = Constraints_On_Child(w, axis);

	if (conn_list->prev == NULL ||
	    conn_list->prev->sibling == (Widget)layout)
		return (conn_list->unfixed_dimension);
	else
		return (unfixed_dimensions(conn_list->prev->sibling,
			layout, axis));
}


/* > yspace_for_previous <
 * find the smallest vertical space between the bottom of the siblings on the
 * prev_list and the home_pos
 */
static Dimension
yspace_for_previous(prev_list, total, home_pos)
Connections prev_list;
Dimension total, home_pos;
{
	Widget w;
	int run_total;

	if ((prev_list == NULL) ||
		XtIsSubclass(prev_list->sibling, ukcLayoutWidgetClass))
			return (total);

	w = prev_list->sibling;
	run_total = home_pos - YBOTTOM(w);
	if ((run_total < 0) || (run_total < total))
		total = POS_ONLY(run_total);
	run_total = yspace_for_previous(prev_list->next, total, home_pos);
	return ((Dimension)run_total);
}


/* > yspace_for_next <
 * find the smallest vertical space between the top of the siblings on the
 * next_list and the home_pos
 */
static Dimension
yspace_for_next(next_list, total, home_pos, layout_height)
Connections next_list;
Dimension total, home_pos, layout_height;
{
	int run_total;
	Widget w;

	if (next_list == NULL) {
		run_total = layout_height - home_pos;
		if ((run_total < 0) || (run_total < total))
			total = POS_ONLY(run_total);
		return (total);
	}

	while (next_list != NULL) {
		w = next_list->sibling;
		run_total = YTOP(w) - home_pos;
		if ((run_total < 0) || (run_total < total))
			total = POS_ONLY(run_total);
		next_list = next_list->next;
	}
	return (total);
}


/* > all_next_yspace <
 * find the sum of the smallest vertical distances between the siblings and
 * the next_list, their connected siblings and the enclosing layout widget
 */
static Dimension
all_next_yspace(next_list, default_space, layout_height)
Connections next_list;
Dimension default_space, layout_height;
{
	Wconns conn_list;
	Dimension in_total = 0;
	Dimension in2_total = 0;
	int total;
	Widget w;

	while (next_list != NULL) {
		if (in_total == 0)
			in_total = default_space;
		if (in2_total == 0)
			in2_total = default_space;

		w = next_list->sibling;
		conn_list = Constraints_On_Child(w, VERTICAL);
		total = yspace_for_next(conn_list->next, default_space,
			   YBOTTOM(w), layout_height);
		if ((total < 0) || (total < in_total))
			in_total = POS_ONLY(total);

		total = all_next_yspace(conn_list->next,
				POS_ONLY(layout_height - YBOTTOM(w)),
				layout_height);
		if ((total < 0) || (total < in2_total))
			in2_total = POS_ONLY(total);

		next_list = next_list->next;
	}
	return (in2_total + in_total);
}


/* > all_prev_yspace <
 * find the sum of the smallest vertical distances between the siblings on
 * the prev_list, their connected siblings and the enclosing layout widget
 */
static Dimension
all_prev_yspace(prev_list, default_space)
Connections prev_list;
Dimension default_space;
{
	Wconns conn_list;
	Dimension in_total = 0;
	Dimension in2_total = 0;
	int total;
	Widget w;

	while ((prev_list != NULL) &&
	   !XtIsSubclass(prev_list->sibling, ukcLayoutWidgetClass)) {
		if (!in_total)
			in_total = default_space;
		if (!in2_total)
			in2_total = default_space;

		w = prev_list->sibling;
		conn_list = Constraints_On_Child(w, VERTICAL);

		total = yspace_for_previous(conn_list->prev, YTOP(w), YTOP(w));

		if ((total < 0) || (total < in_total))
			in_total = POS_ONLY(total);

		total = all_prev_yspace(conn_list->prev, YTOP(w));

		if ((total < 0) || (total < in2_total))
			in2_total = POS_ONLY(total);

		prev_list = prev_list->next;
	}
	return (in2_total + in_total);
}


/* > move_prev_in_y <
 * move this_child up by `distance' and recursively move up any connected
 * siblings who may be occluded by this move
 */
static void
move_prev_in_y(layout, w, distance)
UKCLayoutWidget layout;
Widget w;
int distance;
{
	int space;
	Connections prev_list;
	
	if (w == (Widget)layout)
		return;

	prev_list = (Constraints_On_Child(w, VERTICAL))->prev;

	space = yspace_for_previous(prev_list, YTOP(w), YTOP(w));

	/* we are going to occluded something by this move ... */
	if (space < distance) {
		/* move all connected siblings up if they in the way */
		for (; prev_list != NULL; prev_list = prev_list->next)
			move_prev_in_y(layout, prev_list->sibling,
							distance - space);
	}

	XtMoveWidget(w, w->core.x, (Position)(w->core.y - distance));
}


/* > move_next_in_y <
 * move this_child down by `distance' and recursively move down any connected
 * siblings who may be occluded by this move
 */
static void
move_next_in_y(layout, w, distance)
UKCLayoutWidget layout;
Widget w;
int distance;
{
	int space;
	Connections next_list = (Constraints_On_Child(w, VERTICAL))->next;

	space = yspace_for_next(next_list,
			POS_ONLY(layout->core.height - YBOTTOM(w)),
			YBOTTOM(w), layout->core.height);

	/* we are going to occlude something by this move ... */
	if (space < distance) {
		/* move all connected siblings down out of the way */
		for (; next_list != NULL; next_list = next_list->next)
			move_next_in_y(layout, next_list->sibling,
				distance - space);
	}

	XtMoveWidget(w, w->core.x, (Position)(w->core.y + distance));
}


/* > resize_in_y <
 * calculate if we can increase this_child by the required_space. Reposition
 * this_child if there is enough room around this_child. Other widgets may be
 * moved if the space between them is enough to allow the resize. UKCLayout's
 * parent may be asked to grow bigger if there is not enough room.
 *
 * compromise_space is modified to contain the space available for the resize.
 *
 * new_pos is modified to contain y offset to the current position, if moved.
 *
 * layout_resized is modified to contain CWHeight if the layout was resized.
 *
 * do_it_again is modified to contain CWHeight if the resize can be made by the
 * movement of connected siblings.
 *
 * Returns: XtGeometryYes if resize can be made (or can be made and siblings
 *	    have been moved, if do_it == TRUE).
 *	    XtGeometryAlmost if only a compromise_space can be given.
 *	    XtGeometryNo/XtGeometryDone are never returned.
 */
static XtGeometryResult
resize_in_y(layout, w, required_space, new_pos,
			compromise_space, layout_resized, do_it, do_it_again)
UKCLayoutWidget layout;	/* parent of widget to resize */
Widget w;		/* widget to resize */
int required_space;	/* extra space needed for widgets height */
Position *new_pos;	/* offset to move widget to on resize */
int *compromise_space;	/* available space if XtGeometryAlmost is returned */
XtGeometryMask *layout_resized;/* sets CWHeight if layout widget was resized */
Boolean do_it;		/* other widgets are only moved if TRUE */
XtGeometryMask *do_it_again; /* set to CWHeight if need to move other widgets */
{
	Dimension prev_space, next_space;
	int up_distance, down_distance;
	XtWidgetGeometry request, reply;
	XtGeometryResult result;
	Wconns conn_list = Constraints_On_Child(w, VERTICAL);

	*compromise_space = required_space;
	*new_pos = 0;
	result = XtGeometryNo;

	/* cannot resize a fixed child */
	if (conn_list->fixed)
		return (XtGeometryNo);

	prev_space = next_space = 0;

	if (!(layout->layout.geom_status & XukcOnlyAskParent)) {
		if (required_space <= 0)
			return (XtGeometryYes);

		/* space immeadiately above widget */
		prev_space = yspace_for_previous(conn_list->prev, YTOP(w),
								  YTOP(w));
		/* space immeadiately below widget */
		next_space = yspace_for_next(conn_list->next,
				POS_ONLY(layout->core.height - YBOTTOM(w)),
				YBOTTOM(w), layout->core.height);

		if (required_space < (prev_space + next_space)) {
		/* there is enough room around widget for a resize so calculate
		 * the offset for the y position.
		 */
			*new_pos = - (prev_space -
				(prev_space + next_space - required_space)/2);
			return (XtGeometryYes);
		}
	}

	/* we may need to move other widgets so check how much space is
	 * free around this child's connected siblings */

	if (!(layout->layout.geom_status & XukcOnlyAskParent)) {
		/* total space above widget */
		prev_space += all_prev_yspace(conn_list->prev, YTOP(w));

		/* total space below widget */
		next_space += all_next_yspace(conn_list->next,
			POS_ONLY(layout->core.height - YBOTTOM(w)),
				layout->core.height);
	}

	*compromise_space = prev_space + next_space;
	*new_pos = 0;
	result = XtGeometryAlmost;

	if ((layout->layout.geom_status & XukcOnlyAskParent)
	    || required_space > *compromise_space) {
		/* need to ask the layout's parent for a change in size,
		 * either to shrink or grow. */
		Dimension orig_size = (Dimension)(unfixed_dimensions(w,
						  layout, VERTICAL)
						  * layout->core.height + 0.5);
#ifdef SHELL_KLUDGE
		Dimension old_height = layout->core.height;
#endif /* SHELL_KLUDGE */

		request.request_mode = do_it ? CWHeight :
					       CWHeight | XtCWQueryOnly;
		request.height = (Dimension)((double)
					(orig_size + required_space) /
					REF_DIMENSION(w, VERTICAL) + 0.5);

		/* deny the new size if it is smaller than the layout's
		 * minimum size. */
		if (layout->layout.minimum_height > 0 &&
		    layout->layout.minimum_height > request.height) {
			*compromise_space = (Dimension)(
				(double)layout->layout.minimum_height
				* REF_DIMENSION(w, VERTICAL) + 0.5)
				- orig_size;
			return (XtGeometryAlmost);
		}

		result = XtMakeGeometryRequest((Widget)layout, &request, &reply);
		switch (result) {
			case XtGeometryNo :
				/* layout cannot be resized so have to
				 * make do.
				 */
				result = XtGeometryAlmost;
				break;
			case XtGeometryAlmost :
				/* layout can get a bit bigger */
				*compromise_space = (Dimension)(
					(double)reply.height
					* REF_DIMENSION(w, VERTICAL) + 0.5)
					- orig_size;
				break;
			case XtGeometryYes :
				/* layout can be resized */
				*compromise_space = required_space;
				if (do_it)
					*layout_resized |= CWHeight;
				else
#ifdef SHELL_KLUDGE
{
	*do_it_again |= CWHeight;
	request.request_mode = CWHeight;
	request.height = old_height;
	(void)XtMakeGeometryRequest((Widget)layout, &request, &reply);
}
#else
					*do_it_again |= CWHeight;
#endif /* SHELL_KLUDGE */
				break;
		}
	} else {
		if (layout->layout.geom_status & XukcAllowMove) {
			/* other widgets must be moved */
			if (prev_space < next_space) {
				if (prev_space < (required_space/2))
					up_distance = prev_space;
				else
					up_distance = required_space/2;

				down_distance = required_space
						   - up_distance;
			} else {
				if (next_space < (required_space/2))
					down_distance = next_space;
				else
					down_distance = required_space/2;
				up_distance = required_space
						- down_distance;
			}

			if (do_it) {
				*new_pos = w->core.y;
				move_prev_in_y(layout, w, up_distance);
				w->core.y = *new_pos;
				move_next_in_y(layout, w, down_distance);
				XtMoveWidget(w, w->core.x, *new_pos);
			} else
				*do_it_again |= CWHeight;

			*new_pos = -up_distance;
			*compromise_space = required_space;
			result = XtGeometryYes;
		}
	}
	return (result);
}


/* > xspace_for_previous <
 * find the smallest horizontal space between the right sides of the siblings
 * on the prev_list and the home_pos
 */
static Dimension
xspace_for_previous(prev_list, total, home_pos)
Connections prev_list;
Dimension total, home_pos;
{
	Widget w;
	int run_total;

	if ((prev_list == NULL) ||
	   XtIsSubclass(prev_list->sibling, ukcLayoutWidgetClass))
		return (total);

	w = prev_list->sibling;
	run_total = home_pos - XBOTTOM(w);
	if ((run_total < 0) || (run_total < total))
		total = POS_ONLY(run_total);
	run_total = xspace_for_previous(prev_list->next, total, home_pos);
	return ((Dimension)run_total);
}


/* > xspace_for_next <
 * find the smallest horizontal space between the left sides of the siblings
 * on the next_list and the home_pos
 */
static Dimension
xspace_for_next(next_list, total, home_pos, layout_width)
Connections next_list;
Dimension total, home_pos, layout_width;
{
	int run_total;
	Widget w;

	if (next_list == NULL) {
		run_total = (layout_width - home_pos);
		if ((run_total < 0) || (run_total < total))
			total = POS_ONLY(run_total);
		return (total);
	}

	while (next_list != NULL) {
		w = next_list->sibling;
		run_total = XTOP(w) - home_pos;
		if ((run_total < 0) || (run_total < total))
			total = POS_ONLY(run_total);
		next_list = next_list->next;
	}
	return (total);
}


/* > all_next_xspace <
 * find the sum of the smallest horizontal distances between the siblings and
 * the next_list, their connected siblings and the enclosing layout widget
 */
static Dimension
all_next_xspace(next_list, default_space, layout_width)
Connections next_list;
Dimension default_space, layout_width;
{
	Dimension in_total = 0;
	Dimension in2_total = 0;
	int total;
	Widget w;
	Wconns conn_list;

	while (next_list != NULL) {
		if (in_total == 0)
			in_total = default_space;
		if (in2_total == 0)
			in2_total = default_space;

		w = next_list->sibling;
		conn_list = Constraints_On_Child(w, HORIZONTAL);

		total = xspace_for_next(conn_list->next, default_space,
						XBOTTOM(w), layout_width);

		if ((total < 0) || (total < in_total))
			in_total = POS_ONLY(total);

		total = all_next_xspace(conn_list->next,
				POS_ONLY(layout_width - XBOTTOM(w)),
				layout_width);

		if ((total < 0) || (total < in2_total))
			in2_total = POS_ONLY(total);

		next_list = next_list->next;
	}
	return (in2_total + in_total);
}


/* > all_prev_xspace <
 * find the sum of the smallest horizontal distances between the siblings on
 * the prev_list, their connected siblings and the enclosing layout widget
 */
static Dimension
all_prev_xspace(prev_list, default_space)
Connections prev_list;
Dimension default_space;
{
	Dimension in_total = 0;
	Dimension in2_total = 0;
	int total;
	Widget w;
	Wconns conn_list;

	while ((prev_list != NULL) &&
	   !XtIsSubclass(prev_list->sibling, ukcLayoutWidgetClass)) {
		if (in_total == 0)
			in_total = default_space;
		if (in2_total == 0)
			in2_total = default_space;

		w = prev_list->sibling;
		conn_list = Constraints_On_Child(w, HORIZONTAL);

		total = xspace_for_previous(conn_list->prev, XTOP(w), XTOP(w));

		if ((total < 0) || (total < in_total))
			in_total = POS_ONLY(total);

		total = all_prev_xspace(conn_list->prev, XTOP(w));

		if ((total < 0) || (total < in2_total))
			in2_total = POS_ONLY(total);

		prev_list = prev_list->next;
	}
	return (in2_total + in_total);
}


/* > move_prev_in_x <
 * move this_child left by `distance' and recursively move left any connected
 * siblings who may be occluded by this move
 */
static void
move_prev_in_x(layout, w, distance)
UKCLayoutWidget layout;
Widget w;
int distance;
{
	int space;
	Connections prev_list;
	
	if (w == (Widget)layout)
		return;

	prev_list = (Constraints_On_Child(w, HORIZONTAL))->prev;

	space = xspace_for_previous(prev_list, XTOP(w), XTOP(w));

	if (space < distance) {
		for (; prev_list != NULL; prev_list = prev_list->next)
			move_prev_in_x(layout, prev_list->sibling,
							distance - space);
	}

	XtMoveWidget(w, (Position)(w->core.x - distance), w->core.y);
}


/* > move_next_in_x <
 * move this_child right by `distance' and recursively move right any connected
 * siblings who may be occluded by this move
 */
static void
move_next_in_x(layout, w, distance)
UKCLayoutWidget layout;
Widget w;
int distance;
{
	int space;
	Connections next_list = (Constraints_On_Child(w, HORIZONTAL))->next;

	space = xspace_for_next(next_list,
			POS_ONLY(layout->core.width - XBOTTOM(w)),
			XBOTTOM(w), layout->core.width);

	if (space < distance) {
		for (; next_list != NULL; next_list = next_list->next)
			move_next_in_x(layout, next_list->sibling,
				distance - space);
	}

	XtMoveWidget(w, (Position)(w->core.x + distance), w->core.y);
}


/* > resize_in_x <
 * calculate if we can increase this_child by the required_space. Reposition
 * this_child if there is enough room around this_child. Other widgets may be
 * moved if the space between them is enough to allow the resize. UKCLayout's
 * parent may be asked to grow bigger if there is not enough room.
 *
 * compromise_space is modified to contain the space available for the resize.
 *
 * new_pos is modified to contain x offset to the current position, if moved.
 *
 * layout_resized is modified to contain CWWidth if the layout was resized.
 * do_it_again is modified to contain CWWidth if the resize can be made by the
 * movement of connected siblings.
 *
 * Returns: XtGeometryYes if resize can be made (or can be made and siblings
 *          have been moved, if do_it == TRUE).
 *          XtGeometryAlmost if only a compromise_space can be given.
 *          XtGeometryNo/XtGeometryDone are never returned.
 */
static XtGeometryResult
resize_in_x(layout, w, required_space, new_pos,
			compromise_space, layout_resized, do_it, do_it_again)
UKCLayoutWidget layout;		/* parent of widget to resize */
Widget w;			/* widget to resize */
int required_space;		/* extra space needed for widgets height */
Position *new_pos;		/* offset to move widget to on resize */
int *compromise_space;		/* space if XtGeometryAlmost is returned */
XtGeometryMask *layout_resized;	/* sets CWWidth if layout widget was resized */
Boolean do_it;			/* other widgets are only moved if TRUE */
XtGeometryMask *do_it_again; /* set to CWWidth if need to move other widgets */
{
	Dimension prev_space, next_space;
	int left_distance, right_distance;
	XtWidgetGeometry request, reply;
	XtGeometryResult result;
	Wconns conn_list = Constraints_On_Child(w, HORIZONTAL);

	*compromise_space = required_space;
	*new_pos = 0;
	result = XtGeometryNo;

	/* cannot resize a fixed child */
	if (conn_list->fixed)
		return (XtGeometryNo);

	prev_space = next_space = 0;

	if (!(layout->layout.geom_status & XukcOnlyAskParent)) {
		if (required_space <= 0)
			return (XtGeometryYes);

		/* space immeadiately to the left of the widget */
		prev_space = xspace_for_previous(conn_list->prev, XTOP(w),
								    XTOP(w));
		/* space immeadiately to the right of the widget */
		next_space = xspace_for_next(conn_list->next,
				POS_ONLY(layout->core.width - XBOTTOM(w)),
				XBOTTOM(w), layout->core.width);
		if (required_space < (prev_space + next_space)) {
		/* there is enough room around widget for a resize so calculate
		 * the offset for the x position
		 */
			*new_pos = - (prev_space -
				(prev_space + next_space - required_space)/2);
			return (XtGeometryYes);
		}
	}

	/* we may need to move other widgets so check how much space is
	 * free around this child's connected siblings */
	if (!(layout->layout.geom_status & XukcOnlyAskParent)) {
		/* total space to left of widget */
		prev_space += all_prev_xspace(conn_list->prev, XTOP(w));
		/* total space to right of widget */
		next_space += all_next_xspace(conn_list->next,
			POS_ONLY(layout->core.width - XBOTTOM(w)),
				layout->core.width);
	}

	*compromise_space = prev_space + next_space;
	*new_pos = 0;
	result = XtGeometryAlmost;

	if ((layout->layout.geom_status & XukcOnlyAskParent)
	    || required_space > *compromise_space) {
		/* need to ask the layout's parent for a change in size,
		 * either to shrink or grow. */ 
		Dimension orig_size = (Dimension)(unfixed_dimensions(w,
						  layout, VERTICAL)
						  * layout->core.width + 0.5);
#ifdef SHELL_KLUDGE
		Dimension old_width = layout->core.width;
#endif /* SHELL_KLUDGE */

		request.request_mode = do_it ? CWWidth :
					       CWWidth | XtCWQueryOnly;
		request.width = (Dimension)((double)
					(orig_size + required_space) /
					REF_DIMENSION(w, HORIZONTAL) + 0.5);

		/* deny the new size if it is smaller than the layout's
		 * minimum size. */
		if (layout->layout.minimum_width > 0 &&
		    layout->layout.minimum_width > request.width) {
			*compromise_space = (Dimension)(
				(double)layout->layout.minimum_width
				* REF_DIMENSION(w, HORIZONTAL) + 0.5)
				- orig_size;
			return (XtGeometryAlmost);
		}

		result = XtMakeGeometryRequest((Widget)layout, &request, &reply);
		switch(result) {
			case XtGeometryNo :
				/* layout cannot be resized so make do */
				result = XtGeometryAlmost;
				break;
			case XtGeometryAlmost :
				/* a bit bigger */
				*compromise_space = (Dimension)(
					(double)reply.width
					* REF_DIMENSION(w, HORIZONTAL) + 0.5)
					- orig_size;
				break;
			case XtGeometryYes :
				/* layout can be resized */
				*compromise_space = required_space;
				if (do_it)
					*layout_resized |= CWWidth;
				else
#ifdef SHELL_KLUDGE
{
	*do_it_again |= CWWidth;
	request.request_mode = CWWidth;
	request.width = old_width;
	(void)XtMakeGeometryRequest((Widget)layout, &request, &reply);
}
#else
					*do_it_again |= CWWidth;
#endif /* SHELL_KLUDGE */
				break;
		}
	} else {
		if (layout->layout.geom_status & XukcAllowMove) {
			/* other widgets must be moved */
			if (prev_space < next_space) {
				left_distance = (prev_space < (required_space/2))
					? prev_space : required_space/2;

				right_distance = required_space
						   - left_distance;
			} else {
				right_distance = (next_space < (required_space/2))
					? next_space : required_space/2;
				left_distance = required_space
						   - right_distance;
			}

			if (do_it) {
				*new_pos = w->core.x;
				move_prev_in_x(layout, w, left_distance);
				w->core.x = *new_pos;
				move_next_in_x(layout, w, right_distance);
				XtMoveWidget(w, *new_pos, w->core.y);
			} else
				*do_it_again |= CWWidth;

			*new_pos = -left_distance;
			*compromise_space = required_space;
			result = XtGeometryYes;
		}
	}
	return (result);
}


static XtGeometryResult
move_in_y(layout, w, request, compromise, do_it, do_it_again)
UKCLayoutWidget layout;
Widget w;
XtWidgetGeometry *request, *compromise;
Boolean do_it;
XtGeometryMask *do_it_again;
{
	/* movement in Y */
	Dimension border_width = w->core.border_width * 2;
	Dimension layout_height = layout->core.height;
	XtGeometryResult result = XtGeometryNo;
	Dimension prev_space, next_space;
	Wconns conn_list = Constraints_On_Child(w, VERTICAL);
	Boolean need_move = FALSE;

	if (request->y < compromise->y) {
		/* requested position is before the current/compromise position
		 * so check to see if there is enough space to do the move
		 */
		prev_space = yspace_for_previous(conn_list->prev,
						POS_ONLY(compromise->y),
						POS_ONLY(compromise->y));

		if ((request->y < (compromise->y - prev_space))
		   && (layout->layout.geom_status & XukcAllowMove)) {
			prev_space += all_prev_yspace(conn_list->prev,
						   POS_ONLY(compromise->y));
			need_move = TRUE;
		}

		if (request->y >= (compromise->y - prev_space)) {
			compromise->y = request->y;
			result = XtGeometryYes;
		} else {
			compromise->y -= prev_space;
			result = XtGeometryAlmost;
		}

		if (do_it)
			move_prev_in_y(layout, w, w->core.y - compromise->y);
	} else {
		next_space = yspace_for_next(conn_list->next,
		    POS_ONLY(layout_height - compromise->y - compromise->height
			- border_width),
		    POS_ONLY(compromise->y + compromise->height + border_width),
		    layout_height);

		if ((request->y > (compromise->y + next_space))
		   && (layout->layout.geom_status & XukcAllowMove)) {
			next_space += all_next_yspace(conn_list->next,
				POS_ONLY(layout_height - compromise->y
				- compromise->height - border_width),
				layout_height);
			need_move = TRUE;
		}

		if (request->y <= (compromise->y + next_space)) {
			compromise->y = request->y;
			result = XtGeometryYes;
		} else {
			compromise->y += next_space;
			result = XtGeometryAlmost;
		}

		if (do_it)
			move_next_in_y(layout, w, compromise->y - w->core.y);
	}

	if (need_move)
		*do_it_again |= CWY;

	return (result);
}


static XtGeometryResult
move_in_x(layout, w, request, compromise, do_it, do_it_again)
UKCLayoutWidget layout;
Widget w;
XtWidgetGeometry *request, *compromise;
Boolean do_it;
XtGeometryMask *do_it_again;
{
	/* movement in X */
	Dimension border_width = w->core.border_width * 2;
	Dimension layout_width = layout->core.width;
	XtGeometryResult result = XtGeometryNo;
	Dimension prev_space, next_space;
	Wconns conn_list = Constraints_On_Child(w, HORIZONTAL);
	Boolean need_move = FALSE;

	if (request->x < compromise->x) {
		prev_space = xspace_for_previous(conn_list->prev,
						POS_ONLY(compromise->x),
						POS_ONLY(compromise->x));

		if ((request->x < (compromise->x - prev_space))
		   && (layout->layout.geom_status & XukcAllowMove)) {
			prev_space += all_prev_xspace(conn_list->prev,
						   POS_ONLY(compromise->x));
			need_move = TRUE;
		}

		if (request->x >= (compromise->x - prev_space)) {
			compromise->x = request->x;
			result = XtGeometryYes;
		} else {
			compromise->x -= prev_space;
			result = XtGeometryAlmost;
		}

		if (do_it)
			move_prev_in_x(layout, w, w->core.x - compromise->x);
	} else {
		next_space = xspace_for_next(conn_list->next,
		    POS_ONLY(layout_width - compromise->x - compromise->width
		    - border_width),
		    POS_ONLY(compromise->x + compromise->width + border_width),
		    layout_width);

		if ((request->x > (compromise->x + next_space))
		   && (layout->layout.geom_status & XukcAllowMove)) {
			next_space += all_next_xspace(conn_list->next,
				POS_ONLY(layout_width - compromise->x
				- compromise->width - border_width),
				layout_width);
			need_move = TRUE;
		}

		if (request->x <= (compromise->x + next_space)) {
			compromise->x = request->x;
			result = XtGeometryYes;
		} else {
			compromise->x += next_space;
			result = XtGeometryAlmost;
		}

		if (do_it)
			move_next_in_x(layout, w, compromise->x - w->core.x);
	}

	if (need_move)
		*do_it_again |= CWX;

	return (result);
}


/*	> change_border_width <
 * Deal with a request to change the size of the widget's border. To do this
 * we must check the space around the widget in all 4 directions.
 */
static XtGeometryResult
change_border_width(layout, w, request, compromise)
UKCLayoutWidget layout;
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *compromise;
{
	Dimension smallest, space;
	Wconns conn_list;

	conn_list = Constraints_On_Child(w, VERTICAL);
	smallest = yspace_for_previous(conn_list->prev, 
			POS_ONLY(compromise->y), POS_ONLY(compromise->y));

	space = yspace_for_next(conn_list->next,
			POS_ONLY(layout->core.height - POS_ONLY(compromise->y)),
			POS_ONLY(compromise->y + compromise->height + 
				 compromise->border_width*2),
			layout->core.height);

	if (space < smallest)
		smallest = space;

	conn_list = Constraints_On_Child(w, HORIZONTAL);
	space = xspace_for_previous(conn_list->prev, POS_ONLY(compromise->x),
						POS_ONLY(compromise->x));

	if (space < smallest)
		smallest = space;

	space = xspace_for_next(conn_list->next,
			POS_ONLY(layout->core.width - POS_ONLY(compromise->x)),
			POS_ONLY(compromise->x + compromise->width +
				 compromise->border_width*2),
			layout->core.width);

	if (space < smallest)
		smallest = space;

	if (smallest == 0)
		return (XtGeometryNo);

	if ((smallest + w->core.border_width) <  request->border_width) {
		compromise->border_width = (smallest + w->core.border_width);
		return (XtGeometryAlmost);
	}

	compromise->border_width = request->border_width;
	return (XtGeometryYes);
}


/* ARGSUSED */
static XtGeometryResult
Geometry_handler(w, request, reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
	UKCLayoutWidget layout = (UKCLayoutWidget)XtParent(w);
	XtWidgetGeometry compromise;
	XtGeometryMask able_to_change = NULL;
	XtGeometryMask nearly_able = NULL;
	XtGeometryMask what_to_change = request->request_mode;
	XtGeometryMask do_it_again = NULL;
	XtGeometryMask layout_resized = NULL;
	XtGeometryResult returns_ok;
	Position orig_x, orig_y;

#if DEBUG
printf("Geometry_handler(%s, %s)\n",layout->core.name, w->core.name);
#endif
	/* if handler is turned off any requests are always allowed */
	if (layout->layout.geom_status & XukcTurnedOff) {
		if (request->request_mode & XtCWQueryOnly)
			return (XtGeometryYes);

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
		return (XtGeometryYes);
	}

	/* if XukcRefuseAll is set then no request is allowed */
	if (layout->layout.geom_status & XukcRefuseAll)
		return (XtGeometryNo);

	/* contains all results from requests */
	compromise.x = w->core.x;
	compromise.y = w->core.y;
	compromise.width = w->core.width;
	compromise.height = w->core.height;
	compromise.border_width = w->core.border_width;
	compromise.sibling = request->sibling;
	compromise.stack_mode = request->stack_mode;
	compromise.request_mode = 0;

	/* change stacking mode ... no restrictions on this */
	if (what_to_change & CWStackMode)
		able_to_change |= CWStackMode | (what_to_change & CWSibling);

	/* check if ok for resize in X direction */
	if ((what_to_change & CWWidth) &&
	    (layout->layout.geom_status & XukcAllowResize)) {
		int old_width = compromise.width;
		returns_ok = resize_in_x(layout, w,
				(int)(request->width - w->core.width),
				&compromise.x, &old_width,
				&layout_resized, FALSE, &do_it_again);

		if (returns_ok != XtGeometryNo) {
			if (returns_ok == XtGeometryAlmost)
				nearly_able |= CWWidth;
			else
				able_to_change |= CWWidth;
			compromise.x += w->core.x;
			compromise.width = old_width + w->core.width;
		}
	}

	/* check to see if widget can move in X direction */
	if (what_to_change & CWX) {
		/* movement in X */
		orig_x = compromise.x;
		returns_ok = move_in_x(layout, w, request, &compromise,
							FALSE, &do_it_again);

		if (returns_ok != XtGeometryNo) {
			if (returns_ok == XtGeometryAlmost)
				nearly_able |= CWX;
			else
				able_to_change |= CWX;
		}
	}

	/* check if ok for resize in Y direction */
	if ((what_to_change & CWHeight) &&
	    (layout->layout.geom_status & XukcAllowResize)) {
		int old_height = compromise.height;
		returns_ok = resize_in_y(layout, w,
				(int)(request->height - w->core.height),
				&compromise.y, &old_height,
				&layout_resized, FALSE, &do_it_again);

		if (returns_ok != XtGeometryNo) {
			if (returns_ok == XtGeometryAlmost)
				nearly_able |= CWHeight;
			else
				able_to_change |= CWHeight;
			compromise.y += w->core.y;
			compromise.height = old_height + w->core.height;
		}
	}

	if (what_to_change & CWY) {
		/* movement in Y */
		orig_y = compromise.y;
		returns_ok = move_in_y(layout, w, request, &compromise,
						FALSE, &do_it_again);

		if (returns_ok != XtGeometryNo) {
			if (returns_ok == XtGeometryAlmost)
				nearly_able |= CWY;
			else
				able_to_change |= CWY;
		}
	}

	if ((what_to_change & CWBorderWidth) &&
	    (layout->layout.geom_status & XukcAllowResize)) {
		returns_ok = change_border_width(layout, w, request, &compromise);
		if (returns_ok != XtGeometryNo) {
			if (returns_ok == XtGeometryAlmost)
				nearly_able |= CWBorderWidth;
			else
				able_to_change |= CWBorderWidth;
		}
	}

	/* could we satisfy any request ? */
	if ((able_to_change | nearly_able) == 0)
		return (XtGeometryNo); /* NO */

	if (what_to_change & XtCWQueryOnly)
		able_to_change |= XtCWQueryOnly;

	if (able_to_change != what_to_change) {
		/* nearly ... so return the compromises */
		compromise.request_mode = able_to_change | nearly_able;
		*reply = compromise;
		return (XtGeometryAlmost);
	}

	/* don't do any changes if this is just a query */
	if (what_to_change & XtCWQueryOnly)
		return (XtGeometryYes);

	/* if we are not debugging we don't want to see individual children
	 * being resized or moved.
	 */
#if 0
	/* ||| with this in mulitple set value requests cause loads of
	 *     exposure events to be queued duing the un/map requests */
	XtUnmapWidget((Widget)layout);
#endif !DEBUG

	/* we can resize in X so this time REALLY do it */
	if (do_it_again & CWWidth) {
		Position junk1;
		int junk2;
		XtGeometryMask junk3;

		(void)resize_in_x(layout, w,
			(int)(request->width - w->core.width),
			&junk1, &junk2, &layout_resized, TRUE, &junk3);

		if (!(layout_resized & CWWidth))
			w->core.width = compromise.width;
		XtMoveWidget(w, compromise.x, w->core.y);
	}

	/* we can resize in Y so this time REALLY do it */
	if (do_it_again & CWHeight) {
		Position junk1;
		int junk2;
		XtGeometryMask junk3;

		(void)resize_in_y(layout, w,
			(int)(request->height - w->core.height),
			&junk1, &junk2, &layout_resized, TRUE, &junk3);

		if (!(layout_resized & CWHeight))
			w->core.height = compromise.height;
		XtMoveWidget(w, w->core.x, compromise.y);
	}

	if (able_to_change & CWBorderWidth)
		w->core.border_width = compromise.border_width;

	/* layout had to resize so adjust the layout of every child */
	if (layout_resized & (CWWidth | CWHeight))
		Resize((Widget)layout);
	else {
		/* make the intrinsics adjust the widget */
		if (able_to_change & CWHeight)
			w->core.height = compromise.height;

		if (able_to_change & CWWidth)
			w->core.width = compromise.width;
	}

	if (do_it_again & CWY) {
		XtGeometryMask junk1;

		compromise.y = orig_y;
		(void)move_in_y(layout, w, request, &compromise, TRUE, &junk1);
	}

	if (do_it_again & CWX) {
		XtGeometryMask junk1;

		compromise.x = orig_x;
		(void)move_in_x(layout, w, request, &compromise, TRUE, &junk1);
	}

	if (layout_resized == NULL)
		XtMoveWidget(w, compromise.x, compromise.y);

#if 0   /* ||| see above ||| */
	XtMapWidget((Widget)layout);
#endif !DEBUG
	return(XtGeometryYes);
}


/*** Manipulation of connection lists by the size of `unfixed_dimension' ***/


/* comparison function used by qsort() in sort_list_by_unfixed() */
static int
unfixed_compare(element1, element2)
Connections element1, element2;
{
	Wconns con1, con2;

	con1 = Constraints_On_Child(element1->sibling, sort_axis);
	con2 = Constraints_On_Child(element2->sibling, sort_axis);

	if (con1->unfixed_dimension == con2->unfixed_dimension)
		return (0);

	/* list is sort with connections with the largest size of
	 * unfixed children at the head
	 */
	if (con1->unfixed_dimension < con2->unfixed_dimension)
		return (1);

	return (-1);
}


static void
sort_list_by_unfixed(layout, w, axis)
UKCLayoutWidget layout;
Widget w;
Cardinal axis;
{
	int i, num_cons = 0;
	Connections temp, temp2;
	Wconns cons;

	if (w == (Widget)layout)
		cons = &layout->layout.connections[axis];
	else
		cons = Constraints_On_Child(w, axis);

	for (temp = cons->next; temp != NULL; temp = temp->next)
		num_cons++;

	if (num_cons == NULL)
		return;

	temp = (Connections)XtCalloc(num_cons, sizeof(struct _wcl));

	/* copy over the connection list in to temp array */
	i = 0;
	for (temp2 = cons->next; temp2 != NULL; temp2 = temp2->next)
		temp[i++] = *temp2;

	/* sort the connections */
	sort_axis = axis;
	qsort((char *)temp, num_cons, sizeof(struct _wcl),  unfixed_compare);

	/* copy them back into the original memory space */
	i = 0;
	for (temp2 = cons->next; temp2 != NULL; temp2 = temp2->next) {
		temp2->sibling = temp[i].sibling;
		temp2->draw_pos = temp[i++].draw_pos;
	}

	XtFree((char *)temp);

	/* recurse for the children on this connection list */
	for (temp2 = cons->next; temp2 != NULL; temp2 = temp2->next)
		sort_list_by_unfixed(layout, temp2->sibling, axis);
}


/* recurse up the previous connection lists finding the highest number of
 * unfixed siblings. Needed by Resize() to do its job right.
 */
static void
bubble_unfixed(w, axis, unfixed_size, layout)
Widget w;
Cardinal axis;
double unfixed_size;
UKCLayoutWidget layout;
{
	Wconns conn_list = Constraints_On_Child(w, axis);
	Connections cur_prev;

	if (w == (Widget)layout)
		return;
	if (unfixed_size <= conn_list->unfixed_dimension)
		return;

	conn_list->unfixed_dimension = unfixed_size;
	unfixed_size += !(conn_list->fixed || !CONSTRAINTS_MANAGED(w)) ?
				conn_list->dimension : 0;

	for (cur_prev = conn_list->prev; cur_prev != NULL; ) {
		bubble_unfixed(cur_prev->sibling, axis, unfixed_size, layout);
		cur_prev = cur_prev->next;
	}
}


static void
bubble_all(layout, axis)
UKCLayoutWidget layout;
Cardinal axis;
{
	int round_loop, current;
	Wconns conn_list;
	Widget w;

	if (layout->core.being_destroyed)
		return;

	/* adjust the amount of unfixed space for each constrained widget */
	layout->layout.connections[axis].unfixed_dimension = 0;
	for (round_loop = 0; round_loop <= 1; round_loop++)
		for (current = 0; current < layout->composite.num_children;) {
			w = layout->composite.children[current];
			conn_list = Constraints_On_Child(w, axis);
			if (round_loop == 0)
				conn_list->unfixed_dimension = 0;
			else
#if 1
				bubble_unfixed(w, axis, 
						!(conn_list->fixed ||
						!CONSTRAINTS_MANAGED(w)) ?
						conn_list->dimension : 0,
						layout);
#else
				bubble_unfixed(w, axis, 0, layout);
#endif
			current++;
		};

	sort_list_by_unfixed(layout, (Widget)layout, axis);
}


/**** Insertion of children and addition of connections routines ****/


static void
add_connection(layout, axis, this_child, widget2connect)
UKCLayoutWidget layout;
Cardinal axis;
Widget this_child, widget2connect;
{
	double total_unfixed = 0;
	Wconns c1, c2;

	if (this_child == (Widget)layout) {
		XtWarning("Cannot force a connection from bottom or right of child to its UKCLayoutWidget parent");
		return;
	}
	else
		c1 = Constraints_On_Child(this_child, axis);

	if (widget2connect == (Widget)layout)
		c2 = &layout->layout.connections[axis];
	else
		c2 = Constraints_On_Child(widget2connect, axis);

	/* add the next connection from the widget2connect to this_child */

	if (c2->next == NULL) {
		/* no connections so allocate head of `next' list */
		c2->next = (Connections)XtMalloc(sizeof(struct _wcl));
		c2->next->next = NULL;
		c2->next->sibling = this_child;
		c2->next->draw_pos = 0;
	} else {
		Connections temp = c2->next;

		/* see if there is already a connection */
		if (temp->sibling != this_child) {
			/* find end of connection `next' list
			 * and add new connection there 
			 * unless we find an existing connection
			 * on the way down */
			while (temp->next != NULL) {
				temp = temp->next;
				if (temp->sibling == this_child)
					return;
			}
			temp->next = (Connections)XtMalloc(sizeof(struct _wcl));
			temp->next->next = NULL;
			temp->next->sibling = this_child;
			temp->next->draw_pos = 0;
		} /* else don't make a connection */
		else
			return;
	}

	/* add the previous connection from this_child to the widget2connect */

	if (c1->prev == NULL) {
		/* no connections so allocate head of `prev' list */
		c1->prev = (Connections)XtMalloc(sizeof(struct _wcl));
		c1->prev->next = NULL;
		c1->prev->sibling = widget2connect;
		c1->prev->draw_pos = 0;
	} else {
		Connections temp = c1->prev;

		if (c2->fixed) {
			/* find end of connection `prev' list and
			 * add new connection there */
			while (temp->next != NULL)
				temp = temp->next;

			temp->next = (Connections)XtMalloc(sizeof(struct _wcl));
			temp->next->next = NULL;
			temp->next->sibling = widget2connect;
			temp->next->draw_pos = 0;
		} else {
			/* unfixed siblings are put at head of list */
			c1->prev = (Connections)XtMalloc(sizeof(struct _wcl));
			c1->prev->sibling = widget2connect;
			c1->prev->next = temp;
		}
	}

	if (c1->next != NULL)
		total_unfixed = c1->unfixed_dimension;
	else
		total_unfixed = !(c1->fixed ||
				!(CONSTRAINTS_MANAGED(this_child)))
				? c1->dimension : 0;

	bubble_unfixed(this_child, axis, total_unfixed, layout);
	sort_list_by_unfixed(layout, (Widget)layout, axis);
}


/*	> Insert_child <
 */
static void
Insert_child(w)
register Widget w;
{
	UKCLayoutWidget layout = (UKCLayoutWidget)XtParent(w);
	ConstraintWidgetClass superclass = (ConstraintWidgetClass) ukcLayoutWidgetClass->core_class.superclass;

#ifdef DEBUG
	printf("Insert_Child(layout, %s)\n",w->core.name);
#endif
	if (XtIsWidget(w))
		XtAddEventHandler(w, EnterWindowMask, FALSE, lwEnteredWidget,
							   (XtPointer)layout);
	/* call layouts parent to do the normal insertion */
	(superclass->composite_class.insert_child)(w);
}


/**** Deletion of children and connections routines ****/


static Boolean
delete_multiple_prev_links(w, ch, axis, recurse)
Widget w, ch;
Cardinal axis;
Boolean recurse;
{
	Connections temp, temp2;
	Wconns conn_list = Constraints_On_Child(w, axis);
	Boolean result = FALSE;

	if (conn_list->prev == NULL)
		return (FALSE);

	temp = conn_list->prev;

	if (conn_list->prev->sibling == ch) {
		conn_list->prev = temp->next;
		XtFree((char *)temp);

		if (recurse)
			(void)delete_multiple_prev_links(w, ch, axis, recurse);
		result = TRUE;
	} else {
		while (temp->next != NULL) {
			if (temp->next->sibling == ch) {
				temp2 = temp->next;
				temp->next = temp->next->next;
				XtFree((char *)temp2);
				result = TRUE;
				if (!recurse)
					break;
			}
			else
				temp = temp->next;
		}
	}
	return (result);
}


static Boolean
delete_multiple_next_links(w, ch, axis, recurse, layout)
Widget w, ch;
Cardinal axis;
Boolean recurse;
UKCLayoutWidget layout;
{
	Connections temp, temp2;
	Boolean result = FALSE;
	Wconns conn_list;
	
	if (w == (Widget)layout)
		conn_list = &layout->layout.connections[axis];
	else
		conn_list = Constraints_On_Child(w, axis);

	if (conn_list->next == NULL)
		return (FALSE);

	temp = conn_list->next;

	if (conn_list->next->sibling == ch) {
		conn_list->next = temp->next;
		XtFree((char *)temp);

		if (recurse)
			(void)delete_multiple_next_links(w, ch, axis, recurse, layout);
		result = TRUE;
	} else {
		while (temp->next != NULL) {
			if (temp->next->sibling == ch) {
				temp2 = temp->next;
				temp->next = temp->next->next;
				XtFree((char *)temp2);
				result = TRUE;
				if (!recurse)
					break;
			}
			else
				temp = temp->next;
		}
	}
	return (result);
}


static void
remove_connection(widget2remove, this_child, axis, layout)
Widget widget2remove, this_child;
Cardinal axis;
UKCLayoutWidget layout;
{
	Boolean ret1, ret2;
	char mess[150];

	ret1 = delete_multiple_next_links(this_child, widget2remove, axis,
								TRUE, layout);
	ret2 = delete_multiple_prev_links(widget2remove, this_child, axis, TRUE);
	if (ret1 && ret2) {
		bubble_all(layout, axis);
		return; /* without error */
	}

	if (!ret1 && !ret2) {
		(void)sprintf(mess, "XukcLayoutRemoveConnection() : there is no connection between `%s' and `%s'",
			this_child->core.name, widget2remove->core.name);
		XtWarning(mess);
	} else {
		(void)sprintf(mess, "XukcLayoutRemoveConnection() : garbled connection list");
		XtError(mess);
		abort(-1);
	}
}


static void
copy_list(axis, layout, ph, nh)
Cardinal axis;
UKCLayoutWidget layout;
register Connections ph, nh;
{
	if (layout->core.being_destroyed)
		return;

	for (; nh != NULL; nh = nh->next)
		add_connection(layout, axis, nh->sibling, ph->sibling);
}


static void
copy_over_list(free_memory, axis, layout, ph, nh, ch)
Boolean free_memory;
Cardinal axis;
UKCLayoutWidget layout;
register Connections ph, nh;
Widget ch;
{
	register Connections temp, temp2;
	register Wconns conn_list;

	/* looking through the `previous' connection list, for
	 * each child connected check its `next' connection list
	 * for the `next' connection to this dying child.
	 */
	while (ph != NULL) {
		if (ph->sibling == (Widget)layout)
			conn_list = &layout->layout.connections[axis];
		else
			conn_list = Constraints_On_Child(ph->sibling, axis);

		if (conn_list->next->sibling == ch) {
			/* remove the connection to dying child */
			temp = conn_list->next;
			conn_list->next = temp->next;
			XtFree((char *)temp);

			copy_list(axis, layout, ph, nh);
		} else {
			/* remove any connections to the dying child
			 * and replace them with a copy of the dying childs
			 * `next' connection list.
			 */
			temp = conn_list->next;
			for (; temp->next != NULL; temp = temp->next) {
				if (temp->next->sibling == ch) {
					temp2 = temp->next;
					temp->next = temp->next->next;
					XtFree((char *)temp2);

					copy_list(axis, layout, ph, nh);
					break; /* out of the for loop */
				}
			}
		}

		(void)delete_multiple_next_links(ph->sibling, ch, axis, TRUE, layout);

		temp = ph->next; /* free the dying childs `previous' list */
		if (free_memory)
			XtFree((char *)ph);
		ph = temp; /* get the next `previous' connection */
	}

	while (nh != NULL) {
		(void)delete_multiple_prev_links(nh->sibling, ch, axis, TRUE);
		nh = nh->next;
	}
}


/* > UnlinkChild <
 * take this child out of the tree of connection lists, free any memory
 * allocated if required.
 */
static void
UnlinkChild(layout, managed, free_memory, axis, w)
UKCLayoutWidget layout;
Boolean managed;
Boolean free_memory;
Cardinal axis;
Widget w;
{
	Wconns conn_list = Constraints_On_Child(w, axis);
	Connections nh = conn_list->next;
	Connections ph = conn_list->prev;
	Connections temp;

	if (XtParent(w) != (Widget)layout) {
		XtError("UnlinkChild() called with non layout child");
		abort(-1);
	}

	if (managed)
		copy_over_list(free_memory, axis, layout, ph, nh, w);

	if (free_memory) {
		/* free memory used in list */
		while (nh != NULL) {
			temp = nh->next;
			XtFree((char *)nh);
			nh = temp;
		}
	}

	bubble_all(layout, axis);
}
	

/* > Delete_child <
 * remove event handler from dying child.
 */
static void
Delete_child(w)
Widget w;
{
	UKCLayoutWidget lw = (UKCLayoutWidget)XtParent(w);
	ConstraintWidgetClass superclass = (ConstraintWidgetClass) ukcLayoutWidgetClass->core_class.superclass;

	if (XtIsWidget(w))
		XtRemoveEventHandler(w, EnterWindowMask, FALSE,
					lwEnteredWidget, (XtPointer)lw);
	(superclass->composite_class.delete_child)(w);
}


/* ARGSUSED */
static void
constraint_Initialize(request, new)
Widget request, new;
{
	Wconns conn_list = Constraints_On_Child(new, VERTICAL);

	conn_list->resized = FALSE;
	conn_list->unfixed_dimension = 0;
	conn_list->dimension = (double)new->core.height /
					(double)XtParent(new)->core.height;
	conn_list->next = NULL;
	conn_list->prev = NULL;

	conn_list = Constraints_On_Child(new, HORIZONTAL);
	conn_list->resized = FALSE;
	conn_list->unfixed_dimension = 0;
	conn_list->dimension = (double)new->core.width /
					(double)XtParent(new)->core.width;
	conn_list->next = NULL;
	conn_list->prev = NULL;

	CONSTRAINTS_MANAGED(new) = FALSE;
}


/* > constraint_Destroy <
 * get rid of the connections to this widget and free the memory allocated.
 */
static void
constraint_Destroy(w)
Widget w;
{
	UKCLayoutWidget lw = (UKCLayoutWidget)XtParent(w);

	UnlinkChild(lw, TRUE, TRUE, HORIZONTAL, w);
	UnlinkChild(lw, TRUE, TRUE, VERTICAL, w);

	if (w == lw->layout.current_widget)
		lw->layout.current_widget = NULL;
}


/* > constraint_SetValues <
 * if the widget has been fixed or unfixed in either axis and it is managed
 * then bubble the number of unfixed siblings up the constraint list tree
 */
/* ARGSUSED */
static Boolean
constraint_SetValues(current, request, new)
Widget current, request, new;
{
	Wconns oldconns, newconns;
	UKCLayoutWidget lw = (UKCLayoutWidget)XtParent(new);
	Boolean do_bubble = FALSE;
	Boolean do_refresh = FALSE;

	oldconns = Constraints_On_Child(current, HORIZONTAL);
	newconns = Constraints_On_Child(new, HORIZONTAL);

	do_bubble = (oldconns->fixed != newconns->fixed);
	if (new->core.width != current->core.width)
		newconns->dimension = (double)new->core.width /
						(double)lw->core.width;

	oldconns = Constraints_On_Child(current, VERTICAL);
	newconns = Constraints_On_Child(new, VERTICAL);

	do_bubble |= (oldconns->fixed != newconns->fixed);
	if (new->core.height != current->core.height)
		newconns->dimension = (double)new->core.height /
						(double)lw->core.height;

	if (CONSTRAINTS_MANAGED(new) && do_bubble) {
		bubble_all(lw, HORIZONTAL);
		bubble_all(lw, VERTICAL);
		do_refresh = TRUE;
	}
	if (do_refresh ||
	    new->core.x != current->core.x ||
	    new->core.y != current->core.y ||
	    new->core.width != current->core.width ||
	    new->core.height != current->core.height)
		XukcLayoutRefreshConnections(lw);
	return (FALSE);
}


static void
now_managed(layout, axis, conn_list)
UKCLayoutWidget layout;
Cardinal axis;
Wconns conn_list;
{
	Connections nl, pl;

	/* remove the connections given to its siblings when this
	 * child was made unmanaged
	 */
	for (nl = conn_list->next; nl != NULL; nl = nl->next) {
		for (pl = conn_list->prev; pl != NULL; pl = pl->next) {
			(void)delete_multiple_next_links(pl->sibling,
					nl->sibling, axis, FALSE, layout);
			(void)delete_multiple_prev_links(nl->sibling,
						pl->sibling, axis, FALSE);
		}
	}
}


/* ARGSUSED */
static void
Change_managed(w)
Widget w;
{
	UKCLayoutWidget layout = (UKCLayoutWidget)w;
	Cardinal i;
	Widget cw;
	Wconns conn_list;
	Connections temp;
#ifdef DEBUG
	printf("Change_managed(layout, %s)\n",w->core.name);
#endif
	layout->layout.managed_once = TRUE;

	for (i = layout->composite.num_children; i > 0; i--) {
		cw = layout->composite.children[i-1];
		if ((CONSTRAINTS_MANAGED(cw) != cw->core.managed)
		   || (layout->layout.resizing & RESIZING)) {
#ifdef DEBUG
printf("\"%s\" has now become %s\n", cw->core.name, cw->core.managed ? "Managed" : "Unmanaged");
#endif
			CONSTRAINTS_MANAGED(cw) = cw->core.managed;

			if (cw->core.managed) {
			    if ((layout->layout.resizing == RESIZING_WITH_FORCE)
				|| (layout->layout.resizing == NOT_RESIZING)) {
				now_managed(layout, HORIZONTAL,
					Constraints_On_Child(cw, HORIZONTAL));
				now_managed(layout, VERTICAL,
					Constraints_On_Child(cw, VERTICAL));
			    } else {
				conn_list = Constraints_On_Child(cw, HORIZONTAL);
				temp = conn_list->prev;
				for (; temp != NULL; temp = temp->next)
					copy_list(HORIZONTAL, layout, temp,
							conn_list->next);
				conn_list = Constraints_On_Child(cw, VERTICAL);
				temp = conn_list->prev;
				for (; temp != NULL; temp = temp->next)
					copy_list(VERTICAL, layout, temp,
							conn_list->next);
				CONSTRAINTS_MANAGED(cw) = FALSE;
			    }
			} else {
				conn_list = Constraints_On_Child(cw, HORIZONTAL);
				temp = conn_list->prev;
				for (; temp != NULL; temp = temp->next)
					copy_list(HORIZONTAL, layout, temp,
							conn_list->next);
				conn_list = Constraints_On_Child(cw, VERTICAL);
				temp = conn_list->prev;
				for (; temp != NULL; temp = temp->next)
					copy_list(VERTICAL, layout, temp,
							conn_list->next);
			}
		}
	}
	bubble_all(layout, HORIZONTAL);
	bubble_all(layout, VERTICAL);
	XukcLayoutRefreshConnections(w);
}


/**** Internal Routines separate to standard X Toolkit widget routines ****/


static void
lwTooSmall(w, too_narrow, too_short)
Widget w;
Boolean too_narrow;
Boolean too_short;
{
	Arg	l_args[5];
	UKCLayoutWidget layout = (UKCLayoutWidget) w;
	Cardinal i = 0;

	layout->layout.drawing_connections = FALSE;

	if (layout->layout.too_small_used &&
	    XtIsSubclass(XtParent(w), shellWidgetClass))
		XtRemoveGrab(layout->layout.too_small_widget);

	if (too_narrow && too_short)
		XtSetArg(l_args[i], XtNlabel, (XtArgVal) "Too Short & Narrow");
	else {
		if (too_narrow)
			XtSetArg(l_args[i], XtNlabel, (XtArgVal) "Window Too Narrow");
		else
			XtSetArg(l_args[i], XtNlabel, (XtArgVal) "Window Too Short");
	}

	i++;

	if (layout->layout.too_small_widget == NULL) {
		XtSetArg(l_args[i], XtNx, 0);	i++;
		XtSetArg(l_args[i], XtNy, 0);	i++;
		XtSetArg(l_args[i], XtNborderWidth, 0);	i++;

		layout->layout.too_small_widget = XukcLayoutCreateWidget("TooSmall",
			labelWidgetClass, (Widget)layout, (ArgList)l_args, i,
			(Widget)layout, (Widget)layout, TRUE, TRUE);
	}
	else
		XtSetValues(layout->layout.too_small_widget, l_args, i);

	XtResizeWidget(layout->layout.too_small_widget, layout->core.width,
			layout->core.height, 0);
	layout->layout.too_small_used = TRUE;
	XtManageChild(layout->layout.too_small_widget);
	if (XtIsSubclass(XtParent(layout), shellWidgetClass))
		XtAddGrab(layout->layout.too_small_widget, TRUE, FALSE);
}


static void
recursive_print_connections(layout, w, conn_list, axis, indent_level)
UKCLayoutWidget layout;
Widget w;
Wconns conn_list;
Cardinal axis;
int indent_level;
{
	Boolean more = FALSE;
	Connections np;
	
	np = conn_list->next;

	printf("%s", w->core.name);
	indent_level += strlen(w->core.name);

	if (conn_list->fixed) {
		printf(" (F)");
		indent_level += 4;
	}
	printf("(%4d)", conn_list->unfixed_dimension);
	indent_level += 6;

	if (((Widget)layout != w) && !CONSTRAINTS_MANAGED(w)) {
		printf(" (U)");
		return;
	}

	while (np != NULL) {
		more |= (np->next != NULL);
		printf(" -> ");
		recursive_print_connections(layout, np->sibling,
				Constraints_On_Child(np->sibling, axis),
				axis, indent_level + 4);
		np = np->next;

		if (more) {
			int i = indent_level;
			putchar('\n');
			while (i-- > 0)
				putchar(' ');
			more = FALSE;
		}
	}
}
		

static int
position_compare(element1, element2)
Connections element1, element2;
{
	if (element1->draw_pos == element2->draw_pos)
		return (0);

	if (element1->draw_pos > element2->draw_pos)
		return (1);

	return (-1);
}


static void
sort_list_by_pos(norp_list, space, axis, layout)
Connections norp_list;
Dimension space;
Cardinal axis;
UKCLayoutWidget layout;
{
	Widget w;
	Connections temp, temp_list;
	int i = 0;
	int num_norps = 0;
	Dimension inc_space;

	for (temp = norp_list; temp != NULL; temp = temp->next) {
		if (((Widget)layout == temp->sibling) ||
		   CONSTRAINTS_MANAGED(temp->sibling))
			num_norps++;
	}

	if (num_norps == 0)
		return;

	temp = (Connections)XtCalloc(num_norps, sizeof(struct _wcl));
	temp_list = norp_list;
	inc_space = space / (num_norps+1);

	for (i = num_norps; temp_list != NULL; temp_list = temp_list->next) {
		if ((temp_list->sibling != (Widget)layout) &&
		     !CONSTRAINTS_MANAGED(temp_list->sibling))
			continue;
		i--;
#ifdef DEBUG
if (i < 0) {
	XtError("Ouch: num_norps mismatch in sort_list_by_pos()");
	abort(-1);
}
#endif
		w = temp_list->sibling;
		temp[i].draw_pos = axis == HORIZONTAL ? 
			(w->core.y + w->core.border_width + w->core.height/2) :
			(w->core.x + w->core.border_width + w->core.width/2);
		temp[i].sibling = w;
	}

	/* quick sort */
	qsort((char *)temp, num_norps, sizeof(struct _wcl), position_compare);

	temp_list = norp_list;
	while (temp_list != NULL) {
		for (i = num_norps; i > 0; ) {
			i--;
			if (temp[i].sibling == temp_list->sibling)
				temp_list->draw_pos = (i+1)*inc_space - LINE_WIDTH/2;
		}

		temp_list = temp_list->next;
	}

	XtFree((char *)temp);
}


static void draw_lines(dw, this_child, gc_unfixed, gc_fixed, axis, layout)
Widget dw, this_child;
GC gc_unfixed, gc_fixed;
Cardinal axis;
UKCLayoutWidget layout;
{
	Widget w1, w2;
	Connections from, to;
	int x1, y1, x2, y2;
	Wconns conn_list;

	if (!(CONSTRAINTS_MANAGED(this_child) && XtIsRealized(this_child)))
		return;

	w1 = this_child;
	conn_list = Constraints_On_Child(this_child, axis); 
	from = conn_list->prev;
	for (from = conn_list->prev; from != NULL; from = from->next) {
		if (from->sibling == (Widget)layout)
			conn_list = &layout->layout.connections[axis];
		else {
			conn_list = Constraints_On_Child(from->sibling, axis);
			if (!CONSTRAINTS_MANAGED(from->sibling))
				continue;
		}

		to = conn_list->next;
		while ((to != NULL) && (to->sibling != this_child))
			to = to->next;

		if (to == NULL) {
			XtError("Corruption of UKCLayoutWidget connection list");
			abort(-1);
		}

		w2 = from->sibling;

		if (axis == HORIZONTAL) {
			x1 = w1->core.x + w1->core.border_width + IN_DISTANCE;
			y1 = w1->core.y + w1->core.border_width + from->draw_pos;
			if (w2 == (Widget)layout) {
				x2 = 0;
				y2 = y1;
			} else {
				x2 = w2->core.x + w2->core.width
					+ w2->core.border_width - IN_DISTANCE;
				y2 = w2->core.y + w2->core.border_width
					+ to->draw_pos;
			}
		} else {
			x1 = w1->core.x + w1->core.border_width + from->draw_pos;
			y1 = w1->core.y + w1->core.border_width + IN_DISTANCE;
			if (w2 == (Widget)layout) {
				x2 = x1;
				y2 = 0;
			} else {
				x2 = w2->core.x + w2->core.border_width
					+ to->draw_pos;
				y2 = w2->core.y + w2->core.height
					+ w2->core.border_width - IN_DISTANCE;
			}
		}
		XDrawLine(XtDisplay(dw), XtWindow(dw),
			  conn_list->fixed ? gc_fixed : gc_unfixed,
			  x1, y1, x2, y2);
	}

	conn_list = Constraints_On_Child(this_child, axis);
	for (to = conn_list->next; to != NULL; to = to->next)
		if (CONSTRAINTS_MANAGED(to->sibling))
			break;

	/* no next connections therefore we are connected to the UKCLayoutWidget */
	if (to == NULL) {
		if (axis == HORIZONTAL) {
			y1 = WIDGET_Y(w1) + WIDGET_BORDER_WIDTH(w1)
				+ WIDGET_HEIGHT(w1)/2;
			x1 = WIDGET_X(w1) + WIDGET_BORDER_WIDTH(w1)
				+ WIDGET_WIDTH(w1) - IN_DISTANCE;
			y2 = y1;
			x2 = WIDGET_WIDTH(layout);
		} else {
			x1 = w1->core.x + w1->core.border_width
				+ w1->core.width/2;
			y1 = w1->core.y + w1->core.border_width
				+ w1->core.height - IN_DISTANCE;
			y2 = layout->core.height;
			x2 = x1;
		}
		XDrawLine(XtDisplay(dw), XtWindow(dw),
			  (Constraints_On_Child(w1, axis))->fixed ? gc_fixed :
			  gc_unfixed, x1, y1, x2, y2);
	}
}


/***** ACTION Routines *****/


/* ARGSUSED */
static void ClearConnections(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLayoutWidget lw;

	while (w != NULL && !XtIsSubclass(w, ukcLayoutWidgetClass))
		w = XtParent(w);

	if (w == NULL) {
		XtWarning("ClearConnections() action called by non-UKCLayoutWidget parent or child");
		return;
	}

	lw = (UKCLayoutWidget)w;
	if (lw->layout.drawing_connections == TRUE) {
		lw->layout.drawing_connections = FALSE;
		XtUnmapWidget(w);
		XtMapWidget(w);
		XFlush(XtDisplay(w));
	}
}


/* ARGSUSED */
static void DrawConnections(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	GC gcuv, gcfv, gcuh, gcfh;
	UKCLayoutWidget lw = (UKCLayoutWidget)w;
	unsigned long gc_mask;
	XGCValues gc_values;
	int i = lw->composite.num_children-1;
	Wconns conn_list;

	if (!XtIsSubclass(w, ukcLayoutWidgetClass)) {
	        XtWarning("DrawConnections() called by non ukcLayoutWidget");
		return;
	}

	lw->layout.drawing_connections = TRUE;

	gc_mask = GCForeground | GCLineWidth | GCJoinStyle | GCSubwindowMode;
	gc_values.line_style = LineOnOffDash;
	gc_values.line_width = LINE_WIDTH;
	gc_values.join_style = JoinBevel;
	gc_values.subwindow_mode = IncludeInferiors;
	gc_values.foreground = lw->layout.draw_vert_foreground;
	gcfv = XtGetGC(w, gc_mask, &gc_values);
	gcuv = XtGetGC(w, gc_mask | GCLineStyle, &gc_values);
	gc_values.foreground = lw->layout.draw_horiz_foreground;
	gcfh = XtGetGC(w, gc_mask, &gc_values);
	gcuh = XtGetGC(w, gc_mask | GCLineStyle, &gc_values);

	for (; i >= 0; i--) {
		if (!(CONSTRAINTS_MANAGED(lw->composite.children[i]) &&
			lw->composite.children[i]->core.mapped_when_managed &&
			XtIsRealized(lw->composite.children[i])))
			continue;

		conn_list = Constraints_On_Child(lw->composite.children[i],
								VERTICAL);
		sort_list_by_pos(conn_list->next,
				lw->composite.children[i]->core.width,
				VERTICAL, lw);
		sort_list_by_pos(conn_list->prev,
				lw->composite.children[i]->core.width,
				VERTICAL, lw);
		conn_list = Constraints_On_Child(lw->composite.children[i],
								HORIZONTAL);
		sort_list_by_pos(conn_list->next,
				 lw->composite.children[i]->core.height,
				 HORIZONTAL, lw);
		sort_list_by_pos(conn_list->prev,
				 lw->composite.children[i]->core.height,
				 HORIZONTAL, lw);
	}

	for (i = lw->composite.num_children-1; i >= 0; i--) {
		draw_lines(w, lw->composite.children[i], gcuv, gcfv,
								VERTICAL, lw);
		draw_lines(w, lw->composite.children[i], gcuh, gcfh,
								HORIZONTAL, lw);
	}
	XtReleaseGC(w, gcuv);	XtReleaseGC(w, gcfv);
	XtReleaseGC(w, gcuh);	XtReleaseGC(w, gcfh);
}


/* ARGSUSED */
static void PrintWidgetConnections(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLayoutWidget lw = (UKCLayoutWidget)w;
	Widget cw;

	if (!XtIsSubclass(w, ukcLayoutWidgetClass)) {
		XtWarning("PrintWidgetConnections() called by non layout widget");
		return;
	}

	cw = lw->layout.current_widget;
	if (cw == NULL)
		return;
	printf("** Connection List for \"%s\" **\nHorizontal = ", cw->core.name);
	recursive_print_connections(lw, cw, Constraints_On_Child(cw, HORIZONTAL),
								HORIZONTAL, 13);
	printf("\nVertical = ");
	recursive_print_connections(lw, cw, Constraints_On_Child(cw, VERTICAL),
								VERTICAL, 11);
	putchar('\n');	
}


/* ARGSUSED */
static void PrintConnections(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLayoutWidget layout = (UKCLayoutWidget) w;

	if (!XtIsSubclass(w, ukcLayoutWidgetClass)) {
		XtWarning("PrintConnections() called by non layout widget");
		return;
	}

	printf("*** CONNECTION LIST for \"%s\" UKCLayoutWidget ***\nHorizontal connections :\n", w->core.name);
	recursive_print_connections(layout, w, &layout->layout.connections[HORIZONTAL], HORIZONTAL, 0);
	printf("\nVertical connections:\n");
	recursive_print_connections(layout, w, &layout->layout.connections[VERTICAL], VERTICAL, 0);
	putchar('\n');
}


/**** Child Event Handlers ****/


/* ARGSUSED */
static void lwEnteredWidget(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	UKCLayoutWidget lw = (UKCLayoutWidget) client_data;

	if (!XtIsSubclass((Widget)lw, ukcLayoutWidgetClass)) {
		XtError("Bad UKCLayout Widget passed to lwEnteredWidget");
		abort(-1);
	}
	lw->layout.current_widget = w;
}

/**** PUBLIC ROUTINES ****/


void
XukcLayoutRefreshConnections(w)
Widget w;
{
	UKCLayoutWidget layout_widget = (UKCLayoutWidget)w;

	if (layout_widget->layout.drawing_connections) {
		ClearConnections(w, (XEvent *)NULL, (String *)NULL,
							(Cardinal *)NULL);
		layout_widget->layout.drawing_connections = TRUE;
	}
}

	
void
XukcDisplayLayoutConnections(w, state)
Widget w;
Boolean state;
{
	if (state)
		DrawConnections(w, (XEvent *)NULL, (String *)NULL,
							(Cardinal *)NULL);
	else
		ClearConnections(w, (XEvent *)NULL, (String *)NULL,
							(Cardinal *)NULL);
}


Widget
XukcLayoutCreateWidget(name, widget_class, parent, args, num_args, widget2left,
					widget_above, x_is_fixed, y_is_fixed)
String name;
WidgetClass widget_class;
Widget parent;
ArgList args;
Cardinal num_args;
Widget widget2left, widget_above;
Boolean x_is_fixed, y_is_fixed;
{
	Arg	l_args[2];
	ArgList n_args;
	int	i = 0;
	Widget new_widget;

	XtSetArg(l_args[i], XtNhorizFixed, (XtArgVal)x_is_fixed); i++;
	XtSetArg(l_args[i], XtNvertFixed, (XtArgVal)y_is_fixed); i++;
	n_args = XtMergeArgLists(args, num_args, (ArgList)l_args, 2);

	new_widget = XtCreateManagedWidget(name, widget_class, parent, n_args, num_args + 2);
	XtFree((char *)n_args);
	XukcLayoutAddConnection((UKCLayoutWidget)parent, new_widget, widget2left, horizontal_before);
	XukcLayoutAddConnection((UKCLayoutWidget)parent, new_widget, widget_above, vertical_above);

	return (new_widget);
}


void
AddOrRemoveConnection(add, layout_widget, widget, widget2connect2, positioning)
Boolean add;
UKCLayoutWidget layout_widget;
Widget widget;
Widget widget2connect2;
Positioning positioning;
{
	Boolean above_before;
	Cardinal axis;

	if (!XtIsSubclass((Widget)layout_widget, ukcLayoutWidgetClass)) {
		if (add)
			XtWarning("XukcLayoutAddConnection() called with non ukcLayoutWidgetClass widget (2nd parameter)");
		else
			XtWarning("XukcLayoutRemoveConnection() called with non ukcLayoutWidgetClass widget (2nd parameter)");
		return;
	}

	switch (positioning) {
		case vertical_above :
			axis = VERTICAL;
			above_before = TRUE;
			break;
		case vertical_below :
			axis = VERTICAL;
			above_before = FALSE;
			break;
		case horizontal_after :
			axis = HORIZONTAL;
			above_before = FALSE;
			break;
		default :
		case horizontal_before :
			axis = HORIZONTAL;
			above_before = TRUE;
			break;
	}

	if (widget == NULL)
		widget = (Widget)layout_widget;
	if (widget2connect2 == NULL)
		widget2connect2 = (Widget)layout_widget;

	if (((widget != (Widget)layout_widget) &&
	     (widget2connect2 != (Widget)layout_widget)) &&
	    ((XtParent(widget) != (Widget)layout_widget) ||
	     (XtParent(widget2connect2) != (Widget)layout_widget))) {
		if (add)
			XtWarning("XukcLayoutAddConnection() widget given as 2nd or 3rd parameter is not a child of the given UKCLayoutWidget");
		else
			XtWarning("XukcLayoutRemoveConnection() widget given as 2nd or 3rd parameter is not a child of the given UKCLayoutWidget");
		return;
	}

	if (widget == widget2connect2) {
		if (add)
			XtWarning("XukcLayoutAddConnection() cannot connect a child to itself");
		else
			XtWarning("XukcLayoutRemoveConnection() cannot connect a child to itself");
		return;
	}

	if (above_before) {
		if (add)
		    add_connection(layout_widget, axis, widget, widget2connect2);
		else
		    remove_connection(widget, widget2connect2, axis, layout_widget);
	} else {
		if (add)
		    add_connection(layout_widget, axis, widget2connect2, widget);
		else
		    remove_connection(widget2connect2, widget, axis, layout_widget);
	}
}
