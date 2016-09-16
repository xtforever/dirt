/* SCCSID: @(#)ScrollListP.h	1.1 (UKC) 5/10/92 */

/* 
 * ScrollListP.h - Private definitions for UKC Scrollable List widget
 * 
 */

#ifndef _UKCScrListP_h
#define _UKCScrListP_h

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
 * UKCScrList Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/ViewportP.h>

/* New fields for the UKCScrList widget class record */
typedef struct {int empty;} UKCScrListClassPart;

/* Full class record declaration */
typedef struct _UKCScrListClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart	constraint_class;
    FormClassPart	form_class;
    UKCViewportClassPart	viewport_class;
    UKCScrListClassPart   scrlist_class;
} UKCScrListClassRec;

extern UKCScrListClassRec ukcValueClassRec;

/* New fields for the UKCScrList widget record */
typedef struct _UKCScrListPart {
	XtCallbackList list_callback;
	String *string_list;
	Cardinal num_strings;
	String list_name;
	int highlighted;
	Boolean list_scroll;
	/* read-only */
	Widget	list_widget;
	/* private state */
	PubMark mark;
} UKCScrListPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _UKCScrListRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    FormPart		form;
    UKCViewportPart	viewport;
    UKCScrListPart	scrlist;
} UKCScrListRec;

#endif _UKCScrListP_h
