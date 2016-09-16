#ifndef lint
static char sccsid[] = "@(#)stubs.c	1.2 (UKC) 5/10/92";
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

#include <X11/Xos.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/text.h>
#include "dirt_structures.h"

/* in Wc - Widget Creation library */
extern void WcRegisterCallback();
/* in dirt.c */
extern void Output();
/* in remote.c */
extern Boolean ObjectRemoveCallback();

extern ObjectPtr GetStartOfList();
extern XtAppContext GetUIContext();

extern Widget WI_action_stubs_entry, WI_action_stubs_list;
extern Widget WI_action_stubs_register;
extern Widget WI_callback_stubs_register, WI_callback_stubs_entry;
extern Widget WI_callback_stubs_list;

String WhichEvent();

static Cardinal action_names = 0;
static Cardinal callback_names = 0;


/* a registered action has been called so output a message for the time
 * being.
 */
static void
default_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	Output("Action-> called from \"%s\" object", 1, XtName(w));
	Output("\tEvent = %s", 1, WhichEvent(event));
	if (*num_params == 0)
		Output("\tNo parameters passed", 0);
	else {
		Cardinal i;

		Output("\tParameters (%d) = ", 1, *num_params);
		for (i = 0; i < *num_params; i++)
			Output("\t\t\"%s\"", 1, params[i]);
	}
}


static void
default_callback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	struct callback_data *cd = (struct callback_data *)client_data;

	Output("Callback-> %s(%s) from \"%s\" object\n\tcall_data = \"0x%x\" (Hex) \"%d\" (Decimal)", 5,
		cd->name, cd->client_data == NULL ? "NULL" : cd->client_data,
		XtName(w), call_data == NULL ? "(NULL)" : call_data,
		call_data);
}


static int
compare_names(a, b)
String a, b;
{
	return (strcmp(*(String *)a, *(String *)b));
}


static void
create_action_list()
{
	String *actions;
	Cardinal num_actions;

	if (action_names == 0)
		return; /* nothing to display */

	actions = (String *)XukcGetListPointer(action_names, &num_actions);
	/* sort the action names */
	qsort(actions, num_actions, sizeof(String), compare_names);

	XukcScrListChange(WI_action_stubs_list, actions, num_actions);
}


static void
create_callback_list()
{
	String *callbacks;
	Cardinal num_callbacks;

	if (callback_names == 0)
		return; /* nothing to display */

	callbacks = (String *)XukcGetListPointer(callback_names,
							&num_callbacks);
	/* sort the callback names */
	qsort(callbacks, num_callbacks, sizeof(String), compare_names);

	XukcScrListChange(WI_callback_stubs_list, callbacks, num_callbacks);
}


/******************* Module Exported Routines *********************/


String *
CurrentlyRegisteredActions(num_actions)
Cardinal *num_actions;
{
	if (action_names == 0) {
		*num_actions = 0;
		return (NULL); /* no added actions */
	}

	return ((String *)XukcGetListPointer(action_names, num_actions));
}


/* clear the name lists ready for a new collection from a file */
void
ClearRegisteredNames()
{
	XukcDestroyList(&action_names, TRUE);
	XukcDestroyList(&callback_names, TRUE);
	XukcScrListChange(WI_action_stubs_list, (String *)NULL, 0);
	XukcScrListChange(WI_callback_stubs_list, (String *)NULL, 0);
}


/* save any names loaded from a spec. file */
void
RegisterName(app_context, name, is_action)
XtAppContext app_context;
String name;
Boolean is_action;
{
	if (name == NULL || *name == '\0')
		return; /* nothing to register */

	(void)XukcAddObjectToList((is_action ? &action_names : &callback_names),
					(XtPointer)XtNewString(name), FALSE);
	if (is_action) {
		static XtActionsRec action_rec[] = {
			{ (String)NULL, default_action }
		};
		action_rec[0].string = XtNewString(name);
		/* NB: does not deal with ICRM remote actions */
		XtAppAddActions(app_context, action_rec, 1);
	} else
		WcRegisterCallback(app_context, name, default_callback,
					(XtPointer)NULL);
}


/* display the names in a scrollable list */
void
DisplayRegisteredNames()
{
	create_action_list();
	create_callback_list();
}


Boolean
HasRegisteredNames(actions)
Boolean actions;
{
	return (actions ? action_names != 0 : callback_names != 0);
}


/* output the registered names as a C code string array used for binding to
 * real application routines.  By default they are bound to a warning message.
 */
void
PrintRegisteredNames(fd, is_action)
FILE *fd;
Boolean is_action;
{
	Cardinal list = is_action ? action_names : callback_names;
	Cardinal num_on_list;
	String *str;

	if (list == 0)
		return;

	str = (String *)XukcGetListPointer(list, &num_on_list);

	for (; num_on_list > 0; num_on_list--)
		fprintf(fd, "\t\"%s\",\n", str[num_on_list-1]);
}


/* save the registered names to a specification file.
 */
void
SaveRegisteredNames(fd, prefix, is_action)
FILE *fd;
String prefix;
Boolean is_action;
{
	Cardinal list = is_action ? action_names : callback_names;
	Cardinal num_on_list;
	String *str;

	if (list == 0)
		return;

	str = (String *)XukcGetListPointer(list, &num_on_list);

	for (; num_on_list > 0; num_on_list--)
		fprintf(fd, "%s %s\n", prefix, str[num_on_list-1]);
}


/* callback for Register button in ``Translation Actions'' popup */
void
RegisterAction(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	String name;

	if (GetStartOfList(TRUE) != NULL && GetStartOfList(TRUE)->is_remote)
		return; /* ||| cannot deal with remote action calls yet ||| */

	name = XukcGetTextWidgetString(WI_action_stubs_entry);
	if (*name == '\0')
		return; /* nothing to register */

	RegisterName(GetUIContext(), name, TRUE);
	create_action_list();
	XukcClearTextWidget(WI_action_stubs_entry);
}


void
DoRegisterAction(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	RegisterAction(w, (XtPointer)NULL, (XtPointer)NULL);
}


/* callback for Register button in ``Callback Routines'' popup */
void
RegisterCallback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	String name;

	if (GetStartOfList(TRUE) != NULL && GetStartOfList(TRUE)->is_remote)
		return; /* ||| cannot deal with remote callbacks yet ||| */

	name = XukcGetTextWidgetString(WI_callback_stubs_entry);
	if (*name == '\0')
		return; /* nothing to register */

	RegisterName(GetUIContext(), name, FALSE);
	create_callback_list();
	XukcClearTextWidget(WI_callback_stubs_entry);
}


void
DoRegisterCallback(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	RegisterCallback(w, (XtPointer)NULL, (XtPointer)NULL);
}


/* removes all the callbacks on the callback list that point to the
 * default_callback.
 */
void
RemoveDefaultCallbacks(obj, list_name, callbacks)
ObjectPtr obj;
String list_name;
XtCallbackList callbacks;
{
	while (callbacks && callbacks->callback != NULL) {
		if (callbacks->callback == (XtCallbackProc)default_callback)
			ObjectRemoveCallback(obj, list_name,
						default_callback,
						callbacks->closure);
		callbacks++;
	}
}


String
WhichEvent(event)
XEvent *event;
{
	/* Expand this to return the name of the event type */
	return ("(unknown)");
}
