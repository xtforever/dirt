#ifndef lint
static char sccsid[] = "@(#)gravity.c	1.2 (UKC) 5/10/92";
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

/* contains code that deals with the "gravitational layout lines".
 */
/* ||| remote instances have not been fully supported here: it WILL BREAK ||| */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/Med3Menu.h>
#include "dirt_structures.h"
#include "dirt_bindings.h"

#define GRAV_PULL 10

extern ObjectPtr NearestWidgetObject(), GetHeadObject();
extern ObjectPtr GetStartOfList();
extern Window XukcSelectPoint();
extern Widget ObjectWindowToWidget();

extern struct app_resources editor_resources;

struct gravity_line {
	ObjectPtr object;
	Boolean vertical_line;
	Position line_pos;
	char gravitational_pull;
};

typedef struct gravity_line GravLineRec;
typedef GravLineRec *GravLinePtr;

static Cardinal gravity_line_list = 0;
static GC line_on_gc = NULL, line_off_gc = NULL;
static UKCMed3MenuWidget menu = NULL;
static Boolean show_lines = FALSE;

void show_gravity_lines();


/* turn Tidy Lines function on and off */
static void
tidy_function(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (show_lines)
		XukcTurnOnButton(menu, TIDY);
	else
		XukcTurnOffButton(menu, TIDY);
	show_gravity_lines(w, client_data, call_data);
}


static void
make_line_gcs(object)
ObjectPtr object;
{
	XGCValues values;

	/* ||| need to make checks here for remote instances (if allowed) ||| */
	values.foreground = editor_resources.grav_colour;
	values.background = editor_resources.background;
	values.line_width = 0;

	line_on_gc = XtGetGC(object->instance, GCLineWidth | GCForeground |
				GCBackground, &values);

	values.foreground = editor_resources.background;
	values.background = editor_resources.grav_colour;

	line_off_gc = XtGetGC(object->instance, GCLineWidth | GCForeground |
				GCBackground, &values);
}


static void
add_gravity_line(object, pos, vertical, pull)
ObjectPtr object;
Position pos;
Boolean vertical;
char pull;
{
	GravLinePtr new;
	
	if (object == NULL)
		return;

	new = XtNew(GravLineRec);
	new->object = object;
	new->vertical_line = vertical;
	new->line_pos = pos;
	new->gravitational_pull = pull;

	(void)XukcAddObjectToList(&gravity_line_list, (XtPointer)new, FALSE);
}


static void
remove_gravity_line(object, pos, vertical)
ObjectPtr object;
Position pos;
Boolean vertical;
{
	GravLinePtr *lines;
	Cardinal num_lines;

	if (gravity_line_list == 0 || object == NULL)
		return;

	lines = (GravLinePtr *)XukcGetListPointer(gravity_line_list, &num_lines);
	for (; num_lines > 0; num_lines--)
		if (lines[num_lines-1]->object == object &&
		    lines[num_lines-1]->line_pos == pos &&
		    lines[num_lines-1]->vertical_line == vertical)
			break;
	if (num_lines > 0)
		(void)XukcRemoveObjectOnList(&gravity_line_list,
						(XtPointer)lines[num_lines-1]);
}


/* ARGSUSED */
static void
redraw_lines(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	if (show_lines)
		show_gravity_lines(w, client_data, (XtPointer)TRUE);
}


static Boolean
get_gravity_settings(object, side, squash, x, y, width, height, border_width)
ObjectPtr object;
Side side; 
Boolean squash;
Position *x, *y;
Dimension *width, *height;
Dimension border_width;
{
#define ABS(x) ((x < 0) ? (-x) : (x))
#define IF_CLOSEST if ((ABS(diff) <= lines[num_lines-1]->gravitational_pull) \
			&& (ABS(diff) < closest)) { \
				closest = ABS(diff); \
				made_change = TRUE;
#define LINE_POS lines[num_lines-1]->line_pos
	GravLinePtr *lines;
	Cardinal num_lines;
	ObjectPtr grav_object;
	Boolean made_change;
	Position new_x, new_y, diff;
	Dimension new_width, new_height, closest;

	if (gravity_line_list == 0 || object == NULL)
		return (FALSE);

	made_change = FALSE;

	new_x = *x; new_y = *y;
	new_width = *width; new_height = *height;

	closest = 100000; /* ||| huge/large number .. never reached! ||| */
	grav_object = object->parent;
	lines = (GravLinePtr *)XukcGetListPointer(gravity_line_list, &num_lines);
	for (; num_lines > 0; num_lines--)
		if (lines[num_lines-1]->object == grav_object) {
			if ((side == left || side == right) &&
			    lines[num_lines-1]->vertical_line) {
				/* check vertical lines */
				if (side == left) {
					diff = *x - LINE_POS;
					IF_CLOSEST
						new_x = LINE_POS;
						if (squash)
							new_width = *width +
									diff;
					}
				} else {
					diff = (*x + *width + border_width*2)
						- LINE_POS;
					IF_CLOSEST
						if (squash)
							new_width = LINE_POS -
							      *x -
							     (border_width*2);
						else
							new_x = LINE_POS -
							      *width -
							      (border_width*2);
					}
				}
			} else if ((side == top || side == bottom) &&
			    !lines[num_lines-1]->vertical_line) {
				/* check horizontal lines */
				if (side == top) {
					diff = *y - LINE_POS;
					IF_CLOSEST
						new_y = LINE_POS;
						if (squash)
							new_height = *height +
									diff;
					}
				} else {
					diff = (*y + *height + border_width*2)
						- LINE_POS;
					IF_CLOSEST
						if (squash)
							new_height = LINE_POS -
							       *y -
							      (border_width*2);
						else
							new_y = LINE_POS -
								*height -
							      (border_width*2);
					}
				}
			}
		}
	if (made_change) {
		*x = new_x;             *y = new_y;
		*width = new_width;     *height = new_height;
	}
	return (made_change);
}


static ObjectPtr
select_object(x, y)
Position *x, *y;
{
	ObjectPtr obj = GetHeadObject();
	Window win;
	Widget w = NULL;

	win = XukcSelectPoint(obj->instance, x, y);

	if (GetStartOfList(TRUE) != NULL)
		w = ObjectWindowToWidget(GetStartOfList(TRUE), win);
	if (w == NULL && GetStartOfList(FALSE) != NULL)
		w = ObjectWindowToWidget(GetStartOfList(FALSE), win);
	if (w != NULL)
		obj = NearestWidgetObject(w);

	return (obj);
}


/* ARGSUSED */
static void
add_grav(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Position x, y;
	Boolean vertical = (Boolean)client_data;
	ObjectPtr obj = select_object(&x, &y);

	if (obj == NULL)
		return;

	add_gravity_line(obj, (Position)(vertical ? x : y), vertical,
							(Position)GRAV_PULL);
	XukcMenuSetToggleState(menu, DISPLAY_GRAVITY, TRUE, TRUE);

	XtRemoveEventHandler(obj->instance, ExposureMask, FALSE, redraw_lines,
							(XtPointer)NULL);
	XtAddEventHandler(obj->instance, ExposureMask, FALSE, redraw_lines,
							(XtPointer)NULL);
}


/* ARGSUSED */
static void
remove_grav(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Position x, y;
	Boolean vertical = (Boolean)client_data;
	ObjectPtr obj = select_object(&x, &y);

	if (obj == NULL)
		return;

	if (show_lines)
		show_gravity_lines(w, client_data, (XtPointer)FALSE);
	remove_gravity_line(obj, (Position)(vertical ? x : y), vertical);

	if (gravity_line_list == 0)
		XukcMenuSetToggleState(menu, DISPLAY_GRAVITY, FALSE, TRUE);
	else if (show_lines)
		show_gravity_lines(w, client_data, (XtPointer)TRUE);
}


/* ARGSUSED */
static void
show_gravity_lines(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Dimension width, height;
	GravLinePtr *lines;
	Cardinal num_lines;
	Arg args[2];
	Boolean turn_off;

	if (gravity_line_list == 0)
		return;

	if (line_on_gc == NULL)
		make_line_gcs(GetHeadObject());

	turn_off = !(Boolean)call_data;

	lines = (GravLinePtr *)XukcGetListPointer(gravity_line_list, &num_lines);
	for (; num_lines > 0; num_lines--) {
		XtSetArg(args[0], XtNwidth, &width);
		XtSetArg(args[1], XtNheight, &height);
		ObjectGetValues(lines[num_lines-1]->object,
						(ArgList)args, 2, FALSE, FALSE);
		XDrawLine(XtDisplayOfObject(lines[num_lines-1]->object->instance),
			  XtWindowOfObject(lines[num_lines-1]->object->instance),
			  turn_off ? line_off_gc : line_on_gc,
			  lines[num_lines-1]->vertical_line ?
				lines[num_lines-1]->line_pos : 0,
			  lines[num_lines-1]->vertical_line ?
				0 : lines[num_lines-1]->line_pos,
			  lines[num_lines-1]->vertical_line ?
				lines[num_lines-1]->line_pos : width,
			  lines[num_lines-1]->vertical_line ?
				height : lines[num_lines-1]->line_pos);
		XFlush(XtDisplayOfObject(lines[num_lines-1]->object->instance));
	}
}


/* ARGSUSED */
static void
tidy_line(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Side side;
	Arg args[5];
	Position px, py, x, y;
	Dimension width, height, bw;
	ObjectPtr obj = select_object(&px, &py);

	if (obj == NULL)
		return;

	XtSetArg(args[0], XtNwidth, &width);
	XtSetArg(args[1], XtNheight, &height);
	XtSetArg(args[2], XtNx, &x);
	XtSetArg(args[3], XtNy, &y);
	XtSetArg(args[4], XtNborderWidth, &bw);
	ObjectGetValues(obj, (ArgList)args, 5, FALSE, FALSE);

	if (px < 0)
		side = left;
	else if (py < 0)
		side = top;
	else if (px >= width)
		side = right;
	else
		side = bottom;

	if (get_gravity_settings(obj, side, (Boolean)client_data,
	    &x, &y, &width, &height, bw)) {
		XtSetArg(args[0], XtNx, x);
		XtSetArg(args[1], XtNy, y);
		XtSetArg(args[2], XtNwidth, width);
		XtSetArg(args[3], XtNheight, height);
		ObjectSetValues(obj, (ArgList)args, 4);
	}
}


/******** Module Exported Routines **********/

void
InitializeGravityLines(mw)
UKCMed3MenuWidget mw;
{
	menu = mw;

	XukcBindButton(menu, TIDY_SHIFT, tidy_line, (XtPointer)FALSE);
	XukcBindButton(menu, TIDY_SHRINK, tidy_line, (XtPointer)TRUE);
	XukcBindButton(menu, ADD_VERT_GRAV, add_grav, (XtPointer)TRUE);
	XukcBindButton(menu, ADD_HORIZ_GRAV, add_grav, (XtPointer)FALSE);
	XukcBindButton(menu, REMOVE_VERT_GRAV, remove_grav, (XtPointer)TRUE);
	XukcBindButton(menu, REMOVE_HORIZ_GRAV, remove_grav, (XtPointer)FALSE);
	XukcBindToggle(menu, DISPLAY_GRAVITY, "Gravity Off", "Gravity On",
					(Boolean *)&show_lines,
					tidy_function, (XtPointer)NULL);
	XukcTurnOffButton(menu, TIDY);
}
