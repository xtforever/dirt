/*
 * SCCSID: @(#)ViewportP.h	1.2 (UKC) 5/10/92 
 * Private declarations for UKCViewportWidgetClass
 */

/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#ifndef _UKCViewportP_h
#define _UKCViewportP_h

#include <X11/Xaw/FormP.h>
#include <X11/Xukc/Viewport.h>

typedef struct {int empty;} UKCViewportClassPart;

typedef struct _UKCViewportClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    FormClassPart	form_class;
    UKCViewportClassPart	viewport_class;
} UKCViewportClassRec;

extern UKCViewportClassRec ukcViewportClassRec;

typedef struct _UKCViewportPart {
    /* resources */
    Boolean forcebars;		/* Whether we should always display */
				/* the selected scrollbars. */
    Boolean allowhoriz;		/* Whether we allow horizontal scrollbars. */
    Boolean allowvert;		/* Whether we allow vertical scrollbars. */
    Boolean usebottom;		/* True iff horiz bars appear at bottom. */
    Boolean useright;		/* True iff vert bars appear at right. */
    Boolean horiz_auto;		/* True iff scroll child to its left */
    Boolean vert_auto;		/* True iff scroll child to its bottom */
    /* private state */
    Widget clip, child;		/* The clipping and (scrolled) child widgets */
    Widget  horiz_bar, vert_bar;/* What scrollbars we currently have. */
} UKCViewportPart;

typedef struct _UKCViewportRec {
    CorePart	core;
    CompositePart	composite;
    ConstraintPart	constraint;
    FormPart		form;
    UKCViewportPart	viewport;
} UKCViewportRec;

typedef struct {
    /* resources */

    /* private state */
    Boolean		reparented; /* True if child has been re-parented */
} UKCViewportConstraintsPart;

typedef struct _UViewportConstraintsRec {
    FormConstraintsPart		form;
    UKCViewportConstraintsPart	viewport;
} UViewportConstraintsRec, *UViewportConstraints;

#endif _UKCViewportP_h
