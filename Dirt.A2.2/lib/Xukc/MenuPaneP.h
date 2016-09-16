#ifndef	_XukcMenuPanePrivate_h
#define _XukcMenuPanePrivate_h

/* SCCSID: @(#)MenuPaneP.h	1.1 (UKC) 5/10/92 */

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
 *	                     UKCMenuPaneWidget			      *
 *                                                                    *
 * Subclass of constraintWidgetClass :looks after its children either *
 *                               horizontally or vertically arranged  *
 **********************************************************************/

#include <X11/ConstrainP.h>
#include <X11/Xukc/MenuPane.h>

struct _mpconstraintpart {
	Dimension	dimension;	/* reference dimension */
	Position	new_dimension;	/* the resized dimension */
};

typedef struct _mpconstraintpart MPConstraintPart;

struct _mpconstraintrec {
	MPConstraintPart menuPane;
};

typedef struct _mpconstraintrec MPConstraintRecord;
typedef MPConstraintRecord *MPConstraint;


typedef struct _ukcMenuPaneClassPart {
	short	mumble; /* no new fields */
} UKCMenuPaneClassPart;

/* Full Class Record declaration */

typedef struct	_UKCMenuPaneClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	UKCMenuPaneClassPart	menuPane_class;
} UKCMenuPaneClassRec;

/* Class record variable */

extern UKCMenuPaneClassRec	ukcMenuPaneClassRec;

struct _ukcMenuPanePart {
	/* settable resources */
	Boolean	vert_pane;	/* TRUE if menu pane is vertical */
	Boolean	ask_child;	/* TRUE if child is asked its prefered size */
	Boolean resize;		/* TRUE if pane should resize to fit children */
};
	
typedef struct _ukcMenuPanePart UKCMenuPanePart;

typedef struct _UKCMenuPaneRec {
	CorePart	core;
	CompositePart	composite;
	ConstraintPart	constraint;
	UKCMenuPanePart	menuPane;
} UKCMenuPaneRec;

#endif	_XukcMenuPanePrivate_h
