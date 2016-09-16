#ifndef _XukcScrList_h
#define _XukcScrList_h

/* SCCSID: @(#)ScrollList.h	1.1 (UKC) 5/10/92 */

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
 * UKCScrList Widget (subclass of UKC Viewport Class)
 *
 ***********************************************************************/

#include <X11/Xukc/Viewport.h>

/* uses children of the following classes */
#include <X11/Xaw/List.h>

#ifndef XtNlistWidget
#define XtNlistWidget "listWidget"
#endif /* XtNlistWidget */

#ifndef XtNlistName
#define XtNlistName "listName"
#endif /* XtNlistName */

#ifndef XtNhighlighted
#define XtNhighlighted "highlighted"
#endif /* XtNhighlighted */

#ifndef XtNlistAutoScroll
#define XtNlistAutoScroll "listAutoScroll"
#endif /* XtNlistAutoScroll */


#ifndef XtCAutoScroll
#define XtCAutoScroll "AutoScroll"
#endif /* XtCAutoScroll */

#ifndef XtCHighlighted
#define XtCHighlighted "Highlighted"
#endif /* XtCHighlighted */

#ifndef XtCListName
#define XtCListName "ListName"
#endif /* XtCListName */


/* Class record constants */

extern WidgetClass ukcScrListWidgetClass;

typedef struct _UKCScrListClassRec *UKCScrListWidgetClass;
typedef struct _UKCScrListRec      *UKCScrListWidget;

/* widget class public and convenience functions */

extern Boolean XukcScrListSelectItem(/* w, item_num, call, auto_scroll */);
/* Widget w;			# scrollList widget
 * Cardinal item_num;		# list entry to highlight
 * Boolean call;		# if TRUE call callbacks on XtNlistCallbacks
 * Boolean auto_scroll;		# whether to scroll list to selected item
 */

extern void XukcScrListUnselectItems(/* w */);
/* Widget w;
 */

extern void XukcScrListChange(/* w, new_list, new_list_length */);
/* Widget w;			#
 * String *new_list;		# Array of String items for list
 * Cardinal new_list_length;	# number of strings in "new_list"
 *				  (if == 0 then is calculated from null
 *				   terminated string list)
 */
#endif _XukcScrList_h
