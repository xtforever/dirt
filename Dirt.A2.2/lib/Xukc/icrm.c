#ifndef lint
/* From:   "$Xukc: icrm.c,v 1.11 91/12/17 10:01:41 rlh2 Rel $"; */
static char sccsid[] = "@(#)icrm.c	1.3 (UKC) 5/17/92";
#endif /* !lint */

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

/* this module contains utility routines for the resource editor / DIRT!
 * editor to allow automatic connections between them and running clients
 * so that resources can be examined/modified behind the clients back!
 * The client must be linked against these for this to happen.
 */
#define DEBUG 1
#undef DEBUG

#include <stdio.h>
#include <ctype.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xukc/ICRM.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/utils.h> 

extern String *XukcWidgetClassActionList();
extern void XukcCopyToArgVal();

static void update_root_properties();
static void add_request_to_pending();
static Cardinal move_pending_to_in_progress();

#define IS_STRING_TYPE(type) (strcmp(type, XtRString) == 0)

#define CLASS_RESOURCES 1
#define CONSTRAINT_RESOURCES 2 

struct _icrm_protocol_atoms {
	/* communication atoms that define the ICRM protocol */
	Atom ACCEPTANCE;		/* atom sent to accept an invitation */
	Atom DROPPED_CONNECTION;	/* connection was dropped | force 1 */
	Atom HIERARCHY_REQUEST;		/* request for widget heirarchy */
	Atom HIERARCHY_RETURN;		/* indicates where the tree is */
	Atom NON_HIERARCHY_REQUEST;	/* request for application resources */
	Atom NON_HIERARCHY_RETURN;	/* indicates what these resources are */
	Atom WINDOW_REQUEST;		/* ask for a window to widget */
	Atom WINDOW_RETURN;		/* the widget id of the window */
	Atom SET_PROPERTY_REQUEST;	/* ask for a special value to be set */
	Atom SET_PROPERTY_RETURN;	/* the named value change acknowledged*/
	Atom GET_PROPERTY_REQUEST;	/* ask for a special value */
	Atom GET_PROPERTY_RETURN;	/* the named value returned */
	Atom GET_RESOURCES_REQUEST;	/* ask for list of widget's resources */
	Atom GET_RESOURCES_RETURN;	/* returned list of widget resources */
	Atom GET_CLASS_RESOURCES_REQUEST;/* ask for list of class's resources */
	Atom GET_CLASS_RESOURCES_RETURN;/* returned list of class resources */
	Atom SET_VALUES_REQUEST;	/* set a resource value */
	Atom SET_VALUES_RETURN;		/* indicates whether it was achieved */
	Atom GET_VALUES_REQUEST;	/* get a resource value request */
	Atom GET_VALUES_RETURN;		/* indicates where the values are */
	Atom GET_ACTION_LIST_REQUEST;	/* what are the available actions */
	Atom GET_ACTION_LIST_RETURN;	/* returns the actions */
	Atom CONVERSION_REQUEST;	/* convert a resource value remotely */
	Atom CONVERSION_RETURN;		/* converted value */
/* ||| NYI ||| */
	Atom CREATE_WIDGET_REQUEST;	/* create a remote widget */
	Atom CREATE_WIDGET_RETURN;	/* returns an id. to it */
	Atom DESTROY_WIDGET_REQUEST;	/* destroy a remote widget */
	Atom DESTROY_WIDGET_RETURN;	/* returns a status */
	Atom ADD_CALLBACK_REQUEST;	/* set a remote callback */
	Atom ADD_CALLBACK_RETURN;	/* returns a status */
	Atom REMOVE_CALLBACK_REQUEST;	/* set a remote callback */
	Atom REMOVE_CALLBACK_RETURN;	/* returns a status */
	Atom CALLED_CALLBACK;		/* a remote callback has happened */
/* ||| NYI ||| */
};

struct _party_details {
	String party_window_name;
	String party_display_name;
	Atom PARTY_WINDOW;
	Atom PARTY_DISPLAY;
	Atom MASTER_DATA_PROPERTY;	/* atom of data sent to slave */
	Widget w;			/* the widget who we use for a window */
	Window party_window;		/* the initial comms window */
	Display *party_dpy;		/* the comms display */
	Cardinal rsvp_windows;		/* where the partygoers are! */
	Cardinal requests_pending;	/* list of unprocessed requests */
	Cardinal requests_pending_on;	/* list of windows they are on */
	Cardinal requests_pending_data;
	Cardinal requests_pending_length;
	Cardinal requests_pending_msg;
	Cardinal requests_pending_args;	/* list of any arguments needed */
	Cardinal requests_pending_num_args; /* number of arguments in list */
	Cardinal requests_pending_callback; /* list of completion callbacks */
	Cardinal requests_in_progress;	/* list of being processed requests */
	Cardinal requests_in_progress_on; /* list of windows they are on */
	Cardinal requests_in_progress_args; /* list of any arguments needed */
	Cardinal requests_in_progress_num_args; /*number of arguments in list */
	Cardinal requests_in_progress_callback; /*list of completion callbacks*/
	struct _icrm_protocol_atoms atoms;
};

static struct _party_details partys[] = {
 { XukcNresEditInvite, XukcNresEditDisplay, (Atom)NULL, (Atom)NULL, (Atom)NULL,
   (Widget)NULL, (Window)NULL, (Display *)NULL,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
 { XukcNdirtInvite, XukcNdirtDisplay, (Atom)NULL, (Atom)NULL, (Atom)NULL,
   (Widget)NULL, (Window)NULL, (Display *)NULL,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

#define PARTY partys[client_num]

struct _invitations {
	String client_window;		/* inviting client's window property */
	String client_display_name;	/* inviting client's display property */
	Atom INVITING_CLIENT;		/* atom rep. of "client_window" */
	Atom INVITING_CLIENT_DISPLAY;	/* atom rep. of "client_display_name" */
	Window inviting_client_window;	/* inviting client's comms window */
	Display *inviting_dpy;		/* inviting client's comms display */
	Window my_window;		/* my side of the comms window */
	Display *my_display;		/* my display */
	Cardinal my_property;		/* my data property list */
	Atom SLAVE_DATA_PROPERTY;	/* name of the data transfer prop.  */
	struct _icrm_protocol_atoms atoms;
};

static struct _invitations _inviting_clients[] = {
 { XukcNresEditInvite, XukcNresEditDisplay, (Atom)NULL, (Atom)NULL,
   (Window)NULL, (Display *)NULL, (Window)NULL, (Display *)NULL, (Cardinal)0,
   (Atom)NULL },
 { XukcNdirtInvite, XukcNdirtDisplay, (Atom)NULL, (Atom)NULL,
   (Window)NULL, (Display *)NULL, (Window)NULL, (Display *)NULL, (Cardinal)0,
   (Atom)NULL },
};

static Cardinal number_of_invitations = XtNumber(_inviting_clients);
static int (*old_error_handler)();
static void (*old_warning)();
static void (*old_msg_warning)();
static Boolean error_happened = FALSE;

#define IC _inviting_clients[invite_num]

Boolean XukcIsClientWindow();

/*** internal routines ***/

static void
ignore_msg_warnings(name, type, class, default_m, params, num_params)
String name, type, class, default_m, *params;
Cardinal *num_params;
{
	error_happened = TRUE;
}


static void
ignore_warnings(warning)
String warning;
{
	error_happened = TRUE;
}


static int
ignore_errors(dpy, errorp)
Display *dpy;
XErrorEvent *errorp;
{
	error_happened = TRUE;
	return 0;
}


static void
delete_window_property(dpy, win, prop)
Display *dpy;
Window win;
Atom prop;
{
	old_error_handler = XSetErrorHandler(ignore_errors);

	XDeleteProperty(dpy, win, prop);
	XSync(dpy, False);

	(void)XSetErrorHandler(old_error_handler);
	error_happened = FALSE;
}


static Boolean
get_window_property(dpy, win, prop, long_offset, long_length, delete,
			req_type, actual_type_return, actual_format_return,
			nitems_return, bytes_after_return, prop_return)
Display *dpy;
Window win;
Atom prop;
long long_offset, long_length;
Bool delete;
Atom req_type;
Atom *actual_type_return;
int *actual_format_return;
unsigned long *nitems_return, *bytes_after_return;
unsigned char **prop_return;
{
	int status;

	error_happened = FALSE;
	old_error_handler = XSetErrorHandler(ignore_errors);

	status = XGetWindowProperty(dpy, win, prop, long_offset, long_length,
			delete, req_type, actual_type_return,
			actual_format_return, nitems_return,
			bytes_after_return, prop_return);

	(void)XSetErrorHandler(old_error_handler);
	status = (status == Success) && !error_happened;
	error_happened = FALSE;
	return ((Boolean)status);
}


static Boolean
send_property_if_clear(client_num, comms_window, str, len)
Cardinal client_num;
Window comms_window;
String str;
int len;
{
	Boolean ok;
	Atom actual_type;
	int actual_format;
	unsigned long nitems_return, bytes_after_return;
	unsigned char *prop_return;

	ok = get_window_property(PARTY.party_dpy, comms_window,
			PARTY.MASTER_DATA_PROPERTY, 0l, 1l, FALSE,
			AnyPropertyType, &actual_type, &actual_format,
			&nitems_return, &bytes_after_return, &prop_return);
	if (ok && actual_type != None)
		/* the property already exists so we must sit and wait
		 * for it to be deleted */
		return (FALSE);

	error_happened = FALSE;
	old_error_handler = XSetErrorHandler(ignore_errors);

	XChangeProperty(PARTY.party_dpy, comms_window,
				PARTY.MASTER_DATA_PROPERTY, XA_STRING, 8,
				PropModeReplace, (unsigned char *)str, 
				len);

	(void)XSetErrorHandler(old_error_handler);
	return (!error_happened);
}


static Boolean
convert_and_store(w, from_type, from, to_type, to)
Widget w;
String from_type, to_type;
XrmValuePtr from, to;
{
	Boolean status;

	error_happened = FALSE;
	old_error_handler = XSetErrorHandler(ignore_errors);
	old_msg_warning = XtAppSetWarningMsgHandler(
						XtWidgetToApplicationContext(w),
						ignore_msg_warnings);
	old_warning = XtAppSetWarningHandler(XtWidgetToApplicationContext(w),
						ignore_warnings);
	status = XtConvertAndStore(w, from_type, from, to_type, to);
	(void)XSetErrorHandler(old_error_handler);
	(void)XtAppSetWarningMsgHandler(XtWidgetToApplicationContext(w),
						old_msg_warning);
	(void)XtAppSetWarningHandler(XtWidgetToApplicationContext(w),
						old_warning);
	status &= !error_happened;
	error_happened = FALSE;
	return (status);
}


static Boolean
set_and_check_value(w, resource_type, resource_name, new_value, size)
Widget w;
String resource_type, resource_name, new_value;
int size;
{
	String oldval[256];

	error_happened = FALSE;
	old_error_handler = XSetErrorHandler(ignore_errors);
	old_msg_warning = XtAppSetWarningMsgHandler(
						XtWidgetToApplicationContext(w),
						ignore_msg_warnings);
	old_warning = XtAppSetWarningHandler(XtWidgetToApplicationContext(w),
						ignore_warnings);
	XtVaGetValues(w, resource_name, oldval, NULL);

	if (IS_STRING_TYPE(resource_type)) {
		if (strcmp(new_value, *(String *)oldval) != 0) {
			XtVaSetValues(w, resource_name, new_value, NULL);
		}
	} else if (bcmp(oldval, new_value, size) != 0) {
		Arg args[1];

		XukcCopyToArgVal(new_value, &args[0].value, size);
		args[0].name = resource_name;
		XtSetValues(w, (ArgList)args, 1);
	}
	(void)XSetErrorHandler(old_error_handler);
	(void)XtAppSetWarningMsgHandler(XtWidgetToApplicationContext(w),
						old_msg_warning);
	(void)XtAppSetWarningHandler(XtWidgetToApplicationContext(w),
						old_warning);
	return (error_happened);
}


static String
build_hierarchy_list(w, str, size, used, level)
Widget w;
String str;
Cardinal *size, *used;
Cardinal level;
{
#define CHUNK_SIZE 300
	char tmp[CHUNK_SIZE];
	Cardinal i;
	Position x, y;
	Dimension width, height, border_width;
	Boolean is_widget = XtIsWidget(w);

	if (is_widget || XtIsRectObj(w)) {
		XtVaGetValues(w, XtNx, &x, XtNy, &y, XtNwidth, &width,
				XtNheight, &height,
				XtNborderWidth, &border_width, NULL);
	} else {
		x = y = 0;
		width = height = border_width = 0;
	}

	(void)sprintf(tmp, "%s\n%d\n%c\n%hd %hd %hd %hd %hd %hd %s\n",
			XtName(w), w, 
			XtIsManaged(w) + '0', level,
			x, y, width, height, border_width,
			XukcClassName(XtClass(w)));

	while (strlen(tmp) + *used > *size) {
		if (*size == 0) {
			str = XtMalloc(CHUNK_SIZE);
			*str = '\0';
		} else
			str = XtRealloc(str, *size + CHUNK_SIZE);
		*size += CHUNK_SIZE;
	};
	*used += strlen(tmp);
	(void)strcat(str, tmp);

	if (is_widget) {
		if (XtIsComposite(w)) {
			CompositeWidget pw;

			pw = (CompositeWidget)w;

			for (i = 0; i < pw->composite.num_children; i++)
				str = build_hierarchy_list(
						   pw->composite.children[i],
						   str, size, used, level + 1);
		}

		for (i = 0; i < w->core.num_popups; i++)
			str = build_hierarchy_list(w->core.popup_list[i],
						   str, size,
						   used, level + 1);
	}
	return str;
}

/*** comms routines ***/

static void
intern_comm_atoms(invite_num)
Cardinal invite_num;
{
	char tmp[50];

	/* communication atoms that define the ICRM protocol */
	IC.atoms.ACCEPTANCE = XInternAtom(IC.inviting_dpy,
					   XukcNicrmAcceptance, FALSE);
	IC.atoms.HIERARCHY_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmHierarchyRequest, FALSE);
	IC.atoms.HIERARCHY_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmHierarchyReturn, FALSE);
	IC.atoms.NON_HIERARCHY_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmNonHierarchyRequest, FALSE);
	IC.atoms.NON_HIERARCHY_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmNonHierarchyReturn, FALSE);
	IC.atoms.GET_RESOURCES_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetResourcesRequest, FALSE);
	IC.atoms.GET_RESOURCES_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetResourcesReturn, FALSE);
	IC.atoms.GET_CLASS_RESOURCES_REQUEST = XInternAtom(IC.inviting_dpy,
				   XukcNicrmGetClassResourcesRequest, FALSE);
	IC.atoms.GET_CLASS_RESOURCES_RETURN = XInternAtom(IC.inviting_dpy,
				   XukcNicrmGetClassResourcesReturn, FALSE);
	IC.atoms.GET_VALUES_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetValuesRequest, FALSE);
	IC.atoms.GET_VALUES_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetValuesReturn, FALSE);
	IC.atoms.SET_VALUES_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmSetValuesRequest, FALSE);
	IC.atoms.SET_VALUES_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmSetValuesReturn, FALSE);
	IC.atoms.WINDOW_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmWindowRequest, FALSE);
	IC.atoms.WINDOW_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmWindowReturn, FALSE);
	IC.atoms.SET_PROPERTY_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmSetPropertyRequest, FALSE);
	IC.atoms.SET_PROPERTY_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmSetPropertyReturn, FALSE);
	IC.atoms.GET_PROPERTY_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetPropertyRequest, FALSE);
	IC.atoms.GET_PROPERTY_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetPropertyReturn, FALSE);
	IC.atoms.DROPPED_CONNECTION = XInternAtom(IC.inviting_dpy,
					   XukcNicrmDroppedConnection, FALSE);
	IC.atoms.GET_ACTION_LIST_REQUEST = XInternAtom(IC.inviting_dpy,
				   XukcNicrmGetActionListRequest, FALSE);
	IC.atoms.GET_ACTION_LIST_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmGetActionListReturn, FALSE);
	IC.atoms.CONVERSION_REQUEST = XInternAtom(IC.inviting_dpy,
					   XukcNicrmConversionRequest, FALSE);
	IC.atoms.CONVERSION_RETURN = XInternAtom(IC.inviting_dpy,
					   XukcNicrmConversionReturn, FALSE);
	(void)sprintf(tmp, XukcNicrmSlaveDataProperty, invite_num);
        IC.SLAVE_DATA_PROPERTY = XInternAtom(IC.my_display, tmp, FALSE);
}


static void
intern_message_atoms(client_num)
Cardinal client_num;
{
	char tmp[50];
	Display *dpy = XtDisplay(PARTY.w);

	PARTY.PARTY_WINDOW = XInternAtom(PARTY.party_dpy,
					PARTY.party_window_name, FALSE);
	PARTY.PARTY_DISPLAY = XInternAtom(PARTY.party_dpy,
					PARTY.party_display_name, FALSE);
	(void)sprintf(tmp, XukcNicrmMasterDataProperty, client_num);
        PARTY.MASTER_DATA_PROPERTY = XInternAtom(PARTY.party_dpy, tmp, FALSE);

	/* communication atoms that define the ICRM protocol */

	PARTY.atoms.ACCEPTANCE = XInternAtom(dpy, XukcNicrmAcceptance, FALSE);
	PARTY.atoms.HIERARCHY_REQUEST = XInternAtom(dpy,
					   XukcNicrmHierarchyRequest, FALSE);
	PARTY.atoms.HIERARCHY_RETURN = XInternAtom(dpy,
					   XukcNicrmHierarchyReturn, FALSE);
	PARTY.atoms.NON_HIERARCHY_REQUEST = XInternAtom(dpy,
					   XukcNicrmNonHierarchyRequest, FALSE);
	PARTY.atoms.NON_HIERARCHY_RETURN = XInternAtom(dpy,
					   XukcNicrmNonHierarchyReturn, FALSE);
	PARTY.atoms.GET_RESOURCES_REQUEST = XInternAtom(dpy,
					   XukcNicrmGetResourcesRequest, FALSE);
	PARTY.atoms.GET_RESOURCES_RETURN = XInternAtom(dpy,
					   XukcNicrmGetResourcesReturn, FALSE);
	PARTY.atoms.GET_CLASS_RESOURCES_REQUEST = XInternAtom(dpy,
				   XukcNicrmGetClassResourcesRequest, FALSE);
	PARTY.atoms.GET_CLASS_RESOURCES_RETURN = XInternAtom(dpy,
				   XukcNicrmGetClassResourcesReturn, FALSE);
	PARTY.atoms.GET_VALUES_REQUEST = XInternAtom(dpy,
					   XukcNicrmGetValuesRequest, FALSE);
	PARTY.atoms.GET_VALUES_RETURN = XInternAtom(dpy,
					   XukcNicrmGetValuesReturn, FALSE);
	PARTY.atoms.SET_VALUES_REQUEST = XInternAtom(dpy,
					   XukcNicrmSetValuesRequest, FALSE);
	PARTY.atoms.SET_VALUES_RETURN = XInternAtom(dpy,
					   XukcNicrmSetValuesReturn, FALSE);
	PARTY.atoms.WINDOW_REQUEST = XInternAtom(dpy,
					   XukcNicrmWindowRequest, FALSE);
	PARTY.atoms.WINDOW_RETURN = XInternAtom(dpy,
					   XukcNicrmWindowReturn, FALSE);
	PARTY.atoms.SET_PROPERTY_REQUEST = XInternAtom(dpy,
					   XukcNicrmSetPropertyRequest, FALSE);
	PARTY.atoms.SET_PROPERTY_RETURN = XInternAtom(dpy,
					   XukcNicrmSetPropertyReturn, FALSE);
	PARTY.atoms.GET_PROPERTY_REQUEST = XInternAtom(dpy,
					   XukcNicrmGetPropertyRequest, FALSE);
	PARTY.atoms.GET_PROPERTY_RETURN = XInternAtom(dpy,
					   XukcNicrmGetPropertyReturn, FALSE);
	PARTY.atoms.DROPPED_CONNECTION = XInternAtom(dpy,
					   XukcNicrmDroppedConnection, FALSE);
	PARTY.atoms.GET_ACTION_LIST_REQUEST = XInternAtom(dpy,
				   XukcNicrmGetActionListRequest, FALSE);
	PARTY.atoms.GET_ACTION_LIST_RETURN = XInternAtom(dpy,
					   XukcNicrmGetActionListReturn, FALSE);
	PARTY.atoms.CONVERSION_REQUEST = XInternAtom(dpy,
					   XukcNicrmConversionRequest, FALSE);
	PARTY.atoms.CONVERSION_RETURN = XInternAtom(dpy,
					   XukcNicrmConversionReturn, FALSE);
}


static int
error_handler(dpy, errorp)
Display *dpy;
XErrorEvent *errorp;
{
	switch (errorp->request_code) {
		case X_GetGeometry:
			if (errorp->error_code == BadDrawable)
				break;
		case X_SendEvent:
			if (errorp->error_code == BadWindow)
				break;
		default:
			(*old_error_handler)(dpy, errorp);
			break;
	}
	return 0;
}


static Boolean
send_message(dpy, window, type, d1, d2, d3, d4, d5)
Display *dpy;
Window window;
Atom type;
long d1, d2, d3, d4, d5;
{
	XClientMessageEvent event;
	Status error;

	event.display = dpy;
	event.window = window;
	event.type = ClientMessage;
	event.format = 32;
	event.message_type = type;
	event.data.l[0] = d1;
	event.data.l[1] = d2;
	event.data.l[2] = d3;
	event.data.l[3] = d4;
	event.data.l[4] = d5;

	old_error_handler = XSetErrorHandler(error_handler);
	error = XSendEvent(event.display, event.window, False,
					(EventMask)0L, (XEvent *)&event);
	(void)XSetErrorHandler(old_error_handler);
	XFlush(dpy);
#ifdef DEBUG
printf("%sSend ClientMessage to %d on %s\n", error ? "" : "Failed to ",
					window, dpy->display_name);
#endif DEBUG
	return (error);
}


static Boolean
check_window_validity(dpy, window, look_for_party_holder)
Display *dpy;
Window window;
Boolean look_for_party_holder;
{
	Status ok;
	Window dummy;	/* dummy return variables */
	int x, y;
	unsigned int w, h, bw, dp;

	/* this function is only used to ask the server whether the window
	 * still exists or not. */
	old_error_handler = XSetErrorHandler(error_handler);
	ok = XGetGeometry(dpy, (Drawable)window, &dummy, &x, &y,
							&w, &h, &bw, &dp);
	(void)XSetErrorHandler(old_error_handler);

	if (ok) {
		/* then check what type of window it is .. look for a
		 * particular property having been set */
		Atom party_holder, party_goer;
		Atom type;
		int format;
		unsigned long nitems, left;
		Window *retwin;

		party_goer = XInternAtom(dpy, XtNicrmPartyGoer, FALSE);
		party_holder = XInternAtom(dpy, XtNicrmPartyHolder, FALSE);

		if (get_window_property(dpy, window,
		    look_for_party_holder ? party_holder : party_goer,
		    0, 4, FALSE, XA_WINDOW, &type, &format, &nitems, &left,
		    (unsigned char **)&retwin) && type == XA_WINDOW) {
			if (*retwin != window)
				ok = 0;
			XtFree((char *)retwin);
		} else
			ok = 0;
	}
	return (ok);
}


static void
kill_party_properties(dpy, window, prop1, prop2)
Display *dpy;
Window window;
Atom prop1, prop2;
{
#ifdef DEBUG
printf("Killing Party Invitation on %s\n", XDisplayString(dpy));
#endif /* DEBUG */
	delete_window_property(dpy, DefaultRootWindow(dpy), prop1);
	delete_window_property(dpy, DefaultRootWindow(dpy), prop2);
	XFlush(dpy);
}

/************** Slave side of communications protocol *******************/

static void
drop_connection(invite_num)
Cardinal invite_num;
{
	if (IC.inviting_client_window == NULL)
		return;

	if (check_window_validity(IC.inviting_dpy, IC.inviting_client_window,
	    TRUE))
		(void)send_message(IC.inviting_dpy, IC.inviting_client_window,
			IC.atoms.DROPPED_CONNECTION, (long)IC.my_window,
			(long)NULL, (long)NULL, (long)NULL, (long)NULL);
	delete_window_property(IC.my_display, IC.my_window,
						IC.SLAVE_DATA_PROPERTY);
	/* clear out array */
	IC.inviting_client_window = NULL;
	IC.inviting_dpy = NULL;
	IC.my_window = NULL;
	IC.my_display = NULL;
}


static void
dying_client(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Cardinal invite_num;

	for (invite_num = 0; invite_num < number_of_invitations; invite_num++)
		drop_connection(invite_num);
}


/* send an "i am here" message to any client who has placed an "invitation"
 * property on the root window .. the different "invitation" properties
 * must have been set up in the list of atoms _inviting_clients[] */
static Boolean
inform_interested_party(invite_num, my_window)
Cardinal invite_num;
Window my_window;
{
#ifdef DEBUG
printf("Sending ACCEPTENCE message to %s\n\t\tat %d on %s : %d\n",
IC.client_window, IC.inviting_client_window, IC.inviting_dpy->display_name,
my_window);
#endif DEBUG
	return (send_message(IC.inviting_dpy, IC.inviting_client_window,
			IC.atoms.ACCEPTANCE, (long)my_window, (long)invite_num,
			(long)NULL, (long)NULL, (long)NULL));
}


static void
open_party_connection(dpy, root, my_window, invite_num, atom)
Display *dpy;
Window root;
Window my_window;
Cardinal invite_num;
Atom atom;
{
	Atom type;
	int format;
	unsigned long nitems, left;
	Window *retwin;
	unsigned char *retstr;

	/* look for the party invite property on the root_window of the
	 * display.  If it exists then open the associated display
	 * and try to accept the invite */
	IC.INVITING_CLIENT = XInternAtom(dpy, IC.client_window, FALSE);
	IC.INVITING_CLIENT_DISPLAY =
			XInternAtom(dpy, IC.client_display_name, FALSE);

	if (atom != NULL && atom != IC.INVITING_CLIENT)
		return; /* ignore a different property change */

	if (IC.inviting_dpy != NULL)
		XCloseDisplay(IC.inviting_dpy);

	IC.inviting_client_window = NULL;
	IC.inviting_dpy = NULL;

	if (get_window_property(dpy, root, IC.INVITING_CLIENT, 0, 4, FALSE,
	    XA_WINDOW, &type, &format, &nitems, &left,
	    (unsigned char **)&retwin) && type == XA_WINDOW) {
		IC.inviting_client_window = *retwin;
		XtFree((char *)retwin);
	}

	if (get_window_property(dpy, root, IC.INVITING_CLIENT_DISPLAY, 0, 1024,
	    FALSE, XA_STRING, &type, &format, &nitems, &left, &retstr)
	    && type == XA_STRING) {
		IC.inviting_dpy = XOpenDisplay((char *)retstr);
		XtFree((char *)retstr);
	}

	if (IC.inviting_client_window == NULL || IC.inviting_dpy == NULL) {
		/* failed to join in the party so ignore it */
		IC.inviting_client_window = NULL;
		IC.inviting_dpy = NULL;
	} else if (!check_window_validity(IC.inviting_dpy,
		   IC.inviting_client_window, TRUE)) {
		/* the party doesn't exist anymore .. probably died or
		 * lost its connection without telling anyone .. so
		 * lets be tidy and clear up the mess! */
		kill_party_properties(IC.inviting_dpy,
					IC.inviting_client_window,
					IC.INVITING_CLIENT_DISPLAY,
					IC.INVITING_CLIENT);
		IC.inviting_client_window = NULL;
		IC.inviting_dpy = NULL;
	} else {
		IC.my_window = my_window;
		IC.my_display = dpy;
		intern_comm_atoms(invite_num);
		delete_window_property(dpy, my_window, IC.SLAVE_DATA_PROPERTY);
		type = XInternAtom(dpy, XtNicrmPartyGoer, FALSE);
		XChangeProperty(dpy, my_window, type, XA_WINDOW, 32,
			PropModeReplace, (unsigned char *)&my_window, 1);

		if (!inform_interested_party(invite_num, my_window) &&
		    !check_window_validity(IC.inviting_dpy,
		    IC.inviting_client_window, TRUE)) {
		    /* failed to send message so check if party died and
		     * if so delete the properties to inform other partygoers
		     */
			kill_party_properties(IC.inviting_dpy,
					IC.inviting_client_window,
					IC.INVITING_CLIENT_DISPLAY,
					IC.INVITING_CLIENT);
			IC.inviting_client_window = NULL;
			IC.inviting_dpy = NULL;
		}
	}
}


static void
update_root_properties(dpy, root, my_window, event)
Display *dpy;
Window root;
Window my_window;
XEvent *event;
{
	Cardinal i;

	if (event->xproperty.state == PropertyNewValue) {
		for (i = 0; i < number_of_invitations; i++)
			open_party_connection(dpy, root, my_window, i,
						event->xproperty.atom);
	} else { /* PropertyDeleted */
		Cardinal invite_num = 0;

		for (; invite_num < number_of_invitations; invite_num++)
		    if (root == event->xproperty.window
		        && IC.INVITING_CLIENT == event->xproperty.atom) {
				/* was a connection really made? */
				if (IC.my_display != NULL &&
				    IC.my_window != NULL &&
				    IC.SLAVE_DATA_PROPERTY != NULL)
					delete_window_property(IC.my_display,
							IC.my_window,
							IC.SLAVE_DATA_PROPERTY);
				if (IC.inviting_dpy != NULL)
					XCloseDisplay(IC.inviting_dpy);
				IC.inviting_dpy = NULL;
				IC.inviting_client_window = NULL;
				break;
		    }
	}
}


static String
get_data_property(dpy, client_window, data_property, used)
Display *dpy;
Window client_window;
Atom data_property;
int *used;
{
#define DEFAULT_PACKET_SIZE 1024 /* number of 32-bits in data */
	Atom type;
	int nitems, format, size, left = 1, offset = 0;
	char *retstr = NULL;

	*used = 0;
	size = DEFAULT_PACKET_SIZE;

	while (data_property != NULL && left) {
		if (retstr != NULL)
			XtFree(retstr);

		/* retrieve the data and then delete the property which
		 * signals to the slave that the master is ready to
		 * receive more data.
		 */
		if (!get_window_property(dpy, client_window, data_property,
		    offset, size, TRUE, XA_STRING, &type, &format, &nitems,
		    &left, &retstr))
			return NULL; /* ||| unable to retrieve data! ||| */
		if (type != XA_STRING) /* ||| hmm, donno what this data is |||*/
			return NULL;
		size += left; 
	};

	*used = nitems;
	return retstr;
}


static void
send_data_to_client(invite_num, my_window, message, str, length)
Cardinal invite_num;
Window my_window;
Atom message;
String str;
Cardinal length;
{
	if (str != NULL)
		XChangeProperty(IC.inviting_dpy, my_window,
				IC.SLAVE_DATA_PROPERTY,
				XA_STRING, 8, PropModeReplace,
				(unsigned char *)str, length + 1);

	(void)send_message(IC.inviting_dpy, IC.inviting_client_window, message,
			(long)my_window,
			(long)(str != NULL ? IC.SLAVE_DATA_PROPERTY : NULL),
			(long)0, (long)0, (long)0);
}


static XtEventHandler
process_client_message(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
#define REQUEST_IS(m) (event->xclient.message_type == IC.atoms.m)
	Window my_window = XtWindow(w);
	Cardinal invite_num = event->xclient.data.l[0];

	if (invite_num > number_of_invitations)
		return;  /* garbled message so it can't be for us! */
	if (IC.inviting_dpy == NULL)
		return;  /* connection has been shut down so ignore it. */
#ifdef DEBUG
{
char *name = XGetAtomName(IC.inviting_dpy, event->xclient.message_type);
printf("Got a %s from %s\n", name, IC.client_window);
XFree(name);
}
#endif DEBUG
	if REQUEST_IS(HIERARCHY_REQUEST) {
		Cardinal size = 0, used = 0;
		String str;

		while (XtParent(w) != NULL)
			w = XtParent(w);
		str = build_hierarchy_list(w, (String)NULL, &size, &used, 0);

		send_data_to_client(invite_num, my_window,
					IC.atoms.HIERARCHY_RETURN, str, used);
		XtFree(str);
	} else if REQUEST_IS(NON_HIERARCHY_REQUEST) {
#if 0 /* ||| Not Yet Implemented ||| */
		send_data_to_client(invite_num, my_window,
				IC.atoms.NON_HIERARCHY_RETURN, str, used);
		XtFree(str);
#endif /* ||| */
	} else if REQUEST_IS(GET_RESOURCES_REQUEST) {
		char retstr[10000]; /* ||| should be dynamic ||| */
		Widget cw;
		Cardinal i, which_resources, num_res, num_cres;
		XtResourceList list, clist;
		
		/* the widget to examine */
		cw = (Widget)event->xclient.data.l[1];

		/* return the normal resources and/or constraint resources? */
		which_resources = (Cardinal)event->xclient.data.l[2];

		/* check for silly missing data */
		if (cw == NULL || which_resources == NULL) {
			send_data_to_client(invite_num, my_window,
					IC.atoms.GET_RESOURCES_RETURN, NULL, 0);
			return;
		}

		list = clist = NULL;
		num_res = num_cres = 0;

		if (which_resources & CLASS_RESOURCES)
			XtGetResourceList(XtClass(cw), &list, &num_res);
		if ((which_resources & CONSTRAINT_RESOURCES) &&
		    (XtParent(cw) != NULL))
			XtGetConstraintResourceList(XtClass(cw), &clist,
								&num_cres);
		/* The resource lists are placed in a new string and
		 * sent back. */
		retstr[0] = '\0';
		for (i = 0; i < num_res; i++) {
			char new_res[100];

			(void)sprintf(new_res, "%s %s %s\001",
						list[i].resource_name,
						list[i].resource_class,
						list[i].resource_type);
			(void)strcat(retstr, new_res);
		}
		for (i = 0; i < num_cres; i++) {
			char new_res[100];

			(void)sprintf(new_res, "%s %s %s\001",
						clist[i].resource_name,
						clist[i].resource_class,
						clist[i].resource_type);
			(void)strcat(retstr, new_res);
		}
		XtFree((char *)list);
		XtFree((char *)clist);

		/* send back the list of resources */
		send_data_to_client(invite_num, my_window,
			IC.atoms.GET_RESOURCES_RETURN, retstr, strlen(retstr));
	} else if REQUEST_IS(GET_CLASS_RESOURCES_REQUEST) {
#if 0 /* ||| Not Yet Implemented : Returns the resources in an object class */
		char retstr[10000]; /* ||| should be dynamic ||| */
		Widget cw;
		String str, ptr, ttr;
		Arg args[1];
		Cardinal i, which_resources;
		int size;
		XtResourceList list, clist;
		Cardinal found, num_res, num_cres;
		
		/* the widget to examine */
		cw = (Widget)event->xclient.data.l[1];

		/* return the normal resources and/or constraint resources? */
		which_resources = (Cardinal)event->xclient.data.l[2];

		/* check for silly missing data */
		if (cw == NULL || which_resources == NULL) {
			send_data_to_client(invite_num, my_window,
					IC.atoms.GET_RESOURCES_RETURN, NULL, 0);
			return;
		}

		list = clist = NULL;
		num_res = num_cres = 0;

		if (which_resources & CLASS_RESOURCES)
			XtGetResourceList(XtClass(cw), &list, &num_res);
		if ((which_resources & CONSTRAINT_RESOURCES) &&
		    (XtParent(cw) != NULL))
			XtGetConstraintResourceList(XtClass(cw), &clist,
								&num_cres);
		/* The resource lists are placed in a new string and
		 * sent back. */
		retstr[0] = '\0';
		for (i = 0; i < num_res; i++) {
			char new_res[100];

			(void)sprintf(new_res, "%s %s %s\001",
						list[i].resource_name,
						list[i].resource_class,
						list[i].resource_type);
			(void)strcat(retstr, new_res);
		}
		for (i = 0; i < num_cres; i++) {
			char new_res[100];

			(void)sprintf(new_res, "%s %s %s\001",
						clist[i].resource_name,
						clist[i].resource_class,
						clist[i].resource_type);
			(void)strcat(retstr, new_res);
		}
		XtFree(list);
		XtFree(clist);

		/* send back the list of resources */
		send_data_to_client(invite_num, my_window,
				IC.atoms.GET_CLASSRESOURCES_RETURN,
				retstr, strlen(retstr));
#endif /* ||| */
	} else if REQUEST_IS(GET_VALUES_REQUEST) {
		char retstr[10000]; /* ||| should be dynamic ||| */
		Widget cw;
		String str, ptr, ttr;
		Arg args[1];
		Cardinal i, num_args;
		int size;
		XtResourceList list, clist = NULL;
		Cardinal num_res, num_cres = 0;
		
		cw = (Widget)event->xclient.data.l[1];
		num_args = (Cardinal)event->xclient.data.l[2];
		str = get_data_property(IC.inviting_dpy, my_window,
					(Atom)event->xclient.data.l[3], &size);

		/* check for silly missing data */
		if (cw == NULL || *str == '\0') {
			send_data_to_client(invite_num, my_window,
					IC.atoms.GET_VALUES_RETURN, NULL, 0);
			XtFree((char *)str);
			return;
		}

		XtGetResourceList(XtClass(cw), &list, &num_res);
		if (XtParent(cw) != NULL)
			XtGetConstraintResourceList(XtClass(cw), &clist,
								&num_cres);

		/* results of the GetValues are placed in a new string and
		 * sent back. */
		retstr[0] = '\0';
		for (ttr = ptr = str, i = 0; i < num_args; i++) {
			XtResourceList slist;
			Cardinal num_s;

			ttr = index(ptr, ':');
			if (ttr == NULL) {
				fprintf(stderr, "Fatal Corruption in ICRM(GetValues)\n");
				abort(1);
			}
			*ttr = '\0';

			/* look for this resource's type */
			slist = list;
			num_s = num_res;
			for (; num_s > 0; num_s--)
				if (strcmp(slist[num_s-1].resource_name,
					ptr) == 0) break;

			if (num_s == 0) {
				/* try for a constraint resource */
				slist = clist;
				for (num_s = num_cres; num_s > 0; num_s--)
				   if (strcmp(slist[num_s-1].resource_name,
						ptr) == 0) break;
			}

			if (num_s > 0) {
				char tmp[100];
				XrmValue from, to;

				from.size = slist[num_s-1].resource_size;
				XtSetArg(args[0], ptr, tmp);
				XtGetValues(cw, (ArgList)args, 1);

				if (!IS_STRING_TYPE(slist[num_s-1].resource_type)) {
					from.addr = tmp;

					to.addr = NULL;
					to.size = 0;

					if (convert_and_store(cw,
					    slist[num_s-1].resource_type, &from,
					    XtRString, &to))
						(void)strcat(retstr, to.addr);
				} else if (*(String *)tmp != NULL)
					(void)strcat(retstr, *(String *)tmp);
			}
			strcat(retstr, "\001");
			ptr = ttr + 1;
		}
		XtFree((char *)list);
		XtFree((char *)clist);
		XtFree(str);

		/* send back the values of the get_values */
		send_data_to_client(invite_num, my_window,
			IC.atoms.GET_VALUES_RETURN, retstr, strlen(retstr));
	} else if REQUEST_IS(SET_VALUES_REQUEST) {
		Widget cw;
		String str, ptr, ttr, rtr;
		XtResourceList list, clist = NULL;
		Cardinal num_res, num_cres = 0;
		Cardinal i, num_args;
		Boolean set_error = FALSE;
		int dummy;
		
		cw = (Widget)event->xclient.data.l[1];
		num_args = (Cardinal)event->xclient.data.l[2];
		str = get_data_property(IC.inviting_dpy, my_window,
					(Atom)event->xclient.data.l[3], &dummy);

		/* check for silly missing data */
		if (cw == NULL || *str == '\0') {
			/* nack - the set values failed */
			(void)send_message(IC.inviting_dpy,
				IC.inviting_client_window,
				IC.atoms.SET_VALUES_RETURN, (long)my_window,
				(long)FALSE, (long)0, (long)0, (long)0);
			XtFree(str);
			return;
		}

		XtGetResourceList(XtClass(cw), &list, &num_res);
		if (XtParent(cw) != NULL)
			XtGetConstraintResourceList(XtClass(cw), &clist,
								&num_cres);
		/* perform the SetValues */
		for (rtr = ttr = ptr = str, i = 0; i < num_args; i++) {
			XtResourceList slist;
			Arg args[2];
			Cardinal num_s;

			ttr = index(ptr, ':');
			if (ttr == NULL) {
				fprintf(stderr, "Fatal Corruption in ICRM(SetValues)\n");
				abort(1);
			}
			*ttr = '\0';
			ttr++;

			rtr = index(ttr, '\001');
			if (rtr == NULL) {
				fprintf(stderr, "Fatal Corruption in ICRM(SetValues)\n");
				abort(1);
			}
			*rtr = '\0';

			/* look for this resource's type */
			slist = list;
			for (num_s = num_res; num_s > 0; num_s--)
				if (strcmp(slist[num_s-1].resource_name,
					ptr) == 0) break;

			if (num_s == 0) {
				/* try for a constraint resource */
				slist = clist;
				num_s = num_cres;
				for (; num_s > 0; num_s--)
				   if (strcmp(slist[num_s-1].resource_name,
						ptr) == 0) break;
			}

			args[0].value = NULL;
			/* convert from the string to the right type */
			if (num_s > 0) {
				XrmValue from, to;

				from.size = strlen(ttr);
				from.addr = ttr;
				to.size = 0;
				to.addr = NULL;

				if (convert_and_store(cw, XtRString, &from,
					slist[num_s-1].resource_type, &to))
					set_error |= set_and_check_value(cw,
						slist[num_s-1].resource_type,
						ptr, to.addr,
						slist[num_s-1].resource_size);
				else
					set_error = TRUE;
			}
			ptr = rtr + 1;
		}
		XtFree((char *)list);
		XtFree((char *)clist);
		XtFree(str);

		/* acknowledge the set values */
		(void)send_message(IC.inviting_dpy, IC.inviting_client_window,
				IC.atoms.SET_VALUES_RETURN, (long)my_window,
				(long)!set_error, (long)0, (long)0, (long)0);
	} else if REQUEST_IS(WINDOW_REQUEST) {
		(void)send_message(IC.inviting_dpy, IC.inviting_client_window,
				IC.atoms.WINDOW_RETURN, (long)my_window,
				(long)XtWindowToWidget(IC.my_display,
					(Window)event->xclient.data.l[1]),
				(long)0, (long)0, (long)0);
	} else if REQUEST_IS(SET_PROPERTY_REQUEST) {
		String str;
		Cardinal i;

		str = get_data_property(IC.inviting_dpy, my_window,
					(Atom)event->xclient.data.l[1], &i);

		i = XukcFindObjectOnList(IC.my_property, (XtPointer)str);

		if (i == 0)
			i = XukcAddObjectToList(&IC.my_property, (XtPointer)str,
									FALSE);
		else {
			XtFree((char *)XukcGetObjectOnList(IC.my_property, i));
			i = XukcChangeObjectOnList(IC.my_property, i,
								(XtPointer)str);
		}

		(void)send_message(IC.inviting_dpy, IC.inviting_client_window,
				IC.atoms.SET_PROPERTY_RETURN, (long)my_window,
				(long)i, (long)0, (long)0, (long)0);
	} else if REQUEST_IS(GET_PROPERTY_REQUEST) {
		String str;

		str = (String)XukcGetObjectOnList(IC.my_property,
					(Cardinal)event->xclient.data.l[1]);
		send_data_to_client(invite_num, my_window,
				IC.atoms.GET_PROPERTY_RETURN,
				str, str == NULL ? 0 : strlen(str));
	} else if REQUEST_IS(GET_ACTION_LIST_REQUEST) {
		String *list, str;
		Cardinal i, k, size;
		Widget w;

		/* get a list of the actions available to this widget */
		i = 0;
		w = (Widget)event->xclient.data.l[1];
		if (w != NULL)
			list = XukcWidgetClassActionList(
					XtWidgetToApplicationContext(w),
					XtClass(w), &i);
		for (size = 0, k = i; k > 0; k--)
			size += strlen(list[k-1]);

		str = (String)XtMalloc(size + 1 + i);
		*str = '\0';
		for (k = i; k > 0; k--) {
			(void)strcat(str, list[k-1]);
			(void)strcat(str, "\n");
		}
		XtFree((char *)list);

		send_data_to_client(invite_num, my_window,
				IC.atoms.GET_ACTION_LIST_RETURN,
				str, str == NULL ? 0 : strlen(str));
		if (str != NULL)
			XtFree(str);
	} else if REQUEST_IS(CONVERSION_REQUEST) {
		Cardinal i;
		Widget w;
		XrmValue from;
		XrmValue to;
		String from_type, to_type, str, ttr;

		/* format of data property = from_type\nto_type\nfrom_value */
		str = get_data_property(IC.inviting_dpy, my_window,
					(Atom)event->xclient.data.l[1], &i);
		w = (Widget)event->xclient.data.l[2];
		from.size = (Cardinal)event->xclient.data.l[3];
		to.addr = NULL;
		to.size = 0;
		from_type = str;
		ttr = (String)index(from_type, '\n');
		if (ttr == NULL) {
			fprintf(stderr, "Fatal Corruption in ICRM(Conversion)\n");
			abort(1);
		}
		*ttr = '\0';
		to_type = ttr + 1;
		ttr = (String)index(to_type, '\n');
		if (ttr == NULL) {
			fprintf(stderr, "Fatal Corruption in ICRM(Conversion)\n");
			abort(1);
		}
		*ttr = '\0';
		from.addr = (XtPointer)XtMalloc(from.size);
		(void)memcpy(from.addr, ttr + 1, from.size);

		if (convert_and_store(w, from_type, &from, to_type, &to)) {
			/* succeeded with conversion */
			if (IS_STRING_TYPE(to_type))
				/* don't waste server bandwidth */
				to.size = strlen((char *)to.addr);

			send_data_to_client(invite_num, my_window,
						IC.atoms.CONVERSION_RETURN,
						(String)to.addr, to.size);
		} else
			send_data_to_client(invite_num, my_window,
						IC.atoms.CONVERSION_RETURN,
						(String)NULL, 0);
		XtFree(str);
		XtFree(from.addr);
	}
}


/************** Master side of communications protocol *******************/

static void
dying_party(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XukcFinishParty((Cardinal)client_data);
}


#ifdef DEBUG
static void
print_hierarchy(obj, level)
RemoteHierarchy obj;
Cardinal level;
{
	Cardinal i;

	if (obj == NULL)
		return;

	for (i = 0; i < level; i++)
		putchar(' ');
	printf("%s (%d) : %s\n", obj->remote_name, obj->remote_widget,
							obj->widget_class);
	print_hierarchy(obj->children, level+1);
	print_hierarchy(obj->next, level);
}
#endif DEBUG


static Boolean
queue_request(client_num, comms_window, request, data, length,
				cd1, cd2, cd3, cd4, arg1, arg2, call_proc)
Cardinal client_num;
Window comms_window;
Atom request;
String data;
Cardinal length;
long cd1, cd2, cd3, cd4;
XtPointer arg1, arg2;
XukcIcrmProc call_proc;
{
	Boolean ok = TRUE;
	Boolean send_now = FALSE;
	String tmp;
	Cardinal chk;
	long *msg;

	if (data != NULL && length != 0) {
		tmp = XtMalloc(length+1);
		bcopy((char *)data, (char *)tmp, length+1);
		msg = (long *)XtMalloc(sizeof(long) * 4);
		msg[0] = cd1;
		msg[1] = cd2;
		msg[2] = cd3;
		msg[3] = cd4;
		data = tmp;
	} else {
		msg = NULL;
		send_now = TRUE;
		length = 0;
	}

	if (!send_now)
		send_now = send_property_if_clear(client_num, comms_window,
							data, length);
	if (send_now)
		ok = send_message(PARTY.party_dpy, comms_window, request,
				(long)client_num,
				(long)cd1, (long)cd2, (long)cd3, (long)cd4);
	if (!ok) {
		if (length > 0) {
			XtFree(tmp);
			XtFree((char *)msg);
		}
		return (FALSE);
	}

	if (send_now) {
		chk = XukcAddObjectToList(&PARTY.requests_in_progress,
					(XtPointer)request, FALSE);
		if (chk != XukcAddObjectToList(&PARTY.requests_in_progress_on,
		    (XtPointer)comms_window, FALSE)
		    || chk != XukcAddObjectToList(&PARTY.requests_in_progress_args,
		    arg1, FALSE)
		    || chk != XukcAddObjectToList(&PARTY.requests_in_progress_num_args,
		    arg2, FALSE)
		    || chk != XukcAddObjectToList(&PARTY.requests_in_progress_callback,
		    (XtPointer)call_proc, FALSE))
			XtError("Bad internal lists in ICRM (send_next_request)");
		return (TRUE);
	}

	chk = XukcAddObjectToList(&PARTY.requests_pending, (XtPointer)request,
									FALSE);
	if (chk != XukcAddObjectToList(&PARTY.requests_pending_on,
	    (XtPointer)comms_window, FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_data, data,
	    FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_length,
	    length, FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_msg, msg,
	    FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_args, arg1,
	    FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_num_args,
	    arg2, FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_callback,
	    (XtPointer)call_proc, FALSE))
		XtError("Bad internal lists in ICRM (add_request_to_pending)");

	return (TRUE);
}


/* ||| Need to Add NON_HIERARCHY_REQUEST and GET_CLASS_RESOURCES_REQUEST ||| */
static void
process_result(client_num, message, status, retstr, arg1, arg2)
Cardinal client_num;
Atom message;
Boolean status;
String retstr;
XtPointer arg1, arg2;
{
	if (arg1 == NULL) /* strange? the program doesn't want any data! */
		XtError("Couldn't process an ICRM data transfer missing result variable address in request");

	/* process the result string and place suitable values in arg1
	 * and arg2 */
	if (message == PARTY.atoms.HIERARCHY_REQUEST) {
		/* arg1 is the address of a variable of type RemoteHierarchy
		 * which is a pointer to the struct _icrm_widget_hierarchy */
		Cardinal current_level, new_level;
		RemoteHierarchy head, prev, parent, child;
		RemoteHierarchy *pass_back;

		/* format of retstr is ...
		 * widget_name\nwidget_id\nmanaged\nlevel_number
		 * <space>x_pos<space>y_pos<space>width<space>height
		 * <space>border_width<space>className\n */
		head = prev = parent = NULL;
		current_level = 0;
		while (retstr != NULL && *retstr != '\0') {
			char tmp[1000];

			child = XtNew(RemoteHierarchyRec);
			child->next = child->prev = child->parent = NULL;

			/* read widget_name */
			if (*retstr == '\n') {
				child->remote_name = XtNewString("");
				retstr++;
			} else { if (sscanf(retstr, "%[^\n]\n", tmp) != 1)
				       XtError("garbled data in ICRM transfer");
				child->remote_name = XtNewString(tmp);
				retstr += strlen(tmp) + 1;
			}

			/* read widget_id and hierarchy level */
			if (sscanf(retstr, "%d\n%c\n%d %hd %hd %hd %hd %hd", 
			    &child->remote_widget, &child->managed, &new_level,
			    &child->x, &child->y, &child->width,
			    &child->height, &child->border_width) != 8)
				XtError("garbled data in ICRM transfer");
			child->managed -= '0';
			while (isdigit(*retstr) || isspace(*retstr) ||
			       *retstr == '-')
				retstr++;

			/* read widget class */
			if (sscanf(retstr, "%[^\n]\n", tmp) != 1)
				XtError("garbled data in ICRM transfer");
			child->widget_class = XtNewString(tmp);
			retstr += strlen(tmp) + 1;

			if (head == NULL) {
				head = child;
				child->prev = child->next = NULL;
				parent = NULL;
			} else {
				if (new_level == current_level) {
					/* child of the current parent */
					child->prev = prev;
					child->next = NULL;
					prev->next = child;
				} else if (new_level > current_level) {
					/* we have a child of a new parent */
					parent = prev;
					child->next = child->prev = prev = NULL;
					parent->children = child;
				} else { /* new_level < current_level */
					/* go back up tree to find parent */
					Cardinal i = current_level - new_level;

					for (; i > 0; i--)
						parent = parent->parent;
					prev = parent->children;
					while (prev->next != NULL)
						prev = prev->next;
					child->prev = prev;
					prev->next = child;
				}
			}
			child->parent = parent;
			prev = child;
			child->children = NULL;
			current_level = new_level;
		}
#if 0
print_hierarchy(head, 0);
#endif
		pass_back = (RemoteHierarchy *)arg1;
		*pass_back = head;
	} else if (message == PARTY.atoms.GET_RESOURCES_REQUEST) {
		String ttr, str, resources_str;
		Cardinal i, *ret_num_res;
		XukcResourceList *ret_resources;
		
		if (arg2 == NULL)
			XtError("Couldn't process an ICRM GetResources request, missing result variable");

		ret_num_res = (Cardinal *)arg2;
		ret_resources = (XukcResourceList *)arg1;

		if (retstr == NULL || *retstr == '\0' || !status) {
			/* no data was returned */
			*ret_resources = NULL;
			*ret_num_res = 0;
			return;
		}

		/* count the number of returned resources */
		resources_str = XtNewString(retstr); /* retstr gets freed */
		str = ttr = resources_str;
		*ret_num_res = 0;
		while ((str = index(ttr, '\001')) != NULL) {
			*ret_num_res++;
			ttr = str + 1;
		}

		*ret_resources = (XukcResourceList)XtMalloc(*ret_num_res *
						sizeof(XukcResourceListRec));
		str = ttr = resources_str;
		for (i = 0; i < *ret_num_res; i++) {
			String ptr;

			str = index(ttr, '\001');
			*str = '\0';
			ret_resources[i]->resource_name = ttr;
			ptr = index(ttr, ' ');
			*ptr = '\0';
			ret_resources[i]->resource_class = ptr + 1;
			ptr = index(ptr + 1, ' ');
			*ptr = '\0';
			ret_resources[i]->resource_type = ptr + 1;
			ttr = str + 1;
		}
	} else if (message == PARTY.atoms.GET_VALUES_REQUEST) {
		String ptr, ttr, str;
		ArgList args;
		Cardinal i, num_args;
		
		if (arg2 == NULL)
			XtError("Couldn't process an ICRM GetValues request, missing result variable");

		num_args = (Cardinal)arg2;
		args = (ArgList)arg1;

		if (retstr == NULL || *retstr == '\0' || !status) {
			/* no data was returned */
			for (i = 0; i > num_args; i++)
				if (args[i].value != NULL)
					bzero(args[i].value, sizeof(XtPointer));
			return;
		}

		for (str = ttr = ptr = retstr, i = 0; i < num_args; i++) {
			str = index(ttr, '\001');
			*str = '\0';

			if (*ttr != '\0') {
				if (args[i].value == NULL)
					args[i].value = (int)ttr;
				else
					bcopy(&ttr, args[i].value,
							sizeof(XtPointer));
			} else if (args[i].value != NULL)
					bzero(args[i].value, sizeof(XtPointer));
			ttr = str + 1;
		}
	} else if (message == PARTY.atoms.SET_VALUES_REQUEST) {
		Boolean *done = (Boolean *)arg1;
		*done = status;
	} else if (message == PARTY.atoms.WINDOW_REQUEST) {
		Widget *pass_back = (Widget *)arg1;
		*pass_back = (Widget)retstr;
	} else if (message == PARTY.atoms.GET_PROPERTY_REQUEST) {
		String *pass_back = (String *)arg1;
		*pass_back = (String)retstr;
	} else if (message == PARTY.atoms.SET_PROPERTY_REQUEST) {
		Cardinal *pass_back = (Cardinal *)arg1;
		*pass_back = (Cardinal)retstr;
	} else if (message == PARTY.atoms.GET_ACTION_LIST_REQUEST) {
		String **pass_back = (String **)arg1;
		Cardinal *num_actions = (Cardinal *)arg2;
		String ttr, str, *tmp;
		Cardinal i, actions_list;
		
		if (arg2 == NULL)
			XtError("Couldn't process an ICRM GetActions request, missing result variable");

		if (retstr == NULL || *retstr == '\0' || !status) {
			/* no data was returned */
			*pass_back = NULL;
			*num_actions = 0;
			return;
		}

		actions_list = 0;
		for (str = ttr = retstr, i = 0; *ttr; i++) {
			str = index(ttr, '\n');
			*str = '\0';

			(void)XukcAddObjectToList(&actions_list,
					(XtPointer)XtNewString(ttr), FALSE);
			ttr = str + 1;
		}
		*pass_back = (String *)XukcGetListPointer(actions_list,
								num_actions);
		tmp = (String *)XtMalloc(sizeof(String) * *num_actions);
		for (i = *num_actions; i > 0; i--)
			tmp[i-1] = (*pass_back)[i-1];
		*pass_back = tmp;
		
		XukcDestroyList(&actions_list, FALSE);
	} else if (message == PARTY.atoms.CONVERSION_REQUEST) {
		XtPointer *pass_back = (XtPointer *)arg1;
		if (status) {
			if ((int)arg2 == 0)
				/* result is a string : free it after use */
				*pass_back = (XtPointer)XtNewString(retstr);
			else
				bcopy(retstr, pass_back, (int)arg2);
		}
	}
}


static Cardinal
find_request_pending(client_num, request, window)
Cardinal client_num;
Atom request;
Window window;
{
	Cardinal rn;
	XtPointer am;
	Window win;

	if (request == NULL) /* find any requests pending on this window */
		return (Cardinal)XukcFindObjectOnList(
						PARTY.requests_pending_on,
						(XtPointer)window);

	rn = XukcFindObjectOnList(PARTY.requests_pending, (XtPointer)request);
	if (rn == 0)
	    return 0;
	win = (Window)XukcGetObjectOnList(PARTY.requests_pending_on, rn);
	while (win != NULL && win != window) {
	    rn++;
	    while ((am = XukcGetObjectOnList(PARTY.requests_pending, rn))
		!= NULL && am != (XtPointer)request)
			rn++;
	    win = (Window)XukcGetObjectOnList(PARTY.requests_pending_on, rn);
	}
	return (win == NULL ? 0 : rn);
}


static Cardinal
find_request_in_progress(client_num, request, window)
Cardinal client_num;
Atom request;
Window window;
{
	Cardinal rn;
	XtPointer am;
	Window win;

	if (request == NULL) /* find any requests in progress on this window */
		return (Cardinal)XukcFindObjectOnList(
						PARTY.requests_in_progress_on,
						(XtPointer)window);

	rn = XukcFindObjectOnList(PARTY.requests_in_progress, (XtPointer)request);
	if (rn == 0)
	    return 0;
	win = (Window)XukcGetObjectOnList(PARTY.requests_in_progress_on, rn);
	while (win != NULL && win != window) {
	    rn++;
	    while ((am = XukcGetObjectOnList(PARTY.requests_in_progress, rn))
		!= NULL && am != (XtPointer)request)
			rn++;
	    win = (Window)XukcGetObjectOnList(PARTY.requests_in_progress_on, rn);
	}
	return (win == NULL ? 0 : rn);
}


static void
send_next_request(client_num)
Cardinal client_num;
{
	Boolean ok;
	String data;
	Cardinal length;
	long *msg;
	Cardinal rn, num_requests;
	Window window;
	Atom request;

	if (PARTY.requests_pending == 0) /* no requests pending */
		return;

	(void)XukcGetListPointer(PARTY.requests_pending_data, &num_requests);

	for (rn = 1; rn <= num_requests; rn++) {
		length = (Cardinal)XukcGetObjectOnList(
					PARTY.requests_pending_length, rn);
		if (length > 0)	/* a pending request that needs to send data */
			break;
	}

	if (rn > num_requests)
		return;	/* no pending requests that require data transfer */

	data = (String)XukcGetObjectOnList(PARTY.requests_pending_data, rn);
	msg = (long *)XukcGetObjectOnList(PARTY.requests_pending_msg, rn);
	window = (Window)XukcGetObjectOnList(PARTY.requests_pending_on, rn);
	request = (Atom)XukcGetObjectOnList(PARTY.requests_pending, rn);

	ok = send_property_if_clear(client_num, window, data, length);
	ok |= send_message(PARTY.party_dpy, window, request,
				(long)client_num, (long)msg[0],
				(long)msg[1], (long)msg[2], (long)msg[3]);
	/* ||| needed ??? ||| */
	if (!ok)
		XtWarning("ICRM Warning: Lost a request");
	(void)move_pending_to_in_progress(client_num, rn, TRUE);
}


static Cardinal
move_pending_to_in_progress(client_num, rn, free_data)
Cardinal client_num, rn;
Boolean free_data;
{
	XtPointer temp;
	Cardinal ret;

	temp = XukcGetObjectOnList(PARTY.requests_pending, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending, rn);
	ret = XukcAddObjectToList(&PARTY.requests_in_progress, temp, FALSE);
	temp = XukcGetObjectOnList(PARTY.requests_pending_on, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending_on, rn);
	if (ret != XukcAddObjectToList(&PARTY.requests_in_progress_on, temp, FALSE))
		XtError("Bad internal lists in ICRM (move_pending_to_in_progress)");
	temp = XukcGetObjectOnList(PARTY.requests_pending_args, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending_args, rn);
	if (ret != XukcAddObjectToList(&PARTY.requests_in_progress_args, temp, FALSE))
		XtError("Bad internal lists in ICRM (move_pending_to_in_progress)");
	temp = XukcGetObjectOnList(PARTY.requests_pending_num_args, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending_num_args, rn);
	if (ret != XukcAddObjectToList(&PARTY.requests_in_progress_num_args, temp, FALSE))
		XtError("Bad internal lists in ICRM (move_pending_to_in_progress)");
	temp = XukcGetObjectOnList(PARTY.requests_pending_callback, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending_callback, rn);
	if (ret != XukcAddObjectToList(&PARTY.requests_in_progress_callback,
	    temp, FALSE))
		XtError("Bad internal lists in ICRM (move_pending_to_in_progress)");

	if (free_data) {
		temp = XukcGetObjectOnList(PARTY.requests_pending_data, rn);
		XtFree(temp);
		temp = XukcGetObjectOnList(PARTY.requests_pending_msg, rn);
		XtFree(temp);
	}
	XukcRemoveEntryOnList(&PARTY.requests_pending_data, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending_length, rn);
	XukcRemoveEntryOnList(&PARTY.requests_pending_msg, rn);
	return ret;
}


static void
add_request_to_pending(client_num, request, client_window, args, num_args,
								callback)
Cardinal client_num;
Atom request;
Window client_window;
XtPointer args, num_args, callback;
{
	Cardinal chk;

	chk = XukcAddObjectToList(&PARTY.requests_pending, (XtPointer)request,
									FALSE);
	if (chk != XukcAddObjectToList(&PARTY.requests_pending_on,
	    (XtPointer)client_window, FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_args, args,
	    FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_num_args,
	    num_args, FALSE)
	    || chk != XukcAddObjectToList(&PARTY.requests_pending_callback,
	    callback, FALSE))
		XtError("Bad internal lists in ICRM (add_request_to_pending)");
}


/* check to see if this packet is the last one in this transfer sequence.
 * if it is then call the registered callback to inform the application
 * the data is available.  Then all traces of the request are destroyed. 8-O
 */
static void
do_finished_transfer(client_num, rn, client_window, status, data_property)
Cardinal client_num, rn;
Window client_window;
Boolean status;
Atom data_property;
{
	XukcIcrmProc callback;
	Atom message;
	char *retstr = NULL;
	XtPointer arg1 = NULL, arg2 = NULL;
	int dummy;

	retstr = get_data_property(PARTY.party_dpy, client_window,
						data_property, &dummy);
	message = (Atom)XukcGetObjectOnList(PARTY.requests_in_progress, rn);
	arg1 = XukcGetObjectOnList(PARTY.requests_in_progress_args, rn);
	arg2 = XukcGetObjectOnList(PARTY.requests_in_progress_num_args, rn);
	process_result(client_num, message, status, retstr, arg1, arg2);

	callback = (XukcIcrmProc)XukcGetObjectOnList(
				PARTY.requests_in_progress_callback, rn);
	if (callback != NULL)
		(*callback)(client_num, client_window, status, arg1, arg2);

	XtFree(retstr);
	XukcRemoveEntryOnList(&PARTY.requests_in_progress, rn);
	XukcRemoveEntryOnList(&PARTY.requests_in_progress_on, rn);
	XukcRemoveEntryOnList(&PARTY.requests_in_progress_args, rn);
	XukcRemoveEntryOnList(&PARTY.requests_in_progress_num_args, rn);
	XukcRemoveEntryOnList(&PARTY.requests_in_progress_callback, rn);
}


static XtEventHandler
process_party_message(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
#define MESSAGE_IS(m) (event->xclient.message_type == PARTY.atoms.m)
#define CHECK_REQUEST(n) if (n == 0) { \
	XtAppWarning(XtWidgetToApplicationContext(w), \
			"Bad message received by ICRM"); \
	return; \
}
	Cardinal rn;
	Cardinal client_num = (Cardinal)client_data;
	Window client_window = (Window)event->xclient.data.l[0];
	Atom data_property = (Atom)event->xclient.data.l[1];

#ifdef DEBUG
{
char *name;
name = XGetAtomName(PARTY.party_dpy, event->xclient.message_type);
printf("Got %s from %d on %s : PartyGo'er\n", name, client_window,
			PARTY.party_dpy->display_name);
XFree(name);
}
#endif DEBUG
	if (!(MESSAGE_IS(ACCEPTANCE) || MESSAGE_IS(DROPPED_CONNECTION)
	      || MESSAGE_IS(CONVERSION_RETURN)
	      || MESSAGE_IS(GET_ACTION_LIST_RETURN)
	      || MESSAGE_IS(HIERARCHY_RETURN)
	      || MESSAGE_IS(NON_HIERARCHY_RETURN)
	      || MESSAGE_IS(WINDOW_RETURN)
	      || MESSAGE_IS(GET_RESOURCES_RETURN)
	      || MESSAGE_IS(GET_CLASS_RESOURCES_RETURN)
	      || MESSAGE_IS(GET_VALUES_RETURN)
	      || MESSAGE_IS(SET_VALUES_RETURN)))
		return; /* the message is not for us! */

	if MESSAGE_IS(ACCEPTANCE) {
		if (client_num != (Cardinal)event->xclient.data.l[1]) {
			XtAppWarning(XtWidgetToApplicationContext(w),
				"Garbled ACCEPTANCE message received by ICRM");
			return;
		}
		if (PARTY.rsvp_windows == 0 ||
		    !XukcFindObjectOnList(PARTY.rsvp_windows,
		    (XtPointer)client_window))
			XukcAddObjectToList(&PARTY.rsvp_windows,
					(XtPointer)client_window, FALSE);
	} else {
	if (PARTY.rsvp_windows == 0 ||
	    !XukcFindObjectOnList(PARTY.rsvp_windows, (XtPointer)client_window))
	{
		XtAppWarning(XtWidgetToApplicationContext(w),
			"Received Message from unknown client in ICRM");
		return;
	}

	if MESSAGE_IS(DROPPED_CONNECTION) {
		/* check each request in the pending and in progress queues and
		 * delete the requests from the queues and call any callback
		 * routines informing them that the connection was dropped and
		 * therefore the request is void.
		 */
		XukcRemoveObjectOnList(&PARTY.rsvp_windows,
					(XtPointer)client_window);
		while (rn = find_request_pending(client_num, (Atom)NULL,
		       client_window) > 0)
			move_pending_to_in_progress(client_num, rn, TRUE);
		while (rn = find_request_in_progress(client_num, (Atom)NULL,
		       client_window) > 0)
			do_finished_transfer(client_num, rn, client_window,
						FALSE, NULL);
	} else if MESSAGE_IS(HIERARCHY_RETURN) {
		rn = find_request_in_progress(client_num,
				PARTY.atoms.HIERARCHY_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(NON_HIERARCHY_RETURN) {
		rn = find_request_in_progress(client_num,
			PARTY.atoms.NON_HIERARCHY_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(GET_RESOURCES_RETURN) {
		rn = find_request_in_progress(client_num,
			PARTY.atoms.GET_RESOURCES_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(GET_CLASS_RESOURCES_RETURN) {
		rn = find_request_in_progress(client_num,
			PARTY.atoms.GET_CLASS_RESOURCES_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(GET_VALUES_RETURN) {
		rn = find_request_in_progress(client_num,
				PARTY.atoms.GET_VALUES_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(SET_VALUES_RETURN) {
		rn = find_request_in_progress(client_num,
				PARTY.atoms.SET_VALUES_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(Boolean)event->xclient.data.l[2],
					data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(GET_ACTION_LIST_RETURN) {
		rn = find_request_in_progress(client_num,
			PARTY.atoms.GET_ACTION_LIST_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(CONVERSION_RETURN) {
		rn = find_request_in_progress(client_num,
			PARTY.atoms.CONVERSION_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window,
					(data_property != NULL), data_property);
		send_next_request(client_num);
	} else if MESSAGE_IS(WINDOW_RETURN) {
		XukcIcrmProc callback;
		Atom message;
		XtPointer arg1 = NULL, arg2 = NULL;

		rn = find_request_in_progress(client_num,
				PARTY.atoms.WINDOW_REQUEST, client_window);
		CHECK_REQUEST(rn);
		arg1 = XukcGetObjectOnList(PARTY.requests_in_progress_args, rn);
		arg2 = XukcGetObjectOnList(PARTY.requests_in_progress_num_args, rn);
		message = (Atom)XukcGetObjectOnList(PARTY.requests_in_progress, rn);
		process_result(client_num, message, TRUE,
				(char *)event->xclient.data.l[1], arg1, arg2);
		callback = (XukcIcrmProc)XukcGetObjectOnList(
				PARTY.requests_in_progress_callback, rn);
		if (callback != NULL)
			(*callback)(client_num, client_window, TRUE, arg1, arg2);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_on, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_args, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_num_args, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_callback, rn);
		send_next_request(client_num);
	} else if MESSAGE_IS(SET_PROPERTY_RETURN) {
		XukcIcrmProc callback;
		Atom message;
		XtPointer arg1 = NULL, arg2 = NULL;

		rn = find_request_in_progress(client_num, 
			PARTY.atoms.SET_PROPERTY_REQUEST, client_window);
		CHECK_REQUEST(rn);
		message = (Atom)XukcGetObjectOnList(PARTY.requests_in_progress,
									rn);
		arg1 = XukcGetObjectOnList(PARTY.requests_in_progress_args, rn);
		arg2 = XukcGetObjectOnList(PARTY.requests_in_progress_num_args, rn);
		process_result(client_num, message, TRUE,
				(char *)event->xclient.data.l[1], arg1, arg2);
		callback = (XukcIcrmProc)XukcGetObjectOnList(
					PARTY.requests_in_progress_callback, rn);
		if (callback != NULL)
			(*callback)(client_num, client_window, TRUE, arg1, arg2);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_on, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_args, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_num_args, rn);
		XukcRemoveEntryOnList(&PARTY.requests_in_progress_callback, rn);
		send_next_request(client_num);
	} else if MESSAGE_IS(GET_PROPERTY_RETURN) {
		rn = find_request_in_progress(client_num, 
			PARTY.atoms.GET_PROPERTY_REQUEST, client_window);
		CHECK_REQUEST(rn);
		do_finished_transfer(client_num, rn, client_window, TRUE,
							data_property);
		send_next_request(client_num);
	} }
#undef MESSAGE_IS
#undef CHECK_REQUEST
}


/******* Module Exported Routines *******/


String
XukcAtomToRequest(invite_num, atom)
Cardinal invite_num;
Atom atom;
{
	if (invite_num > number_of_invitations)
		XtError("Invalid client type passed to XukcAtomToRequest()");
	if (atom == IC.atoms.HIERARCHY_REQUEST)
		return XukcNicrmHierarchyRequest;
	if (atom == IC.atoms.HIERARCHY_RETURN)
		return XukcNicrmHierarchyReturn;
	if (atom == IC.atoms.NON_HIERARCHY_REQUEST)
		return XukcNicrmNonHierarchyRequest;
	if (atom == IC.atoms.NON_HIERARCHY_RETURN)
		return XukcNicrmNonHierarchyReturn;
	if (atom == IC.atoms.GET_RESOURCES_REQUEST)
		return XukcNicrmGetResourcesRequest;
	if (atom == IC.atoms.GET_RESOURCES_RETURN)
		return XukcNicrmGetResourcesReturn;
	if (atom == IC.atoms.GET_CLASS_RESOURCES_REQUEST)
		return XukcNicrmGetClassResourcesRequest;
	if (atom == IC.atoms.GET_CLASS_RESOURCES_RETURN)
		return XukcNicrmGetClassResourcesReturn;
	if (atom == IC.atoms.GET_VALUES_REQUEST)
		return XukcNicrmGetValuesRequest;
	if (atom == IC.atoms.GET_VALUES_RETURN)
		return XukcNicrmGetValuesReturn;
	if (atom == IC.atoms.SET_VALUES_REQUEST)
		return XukcNicrmSetValuesRequest;
	if (atom == IC.atoms.SET_VALUES_RETURN)
		return XukcNicrmSetValuesReturn;
	if (atom == IC.atoms.WINDOW_REQUEST)
		return XukcNicrmWindowRequest;
	if (atom == IC.atoms.WINDOW_RETURN)
		return XukcNicrmWindowReturn;
	if (atom == IC.atoms.SET_PROPERTY_REQUEST)
		return XukcNicrmSetPropertyRequest;
	if (atom == IC.atoms.SET_PROPERTY_RETURN)
		return XukcNicrmSetPropertyReturn;
	if (atom == IC.atoms.GET_PROPERTY_REQUEST)
		return XukcNicrmGetPropertyRequest;
	if (atom == IC.atoms.GET_PROPERTY_RETURN)
		return XukcNicrmGetPropertyReturn;
	if (atom == IC.atoms.CONVERSION_REQUEST)
		return XukcNicrmConversionRequest;
	if (atom == IC.atoms.CONVERSION_RETURN)
		return XukcNicrmConversionReturn;
	if (atom == IC.atoms.GET_ACTION_LIST_REQUEST)
		return XukcNicrmGetActionListRequest;
	if (atom == IC.atoms.GET_ACTION_LIST_RETURN)
		return XukcNicrmGetActionListReturn;
	return NULL;
}


Boolean
XukcIsClientWindow(client_num, win)
Cardinal client_num;
Window win;
{
	Boolean ok = FALSE;

	if (client_num > number_of_invitations)
		XtWarning("Bad client number passed to XukcIsClientWindow()");
	else if (PARTY.rsvp_windows != 0) {
		ok = (Boolean)XukcFindObjectOnList(PARTY.rsvp_windows,
							(XtPointer)win);
		if (ok && !check_window_validity(PARTY.party_dpy, win, FALSE)) {
			/* the window doesn't exist anymore so generate a
			 * dying window event */
#if 0 /* ||| what the heck should we be doing here??? ||| */
			kill_party_properties(PARTY.party_dpy, win,
							PARTY.PARTY_DISPLAY,
							PARTY.PARTY_WINDOW);
#endif /* ||| */
			ok = FALSE;
		}
	}
	return (ok);
}


Cardinal
XukcClientWindows(client_num)
Cardinal client_num;
{
	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcClientWindows()");
	return (PARTY.rsvp_windows);
}

Boolean
XukcStillDoingTransfers(client_num, comms_window)
Cardinal client_num;
Window comms_window;
{
	Boolean ok = FALSE;

	if (client_num > number_of_invitations) {
		XtWarning("Bad client number passed to XukcStillDoingTransfers()");
		return FALSE;
	}

	if (PARTY.rsvp_windows != 0) {
		ok = (Boolean)XukcFindObjectOnList(PARTY.rsvp_windows,
						(XtPointer)comms_window);
		if (ok && !check_window_validity(PARTY.party_dpy, comms_window,
		    FALSE)) {
			/* window is no more .. abort any transfers that are
			 * mid-session: do this via a faked DROPPED_CONNECTION
			 * event.
			 */
			(void)send_message(PARTY.party_dpy, PARTY.party_window,
				PARTY.atoms.DROPPED_CONNECTION,
				(long)comms_window,
				(long)NULL, (long)NULL, (long)NULL, (long)NULL);
			return FALSE;
		}
	}
	if (!ok)
		return FALSE;

	/* window still exists .. look for active transfers or
	 * pending ones.  Return FALSE if the queues are empty.
	 */
	if (!find_request_in_progress(client_num, (Atom)NULL, comms_window) &&
	    !find_request_pending(client_num, (Atom)NULL, comms_window))
		return FALSE;
	return TRUE;
}


Boolean
XukcRemoteConversion(client_num, comms_window, remote_widget, from_type,
						from, to_type, to, c_proc)
Cardinal client_num;
Window comms_window;
Widget remote_widget;
String from_type;
XrmValuePtr from;
String to_type;
XrmValuePtr *to;
XukcIcrmProc c_proc;
{
	Boolean ok;
	String str, tmp;
	Cardinal i;
	XtPointer passback;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteConversion()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	str = (String)XtMalloc(strlen(from_type) + strlen(to_type) +
								from->size + 3);
	(void)sprintf(str, "%s\n%s\n", from_type, to_type);
	tmp = str + strlen(str);
	for (i = 0; i < from->size; i++, tmp++)
		*tmp = from->addr[i];

	if ((*to)->addr == NULL)
		/* no memory has been allocated for the converted value so we
		 * put the value into the structure itself */
		passback = (XtPointer)&((*to)->addr);
	else	/* converted value is placed in the memory pointed to by
		 * addr */
		passback = (*to)->addr;

	ok = queue_request(client_num, comms_window,
		PARTY.atoms.CONVERSION_REQUEST, str, tmp - str, 
		(long)PARTY.MASTER_DATA_PROPERTY, (long)remote_widget,
		(long)from->size, (long)0, passback,
		(XtPointer)(*to)->size, c_proc);
	XtFree(str);
	return (ok);
}


Boolean
XukcRemoteActionList(client_num, comms_window, remote_widget, action_list,
							num_actions, c_proc)
Cardinal client_num;
Window comms_window;
Widget remote_widget;
String **action_list;
Cardinal *num_actions;
XukcIcrmProc c_proc;
{
	Boolean ok;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteActionList()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.GET_ACTION_LIST_REQUEST, NULL, 0, 
			(long)remote_widget, (long)0, (long)0, (long)0,
			(XtPointer)action_list, (XtPointer)num_actions,
			c_proc);
	return (ok);
}


Boolean
XukcRemoteSetProperty(client_num, comms_window, data, num_ret, c_proc)
Cardinal client_num;
Window comms_window;
String data;
Cardinal *num_ret;
XukcIcrmProc c_proc;
{
	Boolean ok;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteWindowToWidget()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.SET_PROPERTY_REQUEST, data, strlen(data), 
			(long)PARTY.MASTER_DATA_PROPERTY,
			(long)0, (long)0, (long)0,
			(XtPointer)num_ret, (XtPointer)NULL,
			c_proc);
	return (ok);
}


Boolean
XukcRemoteGetProperty(client_num, comms_window, prop_num, c_proc)
Cardinal client_num;
Window comms_window;
Cardinal prop_num;
XukcIcrmProc c_proc;
{
	Boolean ok;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteWindowToWidget()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.GET_PROPERTY_REQUEST, NULL, 0, 
			(long)prop_num,
			(long)0, (long)0, (long)0,
			(XtPointer)NULL, (XtPointer)NULL,
			(XtPointer)c_proc);
	return (ok);
}


Boolean
XukcRemoteResourceList(client_num, comms_window, remote_widget,
				get_normal_resources, get_constraints,
				ret_list, ret_num_on_list, c_proc)
Cardinal client_num;
Window comms_window;
Widget remote_widget;
Boolean get_normal_resources;
Boolean get_constraints;
XukcResourceList *ret_list;
Cardinal *ret_num_on_list;
XukcIcrmProc c_proc;
{
	Boolean ok;
	Cardinal which_resources = NULL;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteResourceList()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	which_resources |= get_normal_resources ? CLASS_RESOURCES : NULL;
	which_resources |= get_constraints ? CONSTRAINT_RESOURCES : NULL;

	if (which_resources == NULL) {
		XtWarning("Must specify class and/or constraint resources in XukcRemoteResourceList() call");
		return FALSE;
	}

	if (ret_list == NULL || ret_num_on_list == NULL) {
		XtWarning("Missing return pointers in XukcRemoteResourceList() call");
		return FALSE;
	}

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.GET_RESOURCES_REQUEST, NULL, 0, 
			(long)remote_widget,
			(long)which_resources, (long)0, (long)0,
			(XtPointer)ret_list, (XtPointer)ret_num_on_list,
			c_proc);
	return (ok);
}


void
XukcFreeRemoteResourceList(list, free_contents)
XukcResourceList list;
Boolean free_contents;
{
	if (list == NULL)
		return;

	if (free_contents)
		XtFree(list->resource_name);
	XtFree((char *)list);
}


Boolean
XukcRemoteWindowToWidget(client_num, comms_window, remote_window, ret_w, c_proc)
Cardinal client_num;
Window comms_window;
Window remote_window;
Widget *ret_w;
XukcIcrmProc c_proc;
{
	Boolean ok;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteWindowToWidget()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.WINDOW_REQUEST, NULL, 0, 
			(long)remote_window,
			(long)0, (long)0, (long)0,
			(XtPointer)ret_w, (XtPointer)NULL,
			c_proc);
	return (ok);
}


Boolean
XukcRemoteSetValues(client_num, comms_window, remote_widget_id,
						arglist, num_args, c_proc)
Cardinal client_num;
Window comms_window;
Widget remote_widget_id;
ArgList arglist;
Cardinal num_args;
XukcIcrmProc c_proc;
{
	Boolean ok;
	char tmp[10000]; /* ||| silly big size ||| */
	Cardinal i, j, k;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteSetValues()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	
	tmp[0] = '\0';
	i = j = k = 0;
	for (; i < num_args && arglist[i].value != NULL; i++) {
		strcpy(tmp + j, (char *)arglist[i].name);
		j += strlen((char *)arglist[i].name);
		tmp[j++] = ':';
		strcpy(tmp + j, (char *)arglist[i].value);
		j += strlen((char *)arglist[i].value);
		tmp[j++] = '\001';
		k++;
	}

	tmp[j++] = '\0';

	if (k == 0)
		return (FALSE);

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.SET_VALUES_REQUEST, tmp, j, 
			(long)remote_widget_id,
			(long)k, (long)PARTY.MASTER_DATA_PROPERTY,
			(long)0,
			(XtPointer)arglist, (XtPointer)num_args,
			c_proc);
	return (ok);
}


Boolean
XukcRemoteGetValues(client_num, comms_window, remote_widget_id,
						arglist, num_args, c_proc)
Cardinal client_num;
Window comms_window;
Widget remote_widget_id;
ArgList arglist;
Cardinal num_args;
XukcIcrmProc c_proc;
{
	char tmp[10000]; /* ||| should be dynamic ||| */
	Cardinal i, j;
	Boolean ok;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcRemoteGetValues()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	for (tmp[0] = '\0', i = 0, j = 0; i < num_args; i++) {
		strcpy(tmp + j, arglist[i].name);
		j += strlen(arglist[i].name);
		tmp[j++] = ':';
	}

	tmp[j++] = '\0';
	ok = queue_request(client_num, comms_window,
			PARTY.atoms.GET_VALUES_REQUEST, tmp, j, 
			(long)remote_widget_id,
			(long)num_args, (long)PARTY.MASTER_DATA_PROPERTY,
			(long)0,
			(XtPointer)arglist, (XtPointer)num_args,
			c_proc);
	return (ok);
}


Boolean
XukcGetClientWidgetTree(client_num, comms_window, start, called_when_done)
Cardinal client_num;
Window comms_window;
RemoteHierarchy *start;
XukcIcrmProc called_when_done;
{
	Boolean ok;

	if (client_num > number_of_invitations)
		XtError("Bad client number passed to XukcGetClientWidgetTree()");
	if (!XukcIsClientWindow(client_num, comms_window))
		return FALSE;

	ok = queue_request(client_num, comms_window,
			PARTY.atoms.HIERARCHY_REQUEST, NULL, 0, 
			(long)0, (long)0, (long)0, (long)0,
			(XtPointer)start, (XtPointer)NULL,
			(XtPointer)called_when_done);
	return (ok);
}


void
XukcFreeRemoteHierarchy(hierarchy, free_contents)
RemoteHierarchy hierarchy;
Boolean free_contents;
{
	if (hierarchy == NULL)
		return;

	if (free_contents) {
		XtFree(hierarchy->remote_name);
		XtFree(hierarchy->widget_class);
	}
	XukcFreeRemoteHierarchy(hierarchy->children, free_contents);
	XukcFreeRemoteHierarchy(hierarchy->next, free_contents);
	XtFree((char *)hierarchy);
}


Boolean
XukcMailInvites(client_num, w, display_name)
Cardinal client_num;
Widget w;
String display_name;
{
	Atom type;
	int format;
	unsigned long nitems, left;
	Window *retwin;
	Display *dpy;

	if (client_num > XtNumber(partys)) {
		XtWarning("Bad client_num passed to XukcMailInvites()");
		return FALSE;
	}

	if (display_name == NULL)
		dpy = XtDisplay(w);
	else if ((dpy = XOpenDisplay(display_name)) == NULL)
		return FALSE;

	PARTY.party_window = XtWindow(w);
	PARTY.party_dpy = dpy;
	PARTY.w = w;
	intern_message_atoms(client_num);

	if (get_window_property(PARTY.party_dpy,
	    DefaultRootWindow(PARTY.party_dpy),
	    PARTY.PARTY_WINDOW, 0, 4, FALSE, XA_WINDOW, &type, &format,
	    &nitems, &left, (unsigned char **)&retwin)
	    && type == XA_WINDOW) {
	    /* the same type of party has already been set up on this
	     * display .. if it is us then re-send this request otherwise fail
	     */
		if (*retwin == PARTY.party_window) {
			XtRemoveEventHandler(w, NoEventMask, TRUE,
					(XtEventHandler)process_party_message,
					(XtPointer)client_num);
			XtRemoveCallback(w, XtNdestroyCallback, dying_party,
						(XtPointer)client_num);
		} else if (check_window_validity(dpy, *retwin, TRUE)) {
			XtFree((char *)retwin);
			return FALSE;
		} else
			kill_party_properties(PARTY.party_dpy,
						PARTY.party_window,
						PARTY.PARTY_DISPLAY,
						PARTY.PARTY_WINDOW);
	}

	XtFree((char *)retwin);
	XtAddEventHandler(w, NoEventMask, TRUE,
				(XtEventHandler)process_party_message,
				(XtPointer)client_num);
	XtAddCallback(w, XtNdestroyCallback, dying_party, (XtPointer)client_num);
	/* no party exists so send out the invites on this display */
	XChangeProperty(PARTY.party_dpy, DefaultRootWindow(PARTY.party_dpy),
			PARTY.PARTY_WINDOW, XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&PARTY.party_window, 1);

	XChangeProperty(PARTY.party_dpy, DefaultRootWindow(PARTY.party_dpy),
			PARTY.PARTY_DISPLAY, XA_STRING, 8, PropModeReplace,
			(unsigned char *)XtDisplay(PARTY.w)->display_name,
			strlen(XtDisplay(PARTY.w)->display_name));

	type = XInternAtom(PARTY.party_dpy, XtNicrmPartyHolder, FALSE);
	XChangeProperty(PARTY.party_dpy, PARTY.party_window,
			type, XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&PARTY.party_window, 1);
	XFlush(PARTY.party_dpy);
	return TRUE;
}


void
XukcFinishParty(client_num)
Cardinal client_num;
{
	if (client_num > XtNumber(partys)) {
		XtWarning("Bad client_num passed to XukcMailInvites()");
		return;
	}

	/* is there a party to finish? */
	if (PARTY.party_window != NULL) {
		Atom type;

		delete_window_property(PARTY.party_dpy,
				DefaultRootWindow(PARTY.party_dpy),
				PARTY.PARTY_WINDOW);
		delete_window_property(PARTY.party_dpy,
				DefaultRootWindow(PARTY.party_dpy),
				PARTY.PARTY_DISPLAY);

		type = XInternAtom(PARTY.party_dpy, XtNicrmPartyHolder, FALSE);
		delete_window_property(PARTY.party_dpy, PARTY.party_window,
								type);
		XtRemoveEventHandler(PARTY.w, NoEventMask, TRUE,
					(XtEventHandler)process_party_message,
					(XtPointer)client_num);
		XukcDestroyList(&PARTY.rsvp_windows, FALSE);
		XukcDestroyList(&PARTY.requests_pending, FALSE);
		XukcDestroyList(&PARTY.requests_pending_on, FALSE);
		/* ||| vvvv    should the contents be freed? ||| */
		XukcDestroyList(&PARTY.requests_pending_args, FALSE);
		XukcDestroyList(&PARTY.requests_pending_callback, FALSE);
		XukcDestroyList(&PARTY.requests_in_progress, FALSE);
		XukcDestroyList(&PARTY.requests_in_progress_on, FALSE);
		/* ||| vvvv    should the contents be freed? ||| */
		XukcDestroyList(&PARTY.requests_in_progress_args, FALSE);
		XukcDestroyList(&PARTY.requests_in_progress_callback, FALSE);

		XFlush(PARTY.party_dpy);
		if (XtDisplay(PARTY.w) != PARTY.party_dpy)
			XCloseDisplay(PARTY.party_dpy);
		PARTY.party_dpy = NULL;
		PARTY.party_window = NULL;
	}
}

/******* These routines override the Xt Toolkit defined ones *******/

void
XukcDestroyApplicationContext(app_context)
XtAppContext app_context;
{
	Cardinal invite_num;

	for (invite_num = 0; invite_num < number_of_invitations; invite_num++) {
		drop_connection(invite_num);
		XukcFinishParty(invite_num);
	}
	XtDestroyApplicationContext(app_context);
}


void
XukcConditionalLoop(client_num, condition)
Cardinal client_num;
Boolean *condition;
{
	XEvent event;
	Display *dpy;
	Window root, my_window;

	if ((dpy = XtDisplay(PARTY.w)) == NULL || condition == NULL ||
	    !*condition)
		return;

	root = DefaultRootWindow(dpy);
	my_window = PARTY.party_window;

#if 0
	/* we have our communications window so look for and accept any
	 * invitations to talk that have been placed in Properties on
	 * the root window of any displays linked into this application
	 * context. */
	if (my_window != NULL) {
		Cardinal i;

		XSelectInput(dpy, root, PropertyChangeMask);
		for (i = 0; i < number_of_invitations; i++)
			open_party_connection(dpy, root, my_window, i,
								(Atom)NULL);
	}
#endif

	/* This is a normal event loop except that changes in root
	 * window properties are caught and ClientMessage events sent
	 * to "my_window" are pulled out.  The two methods form the
	 * the basic mechanism to implement the RemoteGet/SetValues
	 * procedures.
	 */
	while (*condition) {
		XtAppNextEvent(XtDisplayToApplicationContext(dpy), &event);

		if (event.type == PropertyNotify
		    && event.xany.window == root)
			update_root_properties(dpy, root, my_window, &event);
		else
			XtDispatchEvent(&event);
	}
}


void
XukcAppMainLoop(app_context, w)
XtAppContext app_context;
Widget w;
{
	XEvent event;
	Display *dpy;
	Window root, my_window;

#if 0
	/* ||| this should really connect to ALL the displays in the
	 *     app_context.  But lets get it working for one first! ||| */
	dpy = app_context->list[0];
#endif 0
	dpy = XtDisplay(w);
	root = DefaultRootWindow(dpy);

	/* A simple window just to catch ClientMessage events behind
	 * the toolkit's back! */
	my_window = XtWindow(w);

#if 0 /* the program must do these before the loop */
	/* set all the resource type to String conversion routines */
	XukcAddConverters();
	XukcOverrideConverters();
#endif

	/* we have our communications window so look for and accept any
	 * invitations to talk that have been placed in Properties on
	 * the root window of any displays linked into this application
	 * context. */
	if (my_window != NULL) {
		Cardinal i;

		XSelectInput(dpy, root, PropertyChangeMask);
		XtAddEventHandler(w, NoEventMask, TRUE,
					(XtEventHandler)process_client_message,
					(XtPointer)NULL);
		XtAddCallback(w, XtNdestroyCallback, dying_client,
							(XtPointer)NULL);

		for (i = 0; i < number_of_invitations; i++)
			open_party_connection(dpy, root, my_window, i,
								(Atom)NULL);
	}

	/* This is a normal event loop except that changes in root
	 * window properties are caught and ClientMessage events sent
	 * to "my_window" are pulled out.  The two methods form the
	 * the basic mechanism to implement the RemoteGet/SetValues
	 * procedures.
	 */
	while (TRUE) {
		XtAppNextEvent(app_context, &event);

		if (event.type == PropertyNotify
		    && event.xany.window == root)
			update_root_properties(dpy, root, my_window, &event);
		else
			XtDispatchEvent(&event);
	}
}


void
XukcMainLoop(w)
Widget w;
{
	XukcAppMainLoop(XtWidgetToApplicationContext(w), w);
}
