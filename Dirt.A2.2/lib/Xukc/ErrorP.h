#ifndef _UKCErrorP_h
#define _UKCErrorP_h

/* SCCSID: @(#)ErrorP.h	1.1 (UKC) 5/10/92 */

/* 
 * ErrorP.h - Private definitions for UKCError widget
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
 * UKCError Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xukc/Error.h>
#include <X11/Xaw/FormP.h>

/* New fields for the UKCError widget class record */
typedef struct {int empty;} UKCErrorClassPart;

/* Full class record declaration */
typedef struct _UKCErrorClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart	constraint_class;
    FormClassPart	form_class;
    UKCErrorClassPart   error_class;
} UKCErrorClassRec;

extern UKCErrorClassRec ukcErrorClassRec;

/* New fields for the UKCError widget record */
typedef struct _UKCErrorPart {
	XtCallbackList callbacks;
	Boolean handle_xt;
	Boolean	handle_xlib;
	Boolean	history_list;
	Boolean pause;
	Boolean beep;
	int volume;
	String title;
	Boolean is_current;
	/* private resources */
	Widget hide_button;
	Widget clear_button;
	Widget error_text;
} UKCErrorPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _UKCErrorRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart  constraint;
    FormPart	    form;
    UKCErrorPart    error;
} UKCErrorRec;

#endif _UKCErrorP_h
