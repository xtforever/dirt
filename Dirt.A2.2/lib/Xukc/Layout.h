#ifndef _XukcLayoutH_h
#define _XukcLayoutH_h

/* SCCSID: @(#)Layout.h	1.1 (UKC) 5/10/92 */

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

/**********************************************************************
 *                           UKCLayoutWidget                          *
 *                                                                    *
 * Subclass of constraintWidgetClass : contrains children by the use  *
 * of horizontal and vertical connection lists.                       *
 **********************************************************************/

/************************  Default Widget Resources  *****************

static XtResource constraint_resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNhorizFixed        HorizFixed         Boolean          FALSE             
XtNvertFixed         VertFixed          Boolean          FALSE             

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNminWidth          MinWidth           Dimension        0                 
XtNminHeight         MinHeight          Dimension        0                 
XtNhorizColor        Foreground         Pixel            XtExtdefaultforeground
XtNvertColor         Foreground         Pixel            XtExtdefaultforeground
XtNgeomStatus        GeomStatus         GeomStatus       (XukcAllowMove|XukcAllowResize)
XtNresizeDone        Callback           Callback         (XtPointer)NULL   
*/

#include <X11/Composite.h>

/*
 * New Resources/Parameters :
 */ 

#ifndef XtNvertFixed
#define XtNvertFixed "vertFixed"
#endif /* XtNvertFixed */

#ifndef XtNhorizFixed
#define XtNhorizFixed "horizFixed"
#endif /* XtNhorizFixed */

#ifndef XtNminWidth
#define XtNminWidth "minWidth"
#endif /* XtNminWidth */

#ifndef XtNminHeight
#define XtNminHeight "minHeight"
#endif /* XtNminHeight */

#ifndef XtNvertColor
#define XtNvertColor "vertColor"
#endif /* XtNvertColor */

#ifndef XtNhorizColor
#define XtNhorizColor "horizColor"
#endif /* XtNhorizColor */

#ifndef XtNgeomStatus
#define XtNgeomStatus "geomStatus"
#endif /* XtNgeomStatus */

#ifndef XtNresizeDone
#define XtNresizeDone "resizeDone"
#endif /* XtNresizeDone */


#ifndef XtCVertFixed
#define XtCVertFixed "VertFixed"
#endif /* XtCVertFixed */

#ifndef XtCHorizFixed
#define XtCHorizFixed "HorizFixed"
#endif /* XtCHorizFixed */

#ifndef XtCMinWidth
#define XtCMinWidth "MinWidth"
#endif /* XtCMinWidth */

#ifndef XtCMinHeight
#define XtCMinHeight "MinHeight"
#endif /* XtCMinHeight */

#ifndef XtCHorizColor
#define XtCHorizColor "HorizColor"
#endif /* XtCHorizColor */

#ifndef XtCVertColor
#define XtCVertColor "VertColor"
#endif /* XtCVertColor */

#ifndef XtCGeomStatus
#define XtCGeomStatus "GeomStatus"
#endif /* XtCGeomStatus */


#ifndef XtRGeomStatus
#define XtRGeomStatus "GeomStatus"
#endif /* XtRGeomStatus */


#ifndef XukcETurnedOff
#define XukcETurnedOff "turnedoff"
#endif /* XukcETurnedOff */

#ifndef XukcEAllowMove
#define XukcEAllowMove "allowmove"
#endif /* XukcEAllowMove */

#ifndef XukcEAllowResize
#define XukcEAllowResize "allowresize"
#endif /* XukcEAllowResize */

#ifndef XukcEOnlyAskParent
#define XukcEOnlyAskParent "onlyaskparent"
#endif /* XukcEOnlyAskParent */

#ifndef XukcERefuseAll
#define XukcERefuseAll "refuseall"
#endif /* XukcERefuseAll */


#define XukcTurnedOff	0x1
#define XukcAllowMove	0x2
#define XukcAllowResize	0x4
#define XukcOnlyAskParent	0x8
#define XukcRefuseAll	0x10

/* enum used by XukcLayoutAdd/XukcLayoutRemoveConnection */
#ifndef _XukcPositioning_
#define _XukcPositioning_ 1

typedef enum _pos {
	horizontal_before,
	horizontal_after,
	vertical_above,
	vertical_below
} Positioning;
#endif /* _XukcPositioning_ */

/* Class record pointer */

extern WidgetClass	ukcLayoutWidgetClass;

/* Widget Type definition */

typedef	struct _UKCLayoutRec	*UKCLayoutWidget;
typedef struct _UKCLayoutClassRec	*UKCLayoutWidgetClass;

/*** PUBLIC ROUTINES defined in UKCLayout Widget ***/

/* new XtCreateWidget() routine .. either use this one for all children of
 * a layoutWidget or call the normal routine and specify values for the
 * XtNvertFixed and XtNhorizFixed resources.
 */
extern Widget XukcLayoutCreateWidget();

#define XukcLayoutAddConnection(a, b, c, d) \
				AddOrRemoveConnection(TRUE, a, b, c, d)
#define XukcLayoutRemoveConnection(a, b, c, d) \
				AddOrRemoveConnection(FALSE, a, b, c, d)
extern void AddOrRemoveConnection(); /* access via the previous defines */
/* a =  UKCLayoutWidget layout_widget;
   b =	Widget widget;
   c =	Widget widget2connect2;
   d =	Cardinal postioning;
*/
#define XukcLayoutChangeConnection(a, b, c, d, e, f) \
		XukcLayoutRemoveConnection(a, b, c, d); \
		XukcLayoutAddConnection(a, b, e, f);
/* where ...
   a =	UKCLayoutWidget layout_widget;
   b =	Widget node_widget;
   c =	Widget widget_connection2remove;
   d =	Cardinal position_of_widget_connection2remove;
   e =	Widget widget_connection2add;
   f =	Cardinal position_of_widget_connection2add;
*/
	
extern void XukcDisplayLayoutConnections(/* lw, turn_on */);
/* Widget lw;   - layout widget to display connections of.
 * Boolean turn_on;  - true to show the connections, false to clear them
 */

extern void XukcLayoutRefreshConnections(/* lw */);
/* Widget lw;   - layout widget to refresh the displayed connections on.
 */

/* The following actions are added by this widget class:
 *
 * "draw_connections"		- Draws lines between connected widgets
 * "clear_connections"		- Clears any lines drawn
 * "print_widget_connections"	- Prints (stdout) the connections to the widget
 * "print_connections"		- Prints all connections on layout widget
 */
#endif	_XukcLayoutH_h
