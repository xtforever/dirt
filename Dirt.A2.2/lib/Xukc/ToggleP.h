#ifndef	_XukcTogglePrivate_h
#define _XukcTogglePrivate_h

/* SCCSID: @(#)ToggleP.h	1.1 (UKC) 5/10/92 */

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

/****************************************************************
 *     UKCToggle Widget : Subclass of ukcCommandWidgetClass     *
 ****************************************************************/

/* my superclasses definitions and resources */
#include <X11/Xukc/CommandP.h>

/* my resources */
#include <X11/Xukc/Toggle.h>

typedef struct _ukctoggleclasspart {
	short	mumble; /* no new fields */
} UKCToggleClassPart;

/* Full Class Record declaration */

typedef struct	_UKCToggleClassRec {
	CoreClassPart		core_class;
	SimpleClassPart		simple_class;
	UKCLabelClassPart	label_class;
	UKCCommandClassPart	command_class;
	UKCToggleClassPart	toggle_class;
} UKCToggleClassRec;

/* Class record variable */

extern UKCToggleClassRec	ukcToggleClassRec;

struct _UKCTogglePart {
	/* the address of an application variable which gets toggled */
	Boolean	*toggle_address;
	Boolean state;		/* buttons current state */

	/* a chain of radio buttons */
	Widget	prev_toggle;	/* private */
	Widget	next_toggle;	/* public */
	int	radio_value;	/* value to pass if a radio button */
	Boolean	inform_when_off; /* tell radio button when it becomes off */

	Boolean	on_left;	/* where to put the bitmaps */
	Pixmap	on_pixmap;	/* bitmap to show when state is set */
	Pixmap	off_pixmap;	/* bitmap to show when state is reset */

	/* Private */
	char	internal_state; /* set of flags for internal use */
};
	
typedef struct _UKCTogglePart UKCTogglePart;

typedef struct _UKCToggleRec {
	CorePart	core;
	SimplePart	simple;
	UKCLabelPart	label;
	UKCCommandPart	command;
	UKCTogglePart	toggle;
} UKCToggleRec;

#endif	_XukcTogglePrivate_h
