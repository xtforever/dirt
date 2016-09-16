/* SCCSID: @(#)PassiveP.h	1.1 (UKC) 5/10/92 */

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
 *    UKCPassive Widget : Subclass of compositeWidgetClass    *
 **************************************************************/

#ifndef	_XukcPassivePrivate_h
#define _XukcPassivePrivate_h

#include <X11/CompositeP.h>
#include <X11/Xukc/Passive.h>

typedef struct _ukcPassiveClassPart {
	int	mumble; /* no new fields */
} UKCPassiveClassPart;

/* Full Class Record declaration */

typedef struct	_UKCPassiveClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	UKCPassiveClassPart	passive_class;
} UKCPassiveClassRec;

/* Class record variable */

extern UKCPassiveClassRec	ukcPassiveClassRec;

struct _ukcPassivePart {
	XtCallbackList	resize_callbacks;
	XtCallbackList	expose_callbacks;
	Pixel	foreground;
	Cursor  cursor;
	Boolean use_bs;
};
	
typedef struct _ukcPassivePart UKCPassivePart;

typedef struct _UKCPassiveRec {
	CorePart	core;
	CompositePart	composite;
	UKCPassivePart	passive;
} UKCPassiveRec;

#endif	_XukcPassivePrivate_h
