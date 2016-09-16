#ifndef _UKCArrowP_h
#define _UKCArrowP_h

/* SCCSID: @(#)ArrowP.h	1.1 (UKC) 5/10/92 */

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

/* include superclass private header file */
#include <X11/Xaw/SimpleP.h>

#include <X11/Xukc/Arrow.h>


typedef struct {
    int empty;
} UKCArrowClassPart;

typedef struct _UKCArrowClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    UKCArrowClassPart	arrow_class;
} UKCArrowClassRec;

extern UKCArrowClassRec ukcArrowClassRec;

typedef struct {
    /* resources */
	Dimension thickness;
	Pixel foreground;
	XtCallbackList callbacks;
	int orientation;
	Boolean filled;
	int start_timeout;
	int repeat_timeout;
    /* private state */
	GC arrow_gc;
	GC invert_gc;
	Boolean inverted;
	XPoint arrows[8];
	XtIntervalId timeout_id;
} UKCArrowPart;

typedef struct _UKCArrowRec {
    CorePart	core;
    SimplePart	simple;
    UKCArrowPart	arrow;
} UKCArrowRec;

#endif  _UKCArrowP_h
