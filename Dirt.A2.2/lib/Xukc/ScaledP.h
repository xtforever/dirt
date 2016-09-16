#ifndef	_XukcScaledPrivate_h
#define _XukcScaledPrivate_h

/* SCCSID: @(#)ScaledP.h	1.1 (UKC) 5/10/92 */

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

/**************************************************************
 *    UKCScaled Widget : Subclass of constraintWidgetClass    *
 **************************************************************/

#include <X11/CompositeP.h>
#include <X11/Xukc/Scaled.h>

typedef struct _ukcScaledClassPart {
	char	mumble; /* no new fields */
} UKCScaledClassPart;

/* Full Class Record declaration */

typedef struct	_UKCScaledClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	UKCScaledClassPart	scaled_class;
} UKCScaledClassRec;

/* Class record variable */

extern UKCScaledClassRec	ukcScaledClassRec;

struct _ukcScaledPart {
	char	mumble;	/* no new fields */
};
	
typedef struct _ukcScaledPart UKCScaledPart;

typedef struct _UKCScaledRec {
	CorePart	core;
	CompositePart	composite;
	ConstraintPart	constraint;
	UKCScaledPart	scaled;
} UKCScaledRec;


/* constraints */

struct _scaledconstraintpart {
	double x;
	double y;
	double width;
	double height;
};

typedef struct _scaledconstraintpart UKCScaledConstraintPart;

struct _scaledconstraintrec {
	UKCScaledConstraintPart scaled;
};

typedef struct _scaledconstraintrec UKCScaledConstraintRecord;
typedef struct _scaledconstraintrec *UKCScaledConstraint;

#endif	_XukcScaledPrivate_h

