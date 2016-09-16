#ifndef XukcICRM
#define XukcICRM v1.0

/* SCCSID: @(#)ICRM.h	1.2 (UKC) 5/10/92 */

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


/* available inviting parties/clients */

#define XukcResourceEditor 0
#define XukcDirtEditor 1

/* Atom names for inter-client communication for resource manipulation */

#define XtNicrmPartyGoer "_ICRM_PARTY_GOER_ID"
#define XtNicrmPartyHolder "_ICRM_PARTY_HOLDER_ID"

/* available inviting parties/clients */

#define XukcNresEditInvite "_ICRM_XT_RESOURCE_EDITOR_WINDOW"
#define XukcNdirtInvite "_ICRM_DIRT_EDITOR_WINDOW"

#define XukcNresEditDisplay "_ICRM_XT_RESOURCE_EDITOR_DISPLAY"
#define XukcNdirtDisplay "_ICRM_DIRT_EDITOR_DISPLAY"

/* ICRM protocol definition atoms */

#define XukcNicrmAcceptance "_ICRM_ACCEPTED_INVITATION"
#define XukcNicrmDroppedConnection "_ICRM_DROPPED_PARTY_CONNECTION"
#define XukcNicrmMasterDataProperty "_ICRM_MASTER_DATA_PROPERTY_%d"
#define XukcNicrmSlaveDataProperty "_ICRM_SLAVE_DATA_PROPERTY_%d"

#define XukcNicrmHierarchyRequest "_ICRM_REQUEST_WIDGET_HIERARCHY"
#define XukcNicrmHierarchyReturn "_ICRM_RETURNED_WIDGET_HEIRARCHY"

#define XukcNicrmNonHierarchyRequest "_ICRM_REQUEST_NON_HIERARCHY"
#define XukcNicrmNonHierarchyReturn "_ICRM_RETURNED_NON_HEIRARCHY"

#define XukcNicrmGetResourcesRequest "_ICRM_REQUEST_RESOURCES"
#define XukcNicrmGetResourcesReturn "_ICRM_RETURNED_RESOURCES"

#define XukcNicrmGetClassResourcesRequest "_ICRM_REQUEST_CLASS_RESOURCES"
#define XukcNicrmGetClassResourcesReturn "_ICRM_RETURNED_CLASS_RESOURCES"

#define XukcNicrmGetValuesRequest "_ICRM_REQUEST_GETVALUES"
#define XukcNicrmGetValuesReturn "_ICRM_RETURNED_GETVALUES"

#define XukcNicrmSetValuesRequest "_ICRM_REQUEST_SETVALUES"
#define XukcNicrmSetValuesReturn "_ICRM_RETURNED_SETVALUES_STATUS"

#define XukcNicrmGetActionListRequest "_ICRM_REQUEST_ACTION_LIST"
#define XukcNicrmGetActionListReturn "_ICRM_RETURNED_ACTION_LIST"

#define XukcNicrmConversionRequest "_ICRM_REQUEST_RESOURCE_TYPE_CONVERSION"
#define XukcNicrmConversionReturn "_ICRM_RETURNED_RESOURCE_TYPE_CONVERSION"

#define XukcNicrmWindowRequest "_ICRM_WINDOW_REQUEST"
#define XukcNicrmWindowReturn "_ICRM_WINDOW_RETURN"

#define XukcNicrmSetPropertyRequest "_ICRM_SET_PROPERTY_REQUEST"
#define XukcNicrmSetPropertyReturn "_ICRM_SET_PROPERTY_RETURN"

#define XukcNicrmGetPropertyRequest "_ICRM_GET_PROPERTY_REQUEST"
#define XukcNicrmGetPropertyReturn "_ICRM_GET_PROPERTY_RETURN"

struct _icrm_widget_hierarchy {
	Widget remote_widget;
	String remote_name;
	String widget_class;
	Boolean managed;
	Position x;
	Position y;
	Dimension width;
	Dimension height;
	Dimension border_width;
	struct _icrm_widget_hierarchy *parent;
	struct _icrm_widget_hierarchy *children;
	struct _icrm_widget_hierarchy *next;
	struct _icrm_widget_hierarchy *prev;
};

typedef struct _icrm_widget_hierarchy RemoteHierarchyRec;
typedef struct _icrm_widget_hierarchy *RemoteHierarchy;

struct _icrm_resource_list_rec {
	String resource_name;
	String resource_class;
	String resource_type;
};

typedef struct _icrm_resource_list_rec XukcResourceListRec;
typedef struct _icrm_resource_list_rec *XukcResourceList;

typedef void (*XukcIcrmProc)(
#if NeedFunctionPrototypes
	Cardinal /* client_num */,
	Window /* client_window */,
	Boolean /* status */,
	XtPointer /* arg1 */,
	XtPointer /* arg2 */
#endif
);

/* Public routines */

extern Boolean XukcMailInvites(/* dpy, party_place, client_type */);
/* Display *dpy;
 * Window party_place;
 * Cardinal client_type;
 */ 

extern void XukcFinishParty(/* client_type */);
/* Cardinal client_type; */

extern Boolean
XukcGetClientWidgetTree(/* client_type, comms_window, called_when_done */);
/* Cardinal client_type;
 * Window comms_window;
 * XukcIcrmProc called_when_done;
 */

extern String
XukcAtomToRequest(/* client_type, atom */);
/* Cardinal client_type;
 * Atom atom;
 */

extern void
XukcConditionalLoop(/* client_num, condition */);
/* Cardinal client_num;
 * Boolean *condition;
 */

extern void
XukcAppMainLoop( /* app_context, w*/);
/* XtAppContext app_context;
 * Widget w;
 */

extern void
XukcDestroyApplicationContext(/* app_context */);
/* XtAppContext app_context;
 */

extern Boolean
XukcRemoteGetValues( /* client_num, comms_window,
                      remote_widget_id, arglist,
                      num_args, c_proc
                    */
                 );
/* Cardinal client_num;
 * Window comms_window;
 * Widget remote_widget_id;
 * ArgList arglist;
 * Cardinal num_args;
 * XukcIcrmProc c_proc;
 */

extern Boolean
XukcIsClientWindow( /* client_num, win */ );
/* Cardinal client_num;
 * Window win;
 */

extern Boolean
XukcStillDoingTransfers( /* client_num, comms_window */ );
/* Cardinal client_num;
 * Window comms_window;
 */

extern Cardinal
XukcClientWindows( /* client_num */ );
/* Cardinal client_num;
 */

extern void
XukcFreeRemoteHierarchy( /* hierarchy, free_contents */ );
/* RemoteHierarchy hierarchy;
 * Boolean free_contents;
 */

extern Boolean
XukcRemoteActionList( /* client_num, comms_window,
                      remote_widget, action_list,
                      num_actions, c_proc */ );
/* Cardinal client_num;
 * Window comms_window;
 * Widget remote_widget;
 * String **action_list;
 * Cardinal *num_actions;
 * XukcIcrmProc c_proc;
 */

extern Boolean
XukcRemoteSetValues( /* client_num, comms_window,
                      remote_widget_id, arglist,
                      num_args, c_proc */ );
/* Cardinal client_num;
 * Window comms_window;
 * Widget remote_widget_id;
 * ArgList arglist;
 * Cardinal num_args;
 * XukcIcrmProc c_proc;
 */

extern Boolean
XukcRemoteWindowToWidget( /* client_num, comms_window,
                           remote_window, ret_w, c_proc */ );
/* Cardinal client_num;
 * Window comms_window;
 * Window remote_window;
 * Widget *ret_w;
 * XukcIcrmProc c_proc;
 */

extern Boolean
XukcRemoteConversion( /* client_num, comms_window,
                       remote_widget, from_type,
                         from, to_type, to, c_proc */ );
/* 
 * Cardinal client_num;
 * Window comms_window;
 * Widget remote_widget;
 * String from_type;
 * XrmValuePtr from;
 * String to_type;
 * XrmValuePtr *to;
 * XukcIcrmProc c_proc;
 */

#endif /* !XukcICRM */
