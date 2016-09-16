#ifndef	_XukcMousePrivate_h
#define _XukcMousePrivate_h

/* SCCSID: @(#)MouseP.h	1.1 (UKC) 5/10/92 */

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

#include <X11/Xaw/SimpleP.h>
#include <X11/Xukc/Mouse.h>

/* number of nested enter/leave calls */
#define MAX_STACK 30

/* button offsets */
#define LEFT 0
#define MIDDLE 1
#define RIGHT 2

typedef struct dyingclients {
	Widget		widget;
	UKCMouseWidget	mw;
	Cardinal	this_client;
} _XukcMouseDyingClient;

typedef struct mouseyclients {
	Widget	widget;
	String	left_name;
	String	middle_name;
	String	right_name;
	Position name_pos_x[3];
	Position name_pos_y[3];
	Position name_lengths[3];
	_XukcMouseDyingClient *client_data;
} _XukcMouseClients;

typedef struct mouseybits {
	/* settable resources */

	Pixel		foreground;
	XFontStruct	*font;
	int		gravity;
	int		buttons_down;
	int		update_rate;

	/* data derived from resources */

	GC		gc;
	XRectangle	boxes[3];
	_XukcMouseClients **clients;
	Cardinal	num_clients;
	Cardinal	num_slots;
	short		current_client;
	XtIntervalId	timer;
	_XukcMouseDyingClient blank_mdc;
} UKCMousePart;

typedef struct _UKCMouseRec {
	CorePart	core;
	SimplePart	simple;
	UKCMousePart	mouse;
} UKCMouseRec;

typedef struct freddiethemouse {
	short	foo;
} UKCMouseClassPart;

/* Full Class Record declaration */

typedef struct	_UKCMouseClassRec {
	CoreClassPart	core_class;
	SimpleClassPart	simple_class;
	UKCMouseClassPart  mouse_class;
} UKCMouseClassRec;

/* Class record variable */

extern UKCMouseClassRec ukcMouseClassRec;

#endif	_XukcMousePrivate_h
