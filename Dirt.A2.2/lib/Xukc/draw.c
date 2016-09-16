#ifndef lint
static char sccsid[] = "@(#)draw.c	1.2 (UKC) 5/10/92";
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


/********* Buffered Drawing Functions for X Toolkit Widgets ********/

#include <X11/Intrinsic.h>

#define OBJECT_BUFFER_SIZE 100

static XRectangle _rectangles[OBJECT_BUFFER_SIZE];
static int _num_rectangles = 0;
static GC _rectangle_gc = NULL;
static Widget _rectangle_widget = NULL;
static Boolean _rectangles_buffered = TRUE;

static XSegment _lines[OBJECT_BUFFER_SIZE];
static int _num_lines = 0;
static Widget _line_widget = NULL;
static GC _line_gc = NULL;
static Boolean _lines_buffered = TRUE;

static XPoint _points[OBJECT_BUFFER_SIZE];
static int _num_points = 0;
static Widget _point_widget = NULL;
static GC _point_gc = NULL;
static int _point_mode = CoordModeOrigin;
static Boolean _points_buffered = TRUE;

/**************** Drawing Rectangles *******************/
void
XukcFlushBufferedRectangles()
{
	if (_num_rectangles == 0)
		return;

	XDrawRectangles(XtDisplay(_rectangle_widget),
			XtWindow(_rectangle_widget), 
			_rectangle_gc, _rectangles, _num_rectangles);
	XFlush(XtDisplay(_rectangle_widget));
	_num_rectangles = 0;
	_rectangle_widget = NULL;
}


void
XukcDrawRectangle(w, gc, x, y, width, height)
Widget w;
GC gc;
Position x, y;
Dimension width, height;
{
	Boolean do_flush;

	if (_rectangles_buffered) {
		if (_num_rectangles < OBJECT_BUFFER_SIZE) {
			do_flush = FALSE; /* don't flush the request yet. */

			if (_rectangle_widget == NULL) {
				_rectangle_widget = w;
				_rectangle_gc = gc;
			} else if (_rectangle_widget != w ||
				   _rectangle_gc != gc) {
				XukcFlushBufferedRectangles();
				_rectangle_widget = w;
				_rectangle_gc = gc;
			}
		} else { /* buffer full so flush it before this request */
			XukcFlushBufferedRectangles();
			_rectangle_widget = w;
			_rectangle_gc = gc;
		}
	} else
		do_flush = TRUE;

	_rectangles[_num_rectangles].x = x;
	_rectangles[_num_rectangles].y = y;
	_rectangles[_num_rectangles].width = width;
	_rectangles[_num_rectangles++].height = height;

	if (do_flush)
		XukcFlushBufferedRectangles();
}


void
XukcBufferDrawRectangles(mode)
Boolean mode;
{
	_rectangles_buffered = mode;
	if (!mode)
		XukcFlushBufferedRectangles();
}


/**************** Drawing Lines *******************/

void
XukcFlushBufferedLines()
{
	if (_num_lines == 0)
		return;

	XDrawSegments(XtDisplay(_line_widget), XtWindow(_line_widget), 
			_line_gc, _lines, _num_lines);
	XFlush(XtDisplay(_line_widget));
	_num_lines = 0;
	_line_widget = NULL;
}


void
XukcDrawLine(w, gc, x1, y1, x2, y2)
Widget w;
GC gc;
Position x1, y1, x2, y2;
{
	Boolean do_flush;

	if (_lines_buffered) {
		if (_num_lines < OBJECT_BUFFER_SIZE) {
			do_flush = FALSE; /* don't flush the request yet. */

			if (_line_widget == NULL) {
				_line_widget = w;
				_line_gc = gc;
			} else if (_line_widget != w ||
				   _line_gc != gc) {
				XukcFlushBufferedLines();
				_line_widget = w;
				_line_gc = gc;
			}
		} else { /* buffer full so flush it before this request */
			XukcFlushBufferedLines();
			_line_widget = w;
			_line_gc = gc;
		}
	} else
		do_flush = TRUE;

	_lines[_num_lines].x1 = x1;
	_lines[_num_lines].y1 = y1;
	_lines[_num_lines].x2 = x2;
	_lines[_num_lines++].y2 = y2;

	if (do_flush)
		XukcFlushBufferedLines();
}


void
XukcBufferDrawLines(mode)
Boolean mode;
{
	_lines_buffered = mode;
	if (!mode)
		XukcFlushBufferedLines();
}


/********************* Draw Points **************************/

void
XukcFlushBufferedPoints()
{
	if (_num_points == 0)
		return;

	XDrawPoints(XtDisplay(_point_widget), XtWindow(_point_widget), 
			_point_gc, _points, _num_points, _point_mode);
	XFlush(XtDisplay(_point_widget));
	_num_points = 0;
	_point_widget = NULL;
}


void
XukcDrawPoint(w, gc, x1, y1)
Widget w;
GC gc;
Position x1, y1;
{
	Boolean do_flush;

	if (_points_buffered) {
		if (_num_points < OBJECT_BUFFER_SIZE) {
			do_flush = FALSE; /* don't flush the request yet. */

			if (_point_widget == NULL) {
				_point_widget = w;
				_point_gc = gc;
			} else if (_point_widget != w ||
				   _point_gc != gc) {
				XukcFlushBufferedPoints();
				_point_widget = w;
				_point_gc = gc;
			}
		} else { /* buffer full so flush it before this request */
			XukcFlushBufferedPoints();
			_point_widget = w;
			_point_gc = gc;
		}
	} else
		do_flush = TRUE;

	_points[_num_points].x = x1;
	_points[_num_points++].y = y1;

	if (do_flush)
		XukcFlushBufferedPoints();
}


void
XukcBufferDrawPoints(mode)
Boolean mode;
{
	_points_buffered = mode;
	if (!mode)
		XukcFlushBufferedPoints();
}


void
XukcDrawPointsMode(mode)
int mode;
{
	_point_mode = mode;
}


/***********************************************************/

void
XukcFlushBufferedOutput()
{
	XukcFlushBufferedRectangles();
	XukcFlushBufferedLines();
	XukcFlushBufferedPoints();
}
