#ifndef	_XukcLayoutPrivate_h
#define _XukcLayoutPrivate_h

/* SCCSID: @(#)LayoutP.h	1.1 (UKC) 5/10/92 */

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

/********************************************************
 * UKCLayout Widget : Subclass of constraintWidgetClass *
 ********************************************************/

#include <X11/CompositeP.h>
#include <X11/Xukc/Layout.h>

#define HORIZONTAL 0
#define VERTICAL 1

struct _wcl {
	Widget		sibling;	/* widget connected to */
	Position	draw_pos;	/* x or y pos. for drawing conns */
	struct _wcl	*next;		/* next connection in list */
};

/* next or previous connection list */
typedef struct _wcl *Connections;

struct _wconns {
	Boolean		fixed;		    /* child is fixed in this axis */
	Boolean		resized;	    /* this child has been resized */
	double		dimension;	    /* reference width/height */
	double		unfixed_dimension;  /* amount of layout resizable */

	Connections	prev;		    /* next connection list */
	Connections	next;		    /* previous connection list */
};

/* connection details for constrained child widget */
typedef struct _wconns *Wconns;

struct _layoutconstraintpart {
	struct _wconns	connections[2];
	Boolean		managed_done;
};

typedef struct _layoutconstraintpart UKCLayoutConstraintPart;

struct _layoutconstraintrec {
	UKCLayoutConstraintPart layout;
};

typedef struct _layoutconstraintrec UKCLayoutConstraintRecord;
typedef struct _layoutconstraintrec *UKCLayoutConstraint;

typedef struct _layoutclasspart {
	int	mumble; /* no new fields */
} UKCLayoutClassPart;

/* Full Class Record declaration */

typedef struct	_UKCLayoutClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	UKCLayoutClassPart	layout_class;
} UKCLayoutClassRec;

/* Class record variable */

extern UKCLayoutClassRec	ukcLayoutClassRec;

struct _layoutpart {
	/* settable resources */
	Pixel		draw_vert_foreground;	/* colour for vertical lines */
	Pixel		draw_horiz_foreground;	/* colour for horiz. lines */
	Dimension	minimum_width;		/* minimum size layout can be */
	Dimension	minimum_height;		/* 	"	" ditto */
	int		geom_status;		/* flags for Geometry_handler */
	XtCallbackList	resize_done;		/* callback to say resize done */

	/* data derived from resources */
	Dimension	oldwidth;		/* used by Resize() */
	Dimension	oldheight;		/* used by Resize() */

	void		(*too_small)();		/* too_small is called when */
	Widget		too_small_widget;	/* layout widget is resized */
	Boolean		too_small_used;		/* smaller than minimum */

	Widget		current_widget;		/* the child currently in */
	Boolean		drawing_connections;	/* used by Redraw() */
	Boolean		resizing;		/* used by Resize() */

	struct _wconns	connections[2];		/* root of connection lists */
	Boolean		managed_once;		/* been managed atleast once */
};
	
typedef struct _layoutpart UKCLayoutPart;

typedef struct _UKCLayoutRec {
	CorePart	core;
	CompositePart	composite;
	ConstraintPart	constraint;
	UKCLayoutPart	layout;
} UKCLayoutRec;

#endif	_XukcLayoutPrivate_h
