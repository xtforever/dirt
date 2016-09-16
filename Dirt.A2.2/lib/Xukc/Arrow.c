#ifndef lint
static char sccsid[] = "@(#)Arrow.c	1.2 (UKC) 5/10/92";
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

/* UKCArrow Widget - displays a arrow, one of any four directions.
 *                Allows a callback to be called whenever the arrow is pressed.
 *                Holding down the button will cause a repeated call of
 *                the callback to be made.  The time before the first repeat
 *                and subsequent repeats is programmable, so is the foreground
 *		  colour and arrow line thickness.
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/Convert.h>
#include "ArrowP.h"

#define BIG_SHIFT 100000
#define THICKNESS 0		/* pixels - zero is quick and dirty! */
#define START_TIME 800		/* milliseconds */
#define REPEAT_TIME 200		/* milliseconds */

static XtResource resources[] = {
#define offset(field) XtOffset(UKCArrowWidget, arrow.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNthickness, XtCThickness, XtRDimension, sizeof(Dimension),
	offset(thickness), XtRImmediate, (XtPointer)THICKNESS },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(foreground), XtRString, XtExtdefaultforeground },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	offset(callbacks), XtRCallback, (XtPointer)NULL },
    { XtNorientation, XtCOrientation, XtRArrowOrientation,
	sizeof(int), offset(orientation), XtRImmediate, (XtPointer)NORTH },
    { XtNfilled, XtCFilled, XtRBoolean, sizeof(Boolean),
	offset(filled), XtRImmediate, (XtPointer)FALSE },
    { XtNstartTimeout, XtCTimeout, XtRInt, sizeof(int),
	offset(start_timeout), XtRImmediate, (XtPointer)START_TIME },
    { XtNrepeatTimeout, XtCTimeout, XtRInt, sizeof(int),
	offset(repeat_timeout), XtRImmediate, (XtPointer)REPEAT_TIME },
    { XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
	XtOffset(UKCArrowWidget, simple.cursor), XtRString, "hand1" },
#undef offset
};

static void Initialize();
static void ClassInitialize();
static void Redisplay();
static void Resize();
static void Destroy();
static Boolean SetValues();

static void invertArrow();
static void makeGC();
static void changeSize();

static void RepeatTimeout();
static void ArrowAction();
static void InvertAction();

static XtActionsRec actions[] = {
	{"arrow", ArrowAction},
	{"invert", InvertAction},
};

static char translations[] =
	"<Btn1Down>:	invert() arrow() \n\
	 <Btn1Up>:	invert()";

UKCArrowClassRec ukcArrowClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &simpleClassRec,
    /* class_name		*/	"UKCArrow",
    /* widget_size		*/	sizeof(UKCArrowRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	translations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	XtInheritChangeSensitive
  },
  { /* arrow fields */
    /* empty			*/	0
  }
};

WidgetClass ukcArrowWidgetClass = (WidgetClass)&ukcArrowClassRec;

static Dimension default_widths[4] = { 15, 15, 13, 13 };
static Dimension default_heights[4] = { 13, 13, 15, 15 };

/* These points define the lines that make up the arrow .. in all four
 * orientations.
 */
#define NUM_POINTS 8
static XPoint arrows[4][NUM_POINTS] = {
	{ {7,2}, {12,7}, {10,7}, {10,10}, {4,10}, {4,7}, {2,7}, {7,2} },
	{ {4,2}, {10,2}, {10,5}, {12,5}, {7,10}, {2,5}, {4,5}, {4,2} },
	{ {7,2}, {7,4}, {10,4}, {10,10}, {7,10}, {7,12}, {2,7}, {7,2} },
	{ {2,4}, {5,4}, {5,2}, {10,7}, {5,12}, {5,10}, {2,10}, {2,4} }
};


/* for resource converters */

static EnumListRec valueList[] = {
	{ XtENorth, NORTH },
	{ XtESouth, SOUTH },
	{ XtEEast, EAST },
	{ XtEWest, WEST },
};

static XtConvertArgRec convertArgs[] = {
	{ XtAddress,	(XtPointer)valueList, sizeof(EnumList) },
	{ XtImmediate,	(XtPointer)XtNumber(valueList), sizeof(Cardinal) },
	{ XtAddress,	XtRArrowOrientation, sizeof(String) },
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
	XtSetTypeConverter(XtRString, XtRArrowOrientation,
			XukcCvtStringToNamedValue,
			convertArgs, XtNumber(convertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRArrowOrientation, XtRString,
			XukcCvtNamedValueToString,
			convertArgs, XtNumber(convertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
}


/* > Initialize < */
/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCArrowWidget aw = (UKCArrowWidget) new;

	if (aw->core.width == 0)
		aw->core.width = default_widths[aw->arrow.orientation];

	if (aw->core.height == 0)
		aw->core.height = default_heights[aw->arrow.orientation];

	aw->arrow.inverted = FALSE;
	makeGC(aw);
	changeSize(aw, (int)(aw->core.width * BIG_SHIFT
				/ default_widths[aw->arrow.orientation]),
		   (int)(aw->core.height * BIG_SHIFT /
			   default_heights[aw->arrow.orientation]));

	aw->arrow.timeout_id = 0;
}

/* ARGSUSED */
static void
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
	GC fill_gc_not_inverted, fill_gc_inverted;
	UKCArrowWidget aw = (UKCArrowWidget) w;

	if (aw->arrow.filled) {
		fill_gc_inverted = aw->arrow.invert_gc;
		fill_gc_not_inverted = aw->arrow.arrow_gc;
	} else {
		fill_gc_inverted = aw->arrow.arrow_gc;
		fill_gc_not_inverted = aw->arrow.invert_gc;
	}
	XFillPolygon(XtDisplay(w), XtWindow(w),
			aw->arrow.inverted ?
			fill_gc_inverted : fill_gc_not_inverted,
			aw->arrow.arrows, NUM_POINTS, Convex, CoordModeOrigin);

	XDrawLines(XtDisplay(w), XtWindow(w), aw->arrow.arrow_gc,
				aw->arrow.arrows, NUM_POINTS, CoordModeOrigin);
}


static void
Destroy(w)
Widget w;
{
	UKCArrowWidget aw = (UKCArrowWidget)w;
	XtReleaseGC(w, aw->arrow.arrow_gc);
	if (aw->arrow.timeout_id != 0)
		XtRemoveTimeOut(aw->arrow.timeout_id);
}


/*
 * Set specified arguments into widget
 */
/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
	UKCArrowWidget oldaw = (UKCArrowWidget) current;
	UKCArrowWidget aw = (UKCArrowWidget) new;
	Boolean redisplay = False;

	if (aw->arrow.thickness != oldaw->arrow.thickness ||
	    aw->arrow.foreground != oldaw->arrow.foreground) {
		XtReleaseGC(new, aw->arrow.arrow_gc);
		XtReleaseGC(new, aw->arrow.invert_gc);
		makeGC(aw);
		redisplay = TRUE;
	}

	if (aw->arrow.filled != oldaw->arrow.filled)
		redisplay = TRUE;

	if (aw->arrow.orientation != oldaw->arrow.orientation) {
		changeSize(aw, (int)(aw->core.width * BIG_SHIFT /
			   default_widths[aw->arrow.orientation]),
			   (int)(aw->core.height * BIG_SHIFT /
			   default_heights[aw->arrow.orientation]));
		redisplay = TRUE;
	}
	return (redisplay);
}


static void Resize(w)
Widget w;
{
	UKCArrowWidget aw = (UKCArrowWidget)w;

	/* Algorithm to increase the length of the lines and move the points */

	changeSize(aw, (int)(aw->core.width * BIG_SHIFT /
			   default_widths[aw->arrow.orientation]),
		   (int)(aw->core.height * BIG_SHIFT /
			   default_heights[aw->arrow.orientation]));
}


static void
invertArrow(aw)
UKCArrowWidget aw;
{
	WidgetClass wc = XtClass(aw);
	aw->arrow.inverted = !aw->arrow.inverted;

	(*wc->core_class.expose)((Widget)aw, (XEvent *)NULL, (Region)NULL);
}


static void
makeGC(aw)
UKCArrowWidget aw;
{
	XGCValues values;

	values.foreground = aw->arrow.foreground;
	values.line_width = aw->arrow.thickness;
	aw->arrow.arrow_gc = XtGetGC((Widget)aw, GCForeground | GCLineWidth,
								&values);
	values.foreground = aw->core.background_pixel;
	aw->arrow.invert_gc = XtGetGC((Widget)aw, GCForeground, &values);
}


static void
changeSize(aw, inc_w, inc_h)
UKCArrowWidget aw;
int inc_w, inc_h;
{
	Cardinal i;

	for (i = 0; i < NUM_POINTS; i++) {
		aw->arrow.arrows[i].x = (Dimension)(arrows[aw->arrow.orientation][i].x * inc_w / BIG_SHIFT + 0.5);
		aw->arrow.arrows[i].y = (Dimension)(arrows[aw->arrow.orientation][i].y * inc_h / BIG_SHIFT + 0.5);
	}
}


/* ARGSUSED */
static void
RepeatTimeout(client_data, id)
XtPointer client_data;
XtIntervalId *id;
{
	UKCArrowWidget aw = (UKCArrowWidget)client_data;

	invertArrow(aw);

	if (aw->arrow.repeat_timeout > 0)
		aw->arrow.timeout_id = XtAppAddTimeOut(
				XtWidgetToApplicationContext((Widget)aw),
				(unsigned long)aw->arrow.repeat_timeout,
				RepeatTimeout, (XtPointer)((Widget)aw));

	if (XtHasCallbacks((Widget)aw, XtNcallback))
		XtCallCallbacks((Widget)aw, XtNcallback, (XtPointer)aw);

	invertArrow(aw);
}
	

/* ARGSUSED */
static void
ArrowAction(w, event, params, num_params)
Widget w;
XEvent* event;
String* params;
Cardinal* num_params;
{
	UKCArrowWidget aw = (UKCArrowWidget)w;

	if (aw->arrow.start_timeout > 0)
		aw->arrow.timeout_id = XtAppAddTimeOut(
				XtWidgetToApplicationContext(w),
				(unsigned long)aw->arrow.start_timeout,
				RepeatTimeout, (XtPointer)aw);

	if (XtHasCallbacks(w, XtNcallback))
		XtCallCallbacks(w, XtNcallback, (XtPointer)w);
}


/* ARGSUSED */
static void
InvertAction(w, event, params, num_params)
Widget w;
XEvent* event;
String* params;
Cardinal* num_params;
{
	UKCArrowWidget aw = (UKCArrowWidget)w;

	invertArrow(aw);

	if (aw->arrow.timeout_id > 0) {
		XtRemoveTimeOut(aw->arrow.timeout_id);
		aw->arrow.timeout_id = 0;
	}
}
