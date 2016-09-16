/* SCCSID: @(#)CommandP.h	1.1 (UKC) 5/10/92 */


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

******************************************************************/

/* 
 * CommandP.h - Private definitions for UKCCommand widget
 *
 * Updated by Richard Hesketh : rlh2@ukc.ac.uk
 * 
 */

#ifndef _XukcCommandP_h
#define _XukcCommandP_h

#include <X11/Xukc/LabelP.h>
#include <X11/Xukc/Command.h>

/***********************************************************************
 *
 * UKCCommand Widget Private Data
 *
 ***********************************************************************/

/************************************
 *
 *  Class structure
 *
 ***********************************/


   /* New fields for the UKCCommand widget class record */
typedef struct _UKCCommandClass 
  {
    Cardinal makes_compiler_happy;  /* not used */
  } UKCCommandClassPart;

   /* Full class record declaration */
typedef struct _UKCCommandClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    UKCLabelClassPart	label_class;
    UKCCommandClassPart    command_class;
} UKCCommandClassRec;

extern UKCCommandClassRec ukcCommandClassRec;

/***************************************
 *
 *  Instance (widget) structure 
 *
 **************************************/

    /* New fields for the UKCCommand widget record */
typedef struct _UKCCommandPart {
    /* resources */
    Dimension   highlight_thickness;
    XtCallbackList callbacks;

    /* private state */
    Pixel	highlight_color;
    Pixmap      gray_pixmap;
    GC		highlight_border_GC;
    GC          normal_GC;
    GC          inverse_GC;
    Boolean     set;
    Boolean     highlighted;
    Region	highlight_region;
    Region	set_region;
} UKCCommandPart;

   /* Full widget declaration */
typedef struct _UKCCommandRec {
    CorePart         core;
    SimplePart	     simple;
    UKCLabelPart	     label;
    UKCCommandPart      command;
} UKCCommandRec;

#endif _XukcCommandP_h
