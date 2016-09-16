/* SCCSID: @(#)RowCol.h	1.1 (UKC) 5/10/92 */

/* Xukc Row Column Widget
 *
 * Author:  Richard Hesketh / rlh2@ukc.ac.uk, 
 *                Computing Lab. University of Kent at Canterbury, UK
 */

/* Heavily modified from: */

/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:        RCManger.h
 **
 **   Project:     X Widgets
 **
 **   Description: Public include file for the RCManager class
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

#ifndef _XukcRowColWidget_
#define _XukcRowColWidget_

#include <X11/Constraint.h>

#ifndef XtNhSpace
#define XtNhSpace "hSpace"
#endif /* XtNhSpace */

#ifndef XtCHSpace
#define XtCHSpace "HSpace"
#endif /* XtCHSpace */


#ifndef XtNvSpace
#define XtNvSpace "vSpace"
#endif /* XtNvSpace */

#ifndef XtCVSpace
#define XtCVSpace "VSpace"
#endif /* XtCVSpace */


#ifndef XtNlayoutType
#define XtNlayoutType "layoutType"
#endif /* XtNlayoutType */

#ifndef XtCLayoutType
#define XtCLayoutType "LayoutType"
#endif /* XtCLayoutType */

#ifndef XtRLayoutType
#define XtRLayoutType "LayoutType"
#endif /* XtRLayoutType */


#ifndef XukcERequestedColumns
#define XukcERequestedColumns "requestedColumns"
#endif /* XukcERequestedColumns */

#ifndef XukcEMaximumColumns
#define XukcEMaximumColumns "maximumColumns"
#endif /* XukcEMaximumColumns */

#ifndef XukcEMaximumUnaligned
#define XukcEMaximumUnaligned "maximumUnaligned"
#endif /* XukcEMaximumUnaligned */


#define XukcRequestedColumns (1 << 0)
#define XukcMaximumColumns (1 << 1)
#define XukcMaximumUnaligned (1 << 2)

#ifndef XtNlayout
#define XtNlayout "layout"
#endif /* XtNlayout */

#ifndef XtCLayout
#define XtCLayout "Layout"
#endif /* XtCLayout */

#ifndef XtRLayout
#define XtRLayout "Layout"
#endif /* XtRLayout */


#ifndef XukcELayoutMaximize
#define XukcELayoutMaximize "maximize"
#endif /* XukcELayoutMaximize */

#ifndef XukcELayoutMinimize
#define XukcELayoutMinimize "minimize"
#endif /* XukcELayoutMinimize */

#ifndef XukcELayoutIgnore
#define XukcELayoutIgnore "ignore"
#endif /* XukcELayoutIgnore */


#define XukcLayoutMaximize (1 << 0)
#define XukcLayoutMinimize (1 << 1)
#define XukcLayoutIgnore (1 << 2)

#ifndef XtNcolumns
#define XtNcolumns "columns"
#endif /* XtNcolumns */

#ifndef XtCColumns
#define XtCColumns "Columns"
#endif /* XtCColumns */


#ifndef XtNforceSize
#define XtNforceSize "forceSize"
#endif /* XtNforceSize */

#ifndef XtCForceSize
#define XtCForceSize "ForceSize"
#endif /* XtCForceSize */


#ifndef XtNsingleRow
#define XtNsingleRow "singleRow"
#endif /* XtNsingleRow */

#ifndef XtCSingleRow
#define XtCSingleRow "SingleRow"
#endif /* XtCSingleRow */


/* Class record constants */

extern WidgetClass ukcRowColWidgetClass;

typedef struct _UKCRowColClassRec *UKCRowColWidgetClass;
typedef struct _UKCRowColRec      *UKCRowColWidget;

#endif /* !_XukcRowColWidget_ */
