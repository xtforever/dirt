/* SCCSID: @(#)RowColP.h	1.1 (UKC) 5/10/92 */

/* Xukc Row Column Widget
 *
 * Author:  Richard Hesketh / rlh2@ukc.ac.uk, 
 *                Computing Lab. University of Kent at Canterbury, UK
 */

/* Modified heavily from: */

/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:        RCManagerP.h
 **
 **   Project:     X Widgets
 **
 **   Description: Private include file for the RCManager class
 **
 *****************************************************************************
 **   
 **   Copyright (c) 1988 by Hewlett-Packard Company
 **   Copyright (c) 1988 by the Massachusetts Institute of Technology
 **   
 **   Permission to use, copy, modify, and distribute this software 
 **   and its documentation for any purpose and without fee is hereby 
 **   granted, provided that the above copyright notice appear in all 
 **   copies and that both that copyright notice and this permission 
 **   notice appear in supporting documentation, and that the names of 
 **   Hewlett-Packard or  M.I.T.  not be used in advertising or publicity 
 **   pertaining to distribution of the software without specific, written 
 **   prior permission.
 **   
 *****************************************************************************
 *************************************<+>*************************************/

#ifndef _XukcRowColP_
#define _XukcRowColP_

#include <X11/ConstrainP.h>
#include <X11/Xukc/RowCol.h>

/*  The RC manager constraint structure  */

typedef struct _RCConstraintRec
{
   Dimension set_width;
   Dimension set_height;
} RCConstraintRec;


/*  Internally used layout structure  */

typedef struct _RCLayoutList
{
   Widget widget;
   Dimension border;
   Position x, y;
   Dimension width, height;
   Dimension set_width, set_height;
} RCLayoutList;


/*  New fields for the RowCol widget class record  */

typedef struct
{
   int mumble;   /* No new procedures */
} UKCRowColClassPart;


/* Full class record declaration */

typedef struct _UKCRowColClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   UKCRowColClassPart   row_col_class;
} UKCRowColClassRec;

extern UKCRowColClassRec ukcRowColClassRec;


/* New fields for the RowCol widget record */

typedef struct
{
   Dimension  h_space, v_space;
   Widget     selectedChild;
   int        layout_type;
   int        layout;
   int        columns;
   Boolean    force_size;
   Boolean    single_row;
   int        mode;
} UKCRowColPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _UKCRowColRec
{
    CorePart	   core;
    CompositePart  composite;
    ConstraintPart constraint;
    UKCRowColPart   row_col;
} UKCRowColRec;

#endif /* !_XukcRowColP_ */
