#ifndef lint
static char sccsid[] = "@(#)selected.c	1.3 (UKC) 5/17/92";
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

#define DO_REPARENTING 1
#undef DO_REPARENTING

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Grip.h>

#ifdef UKC_WIDGETS
#	include <X11/Xukc/Layout.h>
#endif /* UKC_WIDGETS */
#include "dirt_structures.h"

#define GRIP_SIZE 10

#define TOP_LEFT 0
#define TOP_MIDDLE 1
#define TOP_RIGHT 2
#define MIDDLE_LEFT 3
#define CENTRE 4
#define MIDDLE_RIGHT 5
#define BOTTOM_LEFT 6
#define BOTTOM_MIDDLE 7
#define BOTTOM_RIGHT 8

#define START "press"
#define MOVE "move"
#define STOP "release"
#define SNAP_STRETCH "snap&stretch2it"
#define SNAP_MOVE "snap2it"

#define NONE 0
#define STARTED 1
#define MOVING 2

#define FRAME_WIDTH 1

#define PICKED XtParent(w)
#define SWAP(a, b) { Position c; c = a; a = b; b = c; }

extern void EditAnInstance();
extern void EditComments();
extern void ChangeMark();
extern Window XukcSelectWindow();
extern Widget ObjectWindowToWidget();
extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern ObjectPtr SearchRecords();
extern ObjectPtr CurrentlyEditing();
extern Boolean NoObjectsCreated();
extern void DrainUIEventQueue();
/* in edit.c */
extern void AllowAnyGeometryChange();
extern void CallParentLayoutProc();


extern struct app_resources editor_resources;

void SnapToGrid();
ObjectPtr NearestWidgetObject();

static Widget top_left_grip = NULL, top_middle_grip, top_right_grip;
static Widget middle_left_grip, middle_grip, middle_right_grip;
static Widget bottom_left_grip, bottom_middle_grip, bottom_right_grip;
static XtTranslations grip_trans = NULL;
static String trans_string = "<Btn1Down>: GripAction(press) \n\
			      Button1<MotionNotify>: GripAction(move) \n\
			      <Btn1Up>: GripAction(release) \n\
			      <Btn2Down>: GripAction(snap&stretch2it) \n\
			      <Btn3Down>: GripAction(snap2it)";
static XSegment lines[8]; 
static ObjectPtr old_parent = NULL;
static Pixmap old_background = NULL;

static void
freeGC(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	GC *gc = (GC *)client_data;

	XtReleaseGC(w, *gc);
	*gc = NULL;
}


static Pixel
grid_colour(w)
Widget w;
{
	Pixel fg = BlackPixelOfScreen(XtScreen(w));
	XrmValue from, to;

	from.addr = (XtPointer)editor_resources.grid_colour;
	from.size = strlen((char *)editor_resources.grid_colour) + 1;
	to.addr = (XtPointer)&fg;
	to.size = sizeof(Pixel);

	(void)XtConvertAndStore(w, XtRString, &from, XtRPixel, &to);
	return (fg);
}


static void
grid_background(w, x_grid, y_grid, fg)
Widget w;
Dimension x_grid, y_grid;
Pixel fg;
{
	char *bitmap;
	static Pixmap pix = NULL;
	static Dimension x = 0, y = 0;
	Cardinal size;
	unsigned int depth;
	Pixel bg;

	if (pix == NULL || x_grid != x || y_grid != y) {
		if (pix != NULL)
			XFreePixmap(XtDisplay(w), pix);

		XtVaGetValues(w, XtNdepth, &depth, XtNbackground, &bg, NULL);

		size = ((x_grid + 7) / 8) * y_grid;
		bitmap = (char *)XtMalloc(size);
		bzero((char *)bitmap, size);
		bitmap[0] = 1;
		pix = XCreatePixmapFromBitmapData(XtDisplay(w), XtWindow(w),
					bitmap, x_grid, y_grid, fg, bg, depth);
	}

	XtVaSetValues(w, XtNbackgroundPixmap, pix, NULL);
}


static GC
invertGC(w)
Widget w;
{
	static GC invert_gc = NULL;
	XGCValues gcvalues;

	if (invert_gc == NULL) {
		/* make the GC that Xors the lines of a frame */
		gcvalues.function = GXinvert;
		gcvalues.line_width = FRAME_WIDTH == 1 ? 0 : FRAME_WIDTH;
		gcvalues.subwindow_mode = IncludeInferiors;
		invert_gc = XtGetGC(w, GCFunction | GCLineWidth |
						GCSubwindowMode, &gcvalues);
		XtAddCallback(w, XtNdestroyCallback, freeGC,
						(XtPointer)&invert_gc);
	}
	return (invert_gc);
}


static void
move_target(parent, target, x, y, width, height)
Widget parent, target;
Position x, y;
Dimension width, height;
{
	XtWidgetGeometry request, reply;
	XtGeometryResult result;

	/* update the target, if the geometry is allowed the change
	 * will be made automatically. */
	request.request_mode = CWX | CWY | CWWidth | CWHeight |
				XtCWQueryOnly;
	request.x = x;
	request.y = y;
	request.width = width;
	request.height = height;
	reply.request_mode = NULL;

	result = XtMakeGeometryRequest(target, &request, &reply);

	if (result == XtGeometryAlmost) {
		if (reply.request_mode & CWX)
			x = reply.x;
		if (reply.request_mode & CWY)
			y = reply.y;
		if (reply.request_mode & CWWidth)
			width = reply.width;
		if (reply.request_mode & CWHeight)
			height = reply.height;
	} else if (result == XtGeometryNo)
		/* override the geometry manager */
		AllowAnyGeometryChange(CurrentlyEditing(), TRUE);

	XtVaSetValues(target, XtNx, x, XtNy, y, XtNwidth, width,
			XtNheight, height, NULL);

	if (result == XtGeometryNo) {
		/* turn the geometry manager back on but let it re-layout
		 * its children.
		 */
		AllowAnyGeometryChange(CurrentlyEditing(), FALSE);
		/* relayout this child without unmanaging/managing it */
		CallParentLayoutProc(XtParent(target));
	}
	DrainUIEventQueue();
}


static void
calculate_end_points(w, target, ev, ax, ay, bx, by)
Widget w, target;
XEvent *ev;
Position *ax, *ay, *bx, *by;
{
	Dimension width, height, bw;
	Position x, y, px, py, rx, ry;

#ifdef DO_REPARENTING
	XtVaGetValues(w, XtNx, &x, XtNy, &y, NULL);

	if (ev->type == MotionNotify) {
		x = ev->xmotion.x - x;
		y = ev->xmotion.y - y;
	} else {
		x = ev->xbutton.x - x;
		y = ev->xbutton.y - y;
	}
#else
	XtTranslateCoords(target, 0, 0, &x, &y);

	if (ev->type == MotionNotify) {
		x = ev->xmotion.x_root - x;
		y = ev->xmotion.y_root - y;
	} else {
		x = ev->xbutton.x_root - x;
		y = ev->xbutton.y_root - y;
	}
#endif /* DO_REPARENTING */

	XtVaGetValues(target, XtNx, &px, XtNy, &py,
			XtNwidth, &width, XtNheight, &height,
			XtNborderWidth, &bw, NULL);
	width += bw + bw - FRAME_WIDTH;
	height += bw + bw - FRAME_WIDTH;

	rx = px + x;
	ry = py + y;
	SnapToGrid(&rx, &ry);
	x = rx - px;
	y = ry - py;

	if (w == top_left_grip) {
		*ax = x - bw; *ay = y - bw;
		*bx = width - bw; *by = height - bw;
	} else if (w == top_middle_grip) {
		*ax = -bw; *ay = y - bw;
		*bx = width - bw; *by = height - bw;
	} else if (w == top_right_grip) {
		*ax = x + bw; *ay = y - bw;
		*bx = -bw; *by = height - bw;
	} else if (w == middle_left_grip) {
		*ax = x - bw; *ay = -bw;
		*bx = width - bw; *by = height - bw;
	} else if (w == middle_grip) {
		*ax = x - width/2; *ay = y - height/2;
		*bx = *ax + width; *by = *ay + height;
	} else if (w == middle_right_grip) {
		*ax = x + bw; *ay = -bw;
		*bx = -bw; *by = height - bw;
	} else if (w == bottom_left_grip) {
		*ax = x - bw; *ay = y + bw;
		*bx = width - bw; *by = -bw;
	} else if (w == bottom_middle_grip) {
		*ax = -bw; *ay = y + bw;
		*bx = width - bw; *by = -bw;
	} else if (w == bottom_right_grip) {
		*ax = x + bw; *ay = y + bw;
		*bx = -bw; *by = -bw;
	} 

	if (*ax > *bx)
		SWAP(*ax, *bx);
	if (*ay > *by)
		SWAP(*ay, *by);
}


/* creates a wire frame between two extremes.  Extremes must be opposite 
 * corners in a rectangle.  Updates the lines[] array. */
static void
make_frame(w, ax, ay, bx, by)
Widget w;
Position ax, ay, bx, by;
{
	Position x1, y1, x2, y2;

	/* we are drawing into the root window so translate the coords */
	XtTranslateCoords(w, ax, ay, &x1, &y1);
	XtTranslateCoords(w, bx, by, &x2, &y2);

	/* x1, y1 to x2, y2 defines a box (0,0)-(a,b) */
	lines[0].x1 = x1; lines[0].y1 = y1;  /* (0,0)-(a,0) */
	lines[0].x2 = x2; lines[0].y2 = lines[0].y1;
	lines[1].x1 = x2; lines[1].y1 = y1;  /* (a,0)-(a,b) */
	lines[1].x2 = x2; lines[1].y2 = y2;
	lines[2].x1 = x2; lines[2].y1 = y2;  /* (a,b)-(0,b) */
	lines[2].x2 = x1; lines[2].y2 = lines[2].y1;
	lines[3].x1 = x1; lines[3].y1 = y2;  /* (0,b)-(0,0) */
	lines[3].x2 = x1; lines[3].y2 = y1;
	lines[4].x1 = x1; lines[4].y1 = (y2-y1)/3+y1;  /* (0,b/3)-(a,b/3) */
	lines[4].x2 = x2; lines[4].y2 = lines[4].y1;
	lines[5].x1 = x1; lines[5].y1 = (y2-y1)*2/3+y1;/* (0,b*2/3)-(a,b*2/3) */
	lines[5].x2 = x2; lines[5].y2 = lines[5].y1;
	lines[6].x1 = (x2-x1)/3+x1; lines[6].y1 = y1;  /* (a/3,0)-(a/3,b) */
	lines[6].x2 = lines[6].x1; lines[6].y2 = y2;
	lines[7].x1 = (x2-x1)*2/3+x1; lines[7].y1 = y1;/* (a*2/3,0)-(a*2/3,b) */
	lines[7].x2 = lines[7].x1; lines[7].y2 = y2;
}


static void
draw_grid(w)
Widget w;
{
	if (editor_resources.x_grid == 0 || editor_resources.y_grid == 0
	    || editor_resources.grid_off)
		return;

	grid_background(w, (Dimension)editor_resources.x_grid,
			(Dimension)editor_resources.y_grid,
			grid_colour(w));
}


static void
draw_frame(w)
Widget w;
{
	XDrawSegments(XtDisplayOfObject(w), XtScreenOfObject(w)->root,
					invertGC(w), lines, 8);
	XFlush(XtDisplayOfObject(w));
}


static void
snap_to_nearest_grid_point(target, grip, stretch_or_shrink)
Widget target, grip;
Boolean stretch_or_shrink;
{
	Dimension width, height, bw;
	Position x, y, new_x, new_y;
	
	XtVaGetValues(target, XtNwidth, &width, XtNheight, &height,
			XtNx, &x, XtNy, &y, XtNborderWidth, &bw, NULL);

	if (grip == top_left_grip) {
		new_x = x;
		new_y = y;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink) {
			width += (x - new_x);
			height += (y - new_y);
		}
		x = new_x;
		y = new_y;
	} else if (grip == top_middle_grip) {
		new_x = x;
		new_y = y;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink)
			height += (y - new_y);
		y = new_y;
	} else if (grip == top_right_grip) {
		new_x = x + width + bw;
		new_y = y;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink) {
			width = (new_x - x - bw);
			height += (y - new_y);
		}
		y = new_y;
	} else if (grip == middle_left_grip) {
		new_x = x;
		new_y = y;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink)
			width += (x - new_x);
		x = new_x;
	} else if (grip == middle_grip) {
		if (stretch_or_shrink) {
			new_x = x + width + bw;
			new_y = y + height + bw;
			SnapToGrid(&new_x, &new_y);
			SnapToGrid(&x, &y);
			width = new_x - x - bw;
			height = new_y - y - bw;
		} else {
			new_x = x +
				(Position)(((double)bw + bw + width) / 2 + 0.5);
			new_y = y +
			       (Position)(((double)bw + bw + height) / 2 + 0.5);
			SnapToGrid(&new_x, &new_y);
			x = new_x -
				(Position)(((double)bw + bw + width) / 2 + 0.5);
			y = new_y -
			       (Position)(((double)bw + bw + height) / 2 + 0.5);
		}
	} else if (grip == middle_right_grip) {
		new_x = x + width + bw;
		new_y = y;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink)
			width = (new_x - x - bw);
	} else if (grip == bottom_left_grip) {
		new_x = x;
		new_y = y + height + bw;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink)
			width += (x - new_x);
		x = new_x;
		height = (new_y - y - bw);
	} else if (grip == bottom_middle_grip) {
		new_x = x;
		new_y = y + height + bw;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink)
			height = (new_y - y - bw);
	} else if (grip == bottom_right_grip) {
		new_x = x + width + bw;
		new_y = y + height + bw;
		SnapToGrid(&new_x, &new_y);
		if (stretch_or_shrink) {
			width = (new_x - x - bw);
			height = (new_y - y - bw);
		}
	} 

	move_target(XtParent(target), target, x, y, width, height);
}


/* drag resize a corner or side grip.  uses a skeleton frame to show where
 * the new corner will be and then resizes and moves the target widget.
 * This routine is called by the GripAction callback and can therefore
 * have 3 states, "just picked up (press)", "dragged (move)" and "just let go
 * (release)", which starts, moves and removes the dragged lines.  The
 * widget is resized and moved when the "release" is set.
 */
static void
resize_target(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	static Cardinal state = NONE;
	static Position ax = 0, ay = 0, bx = 0, by = 0;
	GripCallData ptr = (GripCallData)call_data;
	Position tx, ty;
	Dimension width, height, bw;

	if (ptr->num_params != 1)
		XtError("resize_target() called with incorrect parameter count");

	if (strcmp(ptr->params[0], START) == 0 && state == NONE) {
		/* start tracking the cursor and drawing the frame */
		XGrabServer(XtDisplayOfObject(w));
		calculate_end_points(w, PICKED, ptr->event, &ax, &ay, &bx, &by);
		make_frame(PICKED, ax, ay, bx, by);
		draw_frame(w);
		state = STARTED;
	} else if (strcmp(ptr->params[0], MOVE) == 0 && state != NONE) {
		/* move the frame to the new tracked cursor position */
		Position old_ax, old_ay, old_bx, old_by;

		old_ax = ax;
		old_ay = ay;
		old_bx = bx;
		old_by = by;
		calculate_end_points(w, PICKED, ptr->event, &ax, &ay, &bx, &by);

		/* optimize the redraw of the frame */
		if (old_ax != ax || old_ay != ay
		    || old_bx != bx || old_by != by) {
			draw_frame(w);
			make_frame(PICKED, ax, ay, bx, by);
			draw_frame(w);
		}
		state = MOVING;
	} else if (strcmp(ptr->params[0], STOP) == 0) {
		/* finished resizing or moving so update the target widget */
		if (state == MOVING) {
			calculate_end_points(w, PICKED, ptr->event, &ax, &ay,
								&bx, &by);
			draw_frame(w);
			make_frame(PICKED, ax, ay, bx, by);
			draw_frame(w);
		}
		draw_frame(w);
		XUngrabServer(XtDisplayOfObject(w));
		XFlush(XtDisplayOfObject(w));

		if (state == STARTED)
			return;

		state = NONE;   

		XtVaGetValues(PICKED, XtNx, &tx, XtNy, &ty,
				XtNborderWidth, &bw, NULL);
		width = (Dimension)(bx - ax + FRAME_WIDTH - bw - bw);
		height = (Dimension)(by - ay + FRAME_WIDTH - bw - bw);
		ax += tx + bw;
		ay += ty + bw;
		move_target(XtParent(PICKED), PICKED, ax, ay, width, height);
	} else if (strcmp(ptr->params[0], SNAP_STRETCH) == 0) {
		if (!editor_resources.no_snapping &&
		    editor_resources.x_grid > 0 &&
		    editor_resources.y_grid > 0)
			snap_to_nearest_grid_point(PICKED, w, TRUE);
	} else if (strcmp(ptr->params[0], SNAP_MOVE) == 0) {
		if (!editor_resources.no_snapping &&
		    editor_resources.x_grid > 0 &&
		    editor_resources.y_grid > 0)
			snap_to_nearest_grid_point(PICKED, w, FALSE);
	}
}


static void
grip_occluded(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	if (XtIsShell(w))
		return;
	XRaiseWindow(XtDisplayOfObject(w), XtWindowOfObject(w));
}


/* when the currently selected window is resized move the grips around */
static void
grip_parent_resized(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	Dimension width, height;
	Arg args[2];
	Cardinal i = 0;

	XtSetArg(args[0], XtNwidth, &width);
	XtSetArg(args[1], XtNheight, &height);
	XtGetValues(w, (ArgList)args, 2);

	i = 0;
	XtSetArg(args[i], XtNx, width/2 - GRIP_SIZE/2); i++;
	XtSetValues(top_middle_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNx, width - GRIP_SIZE); i++;
	XtSetValues(top_right_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNy, height/2 - GRIP_SIZE/2); i++;
	XtSetValues(middle_left_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNx, width/2 - GRIP_SIZE/2); i++;
	XtSetArg(args[i], XtNy, height/2 - GRIP_SIZE/2); i++;
	XtSetValues(middle_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNx, width - GRIP_SIZE); i++;
	XtSetArg(args[i], XtNy, height/2 - GRIP_SIZE/2); i++;
	XtSetValues(middle_right_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNy, height - GRIP_SIZE); i++;
	XtSetValues(bottom_left_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNx, width/2 - GRIP_SIZE/2); i++;
	XtSetArg(args[i], XtNy, height - GRIP_SIZE); i++;
	XtSetValues(bottom_middle_grip, (ArgList)args, i);

	i = 0;
	XtSetArg(args[i], XtNx, width - GRIP_SIZE); i++;
	XtSetArg(args[i], XtNy, height - GRIP_SIZE); i++;
	XtSetValues(bottom_right_grip, (ArgList)args, i);

	if (!XtIsShell(w))
		XRaiseWindow(XtDisplayOfObject(w), XtWindowOfObject(w));
	XFlush(XtDisplayOfObject(w));
}


static void
flush_dpy(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XFlush((Display *)client_data);
}


static void
destroy_selection_grips()
{
	Display *dpy;

	if (top_left_grip == NULL) /* safety net */
		return;

	dpy = XtDisplay(top_left_grip);
	XtRemoveEventHandler(XtParent(top_left_grip), StructureNotifyMask,
				FALSE, grip_parent_resized, (XtPointer)NULL);
	XtDestroyWidget(top_left_grip);
	XtDestroyWidget(top_middle_grip);
	XtDestroyWidget(top_right_grip);
	XtDestroyWidget(middle_left_grip);
	XtDestroyWidget(middle_grip);
	XtDestroyWidget(middle_right_grip);
	XtDestroyWidget(bottom_left_grip);
	XtDestroyWidget(bottom_middle_grip);
	XtAddCallback(bottom_right_grip, XtNdestroyCallback, flush_dpy,
					(XtPointer)dpy);
	XtDestroyWidget(bottom_right_grip);
	top_left_grip = NULL;
}


static void
create_selection_grips(target)
Widget target;
{
#ifdef DO_REPARENTING
#define REPARENT_GRIP(w) { \
	Position rx, ry; \
	XSetWindowAttributes wa; \
	Window junk; \
	XtVaGetValues(w, XtNx, &rx, XtNy, &ry, NULL); \
	rx += x;   ry += y; \
	wa.override_redirect = TRUE; \
	XChangeWindowAttributes(XtDisplayOfObject(w), XtWindowOfObject(w), \
				CWOverrideRedirect, &wa); \
	XReparentWindow(XtDisplayOfObject(w), XtWindowOfObject(w), \
			XtScreenOfObject(w)->root, \
			(int)rx, (int)ry); \
	XtAddEventHandler(w, VisibilityChangeMask, FALSE, \
					grip_occluded, (XtPointer)NULL); \
	XtVaSetValues(w, XtNx, rx, XtNy, ry, NULL); \
}
#endif /* DO_REPARENTING */
	Dimension width, height;
	Position x, y;
	Pixel bg = grid_colour(target);

	if (!XtIsShell(target))
	      XRaiseWindow(XtDisplayOfObject(target), XtWindowOfObject(target));
	XtVaGetValues(target, XtNwidth, &width, XtNheight, &height,
				XtNx, &x, XtNy, &y, NULL);
#ifdef DO_REPARENTING
	XtTranslateCoords(target, x, y, &x, &y, NULL);
#endif /* DO_REPARENTING */

	top_left_grip = XtVaCreateWidget("_dirtTopLeftGrip", gripWidgetClass,
				target, XtNtranslations, grip_trans,
				XtNforeground, bg,
				NULL);
	XtAddCallback(top_left_grip, XtNcallback, resize_target,
						(XtPointer)TOP_LEFT);
	XtRealizeWidget(top_left_grip);

	top_middle_grip = XtVaCreateWidget("_dirtTopMiddleGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNx, width/2 - GRIP_SIZE/2,
				XtNforeground, bg,
				NULL);
	XtAddCallback(top_middle_grip, XtNcallback, resize_target,
						(XtPointer)TOP_MIDDLE);
	XtRealizeWidget(top_middle_grip);

	top_right_grip = XtVaCreateWidget("_dirtTopRightGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNx, width - GRIP_SIZE,
				XtNforeground, bg,
				NULL);
	XtAddCallback(top_right_grip, XtNcallback, resize_target,
						(XtPointer)TOP_RIGHT);
	XtRealizeWidget(top_right_grip);

	middle_left_grip = XtVaCreateWidget("_dirtMiddleLeftGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNy, height/2 - GRIP_SIZE/2,
				XtNforeground, bg,
				NULL);
	XtAddCallback(middle_left_grip, XtNcallback, resize_target,
						(XtPointer)MIDDLE_LEFT);
	XtRealizeWidget(middle_left_grip);

	middle_grip = XtVaCreateWidget("_dirtMiddleGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNx, width/2 - GRIP_SIZE/2,
				XtNy, height/2 - GRIP_SIZE/2,
				XtNforeground, bg,
				NULL);
	XtAddCallback(middle_grip, XtNcallback, resize_target, (XtPointer)NULL);
	XtRealizeWidget(middle_grip);

	middle_right_grip = XtVaCreateWidget("_dirtMiddleRightGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNx, width - GRIP_SIZE,
				XtNy, height/2 - GRIP_SIZE/2,
				XtNforeground, bg,
				NULL);
	XtAddCallback(middle_right_grip, XtNcallback, resize_target,
						(XtPointer)MIDDLE_RIGHT);
	XtRealizeWidget(middle_right_grip);

	bottom_left_grip = XtVaCreateWidget("_dirtBottomLeftGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNy, height - GRIP_SIZE,
				XtNforeground, bg,
				NULL);
	XtAddCallback(bottom_left_grip, XtNcallback, resize_target,
						(XtPointer)BOTTOM_LEFT);
	XtRealizeWidget(bottom_left_grip);

	bottom_middle_grip = XtVaCreateWidget("_dirtBottomMiddleGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNx, width/2 - GRIP_SIZE/2,
				XtNy, height - GRIP_SIZE,
				XtNforeground, bg,
				NULL);
	XtAddCallback(bottom_middle_grip, XtNcallback, resize_target,
						(XtPointer)BOTTOM_MIDDLE);
	XtRealizeWidget(bottom_middle_grip);

	bottom_right_grip = XtVaCreateWidget("_dirtBottomRightGrip",
				gripWidgetClass, target,
				XtNtranslations, grip_trans,
				XtNx, width - GRIP_SIZE,
				XtNy, height - GRIP_SIZE,
				XtNforeground, bg,
				NULL);
	XtAddCallback(bottom_right_grip, XtNcallback, resize_target,
						(XtPointer)BOTTOM_RIGHT);
	XtRealizeWidget(bottom_right_grip);

#ifdef DO_REPARENTING
	REPARENT_GRIP(top_left_grip);
	REPARENT_GRIP(top_middle_grip);
	REPARENT_GRIP(top_right_grip);
	REPARENT_GRIP(middle_left_grip);
	REPARENT_GRIP(middle_grip);
	REPARENT_GRIP(middle_right_grip);
	REPARENT_GRIP(bottom_left_grip);
	REPARENT_GRIP(bottom_middle_grip);
	REPARENT_GRIP(bottom_right_grip);
#endif /* DO_REPARENTING */

	XtMapWidget(top_left_grip);
	XtMapWidget(top_middle_grip);
	XtMapWidget(top_right_grip);
	XtMapWidget(middle_left_grip);
	XtMapWidget(middle_grip);
	XtMapWidget(middle_right_grip);
	XtMapWidget(bottom_left_grip);
	XtMapWidget(bottom_middle_grip);
	XtMapWidget(bottom_right_grip);
	XFlush(XtDisplayOfObject(top_left_grip));

	XtAddEventHandler(XtParent(top_left_grip), StructureNotifyMask, FALSE,
				grip_parent_resized, (XtPointer)NULL);
}


static void
forget_old_parent(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	old_parent = NULL;
}


static void
clear_grid()
{
	XtVaSetValues(old_parent->instance, XtNbackgroundPixmap,
				old_background, NULL);
	old_parent = NULL;
}


/***** Module Exported Routines *****/


void
DrawGrid(parent)
ObjectPtr parent;
{
	if (parent == old_parent)
		return;

	if (old_parent != NULL)
		clear_grid();

	if (parent != NULL && parent->instance != NULL && !parent->is_remote) {
		if ((parent = NearestWidgetObject(parent->instance)) != NULL) {
			old_parent = parent;
			XtVaGetValues(parent->instance, XtNbackgroundPixmap,
					&old_background, NULL);
			draw_grid(parent->instance);
			XtAddCallback(parent->instance, XtNdestroyCallback,
					forget_old_parent, (XtPointer)NULL);
		}
	}
}


void
ChangeSelection(new_object)
ObjectPtr new_object;
{
	static ObjectPtr old_picked = NULL;

	if (old_picked != NULL)
		destroy_selection_grips();

	old_picked = NULL;
	DrawGrid(new_object == NULL ? NULL : new_object->parent);
	ChangeMark(new_object);
	EditAnInstance(new_object, FALSE);
	EditComments(new_object);

	if (new_object == NULL || new_object->instance == NULL
	    || !new_object->managed || new_object->is_remote)
		return;  /* safety net */

	if (grip_trans == NULL)
		grip_trans = XtParseTranslationTable(trans_string);

	if (XtIsWidget(new_object->instance))
		create_selection_grips(new_object->instance);
	old_picked = new_object;
}


ObjectPtr
NearestWidgetObject(w)
Widget w;
{
	ObjectPtr ret_obj;
	
	if (w == NULL)
		return (NULL);

	if (GetStartOfList(FALSE) != NULL &&
	    (ret_obj = SearchRecords(GetStartOfList(FALSE), w))
	    != NULL)
		return (ret_obj);

	ret_obj = SearchRecords(GetStartOfList(TRUE), w);
	if (ret_obj != NULL && !ret_obj->is_resource)
		return (ret_obj);
	else
		return (NearestWidgetObject(XtParent(w)));
}


void
GridObjectHasDied(obj)
ObjectPtr obj;
{
	if (obj == old_parent)
		clear_grid();
}


void
PickedSelection(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Widget picked = NULL;
	Window win;
	ObjectPtr selected;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "pickedSelection",
				"No Objects To Pick", 0);
		return;
	}

	win = XukcSelectWindow(GetHeadObject()->instance);

	selected = GetStartOfList(TRUE);
	if (selected != NULL)
		picked = ObjectWindowToWidget(selected, win);
	if (picked == NULL && (selected = GetStartOfList(FALSE)) != NULL)
		picked = ObjectWindowToWidget(selected, win);

	if (picked != NULL) {
		selected = NearestWidgetObject(picked);
		if (selected != NULL)
			ChangeSelection(selected);
	}
}


/* perform snapping of points to the grid */
void
SnapToGrid(x, y)
Position *x, *y;
{
	if (editor_resources.no_snapping ||
	    editor_resources.x_grid == 0 || editor_resources.y_grid == 0)
		return;

	*x = (Position)(((double)*x / (double)editor_resources.x_grid) + 0.5);
	*x = *x * editor_resources.x_grid;
	*y = (Position)(((double)*y / (double)editor_resources.y_grid) + 0.5);
	*y = *y * editor_resources.y_grid;
}
