#ifndef lint
/* From:   "$Xukc: window.c,v 1.8 91/12/17 10:02:03 rlh2 Rel $"; */
static char sccsid[] = "@(#)window.c	1.2 (UKC) 5/10/92";
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

/* Various Xlib based routines that grab the pointer and allows boxes
 * or lines etc to be dragged out interactively.
 *
 * Plus routines to query the window tree and find specific windows.
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xproto.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#ifdef UKC_WIDGETS
#include <X11/Xukc/Label.h>
#else
#	ifdef XAW
#		include <X11/Xaw/Label.h>
#	endif /* XAW */
#endif /* UKC_WIDGETS */

static int (*old_error_handler)() = NULL;

/*
 *change a Label Widget's label without waiting for the exposure event.
 */  
void
XukcChangeLabel(w, new_str, force_display)
Widget w;
String new_str;
Boolean force_display;
{
	XtVaSetValues(w, XtNlabel, new_str, NULL);

	if (force_display) {	/* fake an exposure to force a redisplay */
		XEvent ev;
		XExposeEvent *eev = (XExposeEvent *)&ev;
		Dimension width, height;

		XtVaGetValues(w, XtNwidth, &width, XtNheight, &height, NULL);

		eev->type = Expose;
		eev->send_event = FALSE;
		eev->display = XtDisplay(w);
		eev->window = XtWindow(w);
		eev->x = eev->y = 0;
		eev->width = width;
		eev->height = height;   /* redraw the whole label! */
		eev->count = 0;

		XtDispatchEvent(&ev);
		XFlush(XtDisplayOfObject(w));
	}
}


Boolean
XukcWaitForWidgetEvent(widget, eventmask, type)
Widget widget;
long eventmask;
int type;
{
	XEvent ev;
	long oldmask;
	XWindowAttributes atts;

	if (!XGetWindowAttributes(XtDisplay(widget), XtWindow(widget), &atts))
		return (FALSE); /* Help! .. failed X request */

	oldmask = atts.your_event_mask;
	XSelectInput(XtDisplay(widget), XtWindow(widget), oldmask | eventmask);
	XSync(XtDisplay(widget), FALSE);
	while (1) {
		if (XtAppPeekEvent(XtWidgetToApplicationContext(widget), &ev)
		    && (ev.type == type)
		    && (ev.xany.window == XtWindow(widget)))
			break;
		XtAppNextEvent(XtWidgetToApplicationContext(widget), &ev);
		XtDispatchEvent(&ev);
	}
	if (!(oldmask & eventmask))
		/* eat up this event if the widget didn't select for it */
		XtAppNextEvent(XtWidgetToApplicationContext(widget), &ev);
	XSelectInput(XtDisplay(widget), XtWindow(widget), oldmask);
	return (TRUE);
}


/*
 * find child-window of target_win that the point (x,y) is within
 */
Window
XukcEnclosingWindow(w, target_win, x, y)
Widget w;	/* used for display and root window only */
Window target_win;
Position x, y;
{
	Window root_win = XtScreen(w)->root;
	Window child_win = root_win;
	int tx, ty, tx_ret, ty_ret;

	tx = tx_ret = x;
	ty = ty_ret = y;

	/* home in on the real window that we are trying to choose */
	while (child_win != None && XTranslateCoordinates(XtDisplay(w),
		root_win, target_win, tx, ty, &tx_ret, &ty_ret, &child_win))
			if (child_win != None) {
				root_win = target_win;
				target_win = child_win;
				tx = tx_ret;
				ty = ty_ret;
			};
	return (target_win);
}


/*
 * Routine to let user select a point and/or a window using the mouse
 */

Window
XukcSelectPoint(w, x, y)
Widget w;
Position *x, *y;
{
	int status;
	Cursor cursor;
	XEvent event;
	Window target_win = None;
	Window root_win;
	int buttons = 0, ix, iy;

	root_win = XtScreen(w)->root;

	/* Make the target cursor */
	cursor = XCreateFontCursor(XtDisplay(w), XC_crosshair);

	/* Grab the pointer using target cursor, letting it roam all over */
	status = XGrabPointer(XtDisplay(w), root_win, False,
			ButtonPressMask|ButtonReleaseMask, GrabModeSync,
			GrabModeAsync, None, cursor, CurrentTime);

	if (status != GrabSuccess) {
		XtWarning("Can't Grab the mouse in XukcSelectWindow()");
		return None;
	}

	/* Let the user select a window... */
	while ((target_win == None) || (buttons != 0)) {
		/* allow one more event */
		XAllowEvents(XtDisplay(w), SyncPointer, CurrentTime);
		XMaskEvent(XtDisplay(w), ButtonPressMask|ButtonReleaseMask, &event);

		switch (event.type) {
	 	   case ButtonPress:
			if (target_win == None) {
				/* window selected */
				target_win = event.xbutton.subwindow;
				if (target_win == None)
					target_win = root_win;
			}
			buttons++;
			break;
		   case ButtonRelease:
			/* there may have been some down before we started */
			if (buttons > 0)
				buttons--;
			break;
		}
	} 

	XUngrabPointer(XtDisplay(w), CurrentTime);      /* Done with pointer */

	target_win = XukcEnclosingWindow(w, target_win, event.xbutton.x,
							event.xbutton.y);
	if (target_win != NULL) {
		XTranslateCoordinates(XtDisplay(w), root_win, target_win,
				event.xbutton.x, event.xbutton.y, &ix, &iy,
				&root_win);
		*x = (Position)ix;
		*y = (Position)iy;
	}
	return (target_win);
}


Window
XukcSelectWindow(w)
Widget w;
{
	Position x, y;

	return (XukcSelectPoint(w, &x, &y));
}


static int
error_handler(dpy, errorp)
Display *dpy;
XErrorEvent *errorp;
{
	switch (errorp->request_code) {
		case X_QueryTree:
		case X_GetGeometry:
			if (errorp->error_code == BadWindow)
				break;
		default:
			return ((*old_error_handler)(dpy, errorp));
	}
	return 0;
}


static Window
find_virtual_root(dpy)
Display *dpy;
{
	Atom __SWM_VROOT = None;
	Window root, rootReturn, parentReturn, *children;
	unsigned int numChildren;
	Cardinal i;

	root = DefaultRootWindow(dpy);

	/* go look for a virtual root */
	__SWM_VROOT = XInternAtom(dpy, "__SWM_VROOT", False);

	if (__SWM_VROOT == None)
		return (root);

	XQueryTree(dpy, root, &rootReturn, &parentReturn, &children,
			&numChildren);
	for (i = 0; i < numChildren; i++) {
		Atom actual_type;
		int actual_format;
		unsigned long nitems, bytesafter;
		Window *newRoot = NULL;
 
		if (XGetWindowProperty(dpy, children[i], __SWM_VROOT, 0L, 1L,
		    False, XA_WINDOW, &actual_type, &actual_format, &nitems,
		    &bytesafter, (unsigned char **) &newRoot) == Success
		    && newRoot) {
			root = *newRoot;
			break;
		}
	}
	return (root);
}


Window
XukcFindUndecoratedWindow(dpy, window, x, y, width, height, border)
Display *dpy;
Window window;
Position *x, *y;
Dimension *width, *height, *border;
{
	Status status;
	Window root, parent, *children;
	unsigned int num_children;
	int wx, wy;
	unsigned int wh, ww, bw, depth;

	old_error_handler = XSetErrorHandler(error_handler);
	do {
		status = XQueryTree(dpy, window, &root, &parent, &children,
							&num_children);
		XFree((char *)children);
	} while (status && parent != root && (window = parent));
	/* NB: the assignment is correct ......^^^^^^^^ */

	if (status) {
		status = XGetGeometry(dpy, window, &root, &wx, &wy, &ww, &wh,
					&bw, &depth);
		if (status) {
			*x = (Position)wx;  *y = (Position)wy;
			*width = (Dimension)ww;  *height = (Dimension)wh;
			*border = (Dimension)bw;
			(void)XSetErrorHandler(old_error_handler);
			return (window);
		}
	}
	(void)XSetErrorHandler(old_error_handler);
	return (NULL);
}


/* drag out a rectangle for selections etc */

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void
DrawBox(dpy, win, gc, x1, y1, x2, y2)
Display *dpy;
Window win;
GC gc;
int x1, y1, x2, y2;
{
	XPoint lines[5];

	lines[0].x = x1;
	lines[0].y = y1;
	lines[1].x = x2;
	lines[1].y = y1;
	lines[2].x = x2;
	lines[2].y = y2;
	lines[3].x = x1;
	lines[3].y = y2;
	lines[4].x = x1;
	lines[4].y = y1;
	XDrawLines(dpy, win, gc, lines, 5, CoordModeOrigin);
}


void
XukcDragBox(w, ret_x1, ret_y1, ret_x2, ret_y2, update_function)
Widget w;
int *ret_x1, *ret_y1, *ret_x2, *ret_y2;
void (*update_function)();
{
#define DELAY_TIME 500
	Cursor ur, ul, ll, lr;          /* cursors for rubber banding    */
	int change_cursor = FALSE;
        int current_cursor;
        int x1, y1;                     /* location of mouse            */
        int x2, y2;                     /* other corner of box          */
        int rootx, rooty;		/* for XQueryPointer */
	unsigned int mask;	        /* for XQueryPointer          */
	Window root, subw;		/* for XQueryPointer */
        int chosen = -1;
        int stop;
        XEvent ev;                      /* someplace to put the event   */
        int events;                     /* what events we want.             */

	GC gc;
	XGCValues gcvalues;
	Window root_win;
	Display *dpy;
	int current_x, current_y;

	dpy = XtDisplay(w);
	root_win = XtScreen(w)->root;
	events = PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
	ul = XCreateFontCursor(dpy, XC_ul_angle);

	if (XGrabPointer(dpy, root_win, FALSE, events, 
	    GrabModeAsync, GrabModeAsync, None, ul, CurrentTime) != 
	    GrabSuccess) {
		XtWarning("Can't Grab mouse in XukcDragBox()");
		XFreeCursor(dpy, ul);
		return;
	}

	(void)XQueryPointer(dpy, root_win, &root, &subw, 
				  &rootx, &rooty, &x1, &y1, &mask);

	/* make the GC that inverts the lines of a rectangle */
	XtVaGetValues(w, XtNbackground, &(gcvalues.foreground), NULL);
	gcvalues.function = GXinvert;
	gcvalues.subwindow_mode = IncludeInferiors;
	gc = XtGetGC(w, GCFunction | GCForeground | GCSubwindowMode, &gcvalues);

	*ret_x1 = *ret_y1 = *ret_x2 = *ret_y2 = 0;
	ur = XCreateFontCursor(dpy, XC_ur_angle);
	ll = XCreateFontCursor(dpy, XC_ll_angle);
	lr = XCreateFontCursor(dpy, XC_lr_angle);
	current_cursor = ul;

	current_x = x2 = x1; 
	current_y = y2 = y1;
	stop = FALSE;
	if (update_function != NULL)
		(*update_function)(w, x1, y1, x2, y2, FALSE);

	while (stop == FALSE) {
		XFlush(dpy);
		XNextEvent(dpy, &ev);

		switch (ev.type) {
			case ButtonPress:
				if (chosen != -1)
					break;
				
				chosen = ev.xbutton.button;
				current_x = x1 = x2 = ev.xbutton.x;
				current_y = y1 = y2 = ev.xbutton.y;
				if (chosen == Button2) {
					change_cursor = TRUE;
					XGrabServer(dpy);
				}
				break;
			case ButtonRelease:
				if (ev.xbutton.button != chosen)
					break;
				x2 = ev.xbutton.x;
				y2 = ev.xbutton.y;
				if (change_cursor) {
					XUngrabServer(dpy);
					DrawBox(dpy, root_win, gc,
							x1, y1, x2, y2);
				}
				stop = TRUE;
				break;
			case MotionNotify:
				if (XPending(dpy)) {
					XEvent pev;
					XPeekEvent(dpy, &pev);
					while (pev.type == MotionNotify) {
					    XNextEvent(dpy, &ev);
					    if (XPending(dpy))
						XPeekEvent(dpy, &pev);
					    else
						break;
					};
				}
				x2 = ev.xmotion.x;
				y2 = ev.xmotion.y;
				break;
			default:
				break;
		}
		if (current_x != x2 || current_y != y2) {
			if (change_cursor)
				DrawBox(dpy, root_win, gc, x1, y1,
						current_x, current_y);
			if (update_function != NULL)
				(*update_function)(w, x1, y1, x2, y2,
								change_cursor);
			if (change_cursor)
				DrawBox(dpy, root_win, gc, x1, y1, x2, y2);
		}
		current_x = x2;  current_y = y2;
		if (change_cursor) {
			if ((x2 >= x1) && (y2 >= y1) &&
			    current_cursor != lr) {
				XChangeActivePointerGrab(dpy, events, lr,
								CurrentTime);
				current_cursor = lr;
			} else if ((x2 >= x1) && (y2 < y1) &&
				   current_cursor != ur) {
				XChangeActivePointerGrab(dpy, events, ur,
								CurrentTime);
				current_cursor = ur;
			} else if ((x2 < x1) && (y2 >= y1) &&
				   current_cursor != ll) {
				XChangeActivePointerGrab(dpy, events, ll,
								CurrentTime);
				current_cursor = ll;
			} else if ((x2 < x1) && (y2 < y1) &&
				   (current_cursor != ul)) {
				XChangeActivePointerGrab(dpy, events, ul,
								CurrentTime);
				current_cursor = ul;
			}
		}
	}
	XUngrabServer(dpy);
	XUngrabPointer(dpy, CurrentTime);
	XFreeCursor(dpy, ur);
	XFreeCursor(dpy, ul);
	XFreeCursor(dpy, lr);
	XFreeCursor(dpy, ll);
	XtReleaseGC(w, gc);
	if (change_cursor) {
		*ret_x1 = MIN(x1, x2);
		*ret_y1 = MIN(y1, y2);
		*ret_x2 = MAX(x1, x2);
		*ret_y2 = MAX(y1, y2);
	} else {
		*ret_x1 = *ret_x2 = x2;
		*ret_y1 = *ret_y2 = y2;
	}
	XFlush(dpy);
}


XukcDragLine(w, w1, ret_x1, ret_y1, w2, ret_x2, ret_y2, update_function)
Widget w;
Window *w1;
int *ret_x1, *ret_y1;
Window *w2;
int *ret_x2, *ret_y2;
void (*update_function)();
{
	Cursor arrow1, arrow2;
        int x1, y1;                     /* location of mouse */
        int x2, y2;                     /* other end of line */
        int rootx, rooty;		/* for XQueryPointer */
        unsigned int mask;		/* for XQueryPointer */
	Window root, subw;		/* for XQueryPointer */
        int chosen = -1;
        int stop;
        XEvent ev;                      /* someplace to put the event */
        int events;                     /* what events we want. */

	GC gc;
	XGCValues gcvalues;
	Window root_win;
	Display *dpy;
	Boolean ok2draw;
	int current_x, current_y;

	*ret_x1 = *ret_y1 = *ret_x2 = *ret_y2 = 0;
	*w1 = *w2 = None;
	dpy = XtDisplay(w);
	root_win = XtScreen(w)->root;
	events = PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
	arrow1 = XCreateFontCursor(dpy, XC_draft_large);

	if (XGrabPointer(dpy, root_win, FALSE, events, 
	    GrabModeAsync, GrabModeAsync, None, arrow1, CurrentTime) != 
	    GrabSuccess) {
		XtWarning("Can't Grab the mouse in XukcDragLine()");
		XFreeCursor(dpy, arrow1);
		return;
	}

	(void)XQueryPointer(dpy, root_win, &root, &subw, 
				  &rootx, &rooty, &x1, &y1, &mask);

	arrow2 = XCreateFontCursor(dpy, XC_top_left_arrow);

	/* make the GC that inverts the line */
	XtVaGetValues(w, XtNbackground, &(gcvalues.foreground), NULL);
	gcvalues.function = GXinvert;
	gcvalues.subwindow_mode = IncludeInferiors;
	gc = XtGetGC(w, GCFunction | GCForeground | GCSubwindowMode, &gcvalues);

	current_x = x2 = x1; 
	current_y = y2 = y1;
	stop = FALSE;
	ok2draw = FALSE;
	if (update_function != NULL)
		(*update_function)(w, x1, y1, x2, y2, FALSE);

	while (stop == FALSE) {
		XFlush(dpy);
		XNextEvent(dpy, &ev);

		switch (ev.type) {
			case ButtonPress:
				if (chosen != -1)
					break;
				
				chosen = ev.xbutton.button;
				current_x = x1 = x2 = ev.xbutton.x;
				current_y = y1 = y2 = ev.xbutton.y;
				if (chosen == Button1) {
					*w1 = ev.xbutton.subwindow;
					ok2draw = TRUE;
					XDrawLine(dpy, root_win,
						gc, x1, y1, x2, y2);
					XChangeActivePointerGrab(dpy,
						events, arrow2, CurrentTime);
					XGrabServer(dpy);
				}
				break;
			case ButtonRelease:
				if (ev.xbutton.button != chosen)
					break;
				x2 = ev.xbutton.x;
				y2 = ev.xbutton.y;
				*w2 = ev.xbutton.subwindow;
				if (ok2draw) {
					XUngrabServer(dpy);
					XDrawLine(dpy, root_win,
						gc, x1, y1, x2, y2);
				}
				stop = TRUE;
				break;
			case MotionNotify:
				if (XPending(dpy)) {
					XEvent pev;
					XPeekEvent(dpy, &pev);
					while (pev.type == MotionNotify) {
					    XNextEvent(dpy, &ev);
					    if (XPending(dpy))
						XPeekEvent(dpy, &pev);
					    else
						break;
					};
				}
				x2 = ev.xmotion.x;
				y2 = ev.xmotion.y;
				break;
			default:
				break;
		}
		if (current_x != x2 || current_y != y2) {
			if (ok2draw)
				XDrawLine(dpy, root_win, gc, x1, y1,
						current_x, current_y);
			if (update_function != NULL)
				(*update_function)(w, x1, y1, x2, y2, ok2draw);
			if (ok2draw)
				XDrawLine(dpy, root_win, gc, x1, y1, x2, y2);
			current_x = x2;
			current_y = y2;
		}
	}
	XUngrabServer(dpy);
	XUngrabPointer(dpy, CurrentTime);
	XFreeCursor(dpy, arrow1);
	XFreeCursor(dpy, arrow2);
	XtReleaseGC(w, gc);
	if (*w1 == None)
		*w1 = root_win;
	if (*w2 == None)
		*w2 = root_win;
	if (ok2draw) {
		*w1 = XukcEnclosingWindow(w, *w1, x1, y1);
		if (*w1 != NULL) {
			XTranslateCoordinates(XtDisplay(w), root_win, *w1,
					x1, y1, ret_x1, ret_y1, &subw);
		} else {
			*ret_x1 = x1;
			*ret_y1 = y1;
		}
		*w2 = XukcEnclosingWindow(w, *w2, x2, y2);
		if (*w2 != NULL) {
			XTranslateCoordinates(XtDisplay(w), root_win, *w2,
					x2, y2, ret_x2, ret_y2, &subw);
		} else {
			*ret_x2 = x2;
			*ret_y2 = y2;
		}
	}
	XFlush(dpy);
}
