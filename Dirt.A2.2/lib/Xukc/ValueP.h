#ifndef _UKCValueP_h
#define _UKCValueP_h

/* SCCSID: @(#)ValueP.h	1.1 (UKC) 5/10/92 */

/* 
 * UKCValueP.h - Private definitions for UKCValue widget
 * 
 */

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

/***********************************************************************
 *
 * UKCValue Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xukc/Value.h>
#include <X11/Xukc/LayoutP.h>

#define STRLEN 30

/* New fields for the UKCValue widget class record */
typedef struct {int empty;} UKCValueClassPart;

/* Full class record declaration */
typedef struct _UKCValueClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart	constraint_class;
    UKCLayoutClassPart	layout_class;
    UKCValueClassPart   value_class;
} UKCValueClassRec;

extern UKCValueClassRec ukcValueClassRec;

/* New fields for the UKCValue widget record */
typedef struct _UKCValuePart {
	int	orientation;
	Boolean	no_border;
	int	value;
	int	min_value;
	int	max_value;
	XtCallbackList callbacks;
	/* read-only */
	Widget	inc_arrow;
	Widget	dec_arrow;
	Widget	value_text;
} UKCValuePart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _UKCValueRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart  constraint;
    UKCLayoutPart   layout;
    UKCValuePart    value;
} UKCValueRec;

#endif _UKCValueP_h
