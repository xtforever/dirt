#ifndef lint
static char sccsid[] = "@(#)remote.c	1.2 (UKC) 5/10/92";
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

#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/ICRM.h>
#include "dirt_structures.h"

extern Window CurrentCommsWindow();
extern XtResourceList GetAResource();
extern Boolean IsStringType();
extern String *XukcWidgetClassActionList();
extern void XukcDestroyList();
/* in dirt.c */
extern void DrainUIEventQueue();
/* in list.c */
extern WidgetClass GetClass();

extern Widget WI_main_layout;

/* Module-Wide Variables */

static Window comms_window = NULL;
static ObjectPtr current_object = NULL;
static Boolean blocking_condition = TRUE;
static Boolean transfer_status = FALSE;
static ArgList original_args = NULL;
static Cardinal free_list = 0;

/***** Remote GetValues and SetValues transfer blocking routines *****/

static void
convert_args(obj, from_args, to_args, num_args, to_string)
ObjectPtr obj;
ArgList from_args, to_args;
Cardinal num_args;
Boolean to_string;
{
	int i;
	XrmValue from, to;
	XtResourceList res;
	Boolean ok, is_constraint;

	for (; num_args > 0; num_args--) {
		if (!to_string) {
			if (to_args[num_args-1].value == NULL)
				GiveError(WI_main_layout, "badArgument",
					"nullArgumentData",
					"internal error: to_args[].value NULL in remote.c/convert_args()", 
					0);
		}
		res = GetAResource(obj, from_args[num_args-1].name, FALSE, &i,
					&is_constraint);
		if (i < 0) {
			GiveWarning(WI_main_layout, "badResource",
					"cannotFindResource",
					"Unknown resource \"%s\" in Object{Set,Get}Values(%s) call",
					2, from_args[num_args-1].name,
					obj->name);
			/* signal a conversion failure */
			if (to_args[num_args-1].value != NULL)
				bzero(to_args[num_args-1].value,
							res[i].resource_size);
			XtFree((char *)res);
			continue;
		}
		/* failed remote operation */
		if (!to_string && from_args[num_args-1].value == NULL) {
			/* signal a conversion failure */
			if (to_args[num_args-1].value != NULL)
				bzero(to_args[num_args-1].value,
							res[i].resource_size);
			XtFree((char *)res);
			continue;
		}
		if (!IsStringType(res[i].resource_type)) {
			if (to_string) {
				from.addr = (XtPointer)&(from_args[num_args-1].value);
				from.size = res[i].resource_size;
				to.addr = NULL;
				to.size = 0;
				ok = DoObjectConversion(obj,
						res[i].resource_type,
						&from, XtRString, &to);

				/* safety net */
				if (to_args[num_args-1].value != NULL &&
				    *(XtPointer *)from.addr !=
				    (XtPointer)to_args[num_args-1].value)
				      XtFree((char *)to_args[num_args-1].value);

				if (ok)
					to_args[num_args-1].value =
						(int)XtNewString(to.addr);
				else /* signal a conversion failure */
					to_args[num_args-1].value = NULL;
			} else {
				from.addr = (XtPointer)from_args[num_args-1].value;
				from.size = STRLEN(from.addr);
				to.addr = (XtPointer)to_args[num_args-1].value;
				to.size = res[i].resource_size;
				ok = DoObjectConversion(obj, XtRString, &from,
						res[i].resource_type, &to);
				if (!ok) /* signal a conversion failure */
					bzero(to_args[num_args-1].value,
							res[i].resource_size);
			}
		} else if (to_string) {
			/* safety net */
			if (to_args[num_args-1].value != NULL &&
			    to_args[num_args-1].value !=
			    from_args[num_args-1].value)
				XtFree((char *)to_args[num_args-1].value);

			to_args[num_args-1].value = (int)XtNewString(
					(String)from_args[num_args-1].value);
		} else {
			/* allocate a (String *) pointer and save its reference
			 * so we can free it later. */
			String *tmp = (String *)XtMalloc(sizeof(String));
			*tmp = XtNewString((String)from_args[num_args-1].value);
#if 0 /* ||| 8-{ we don't know when tmp is not wanted anymore 8-( ||| */
			(void)XukcAddObjectToList(&free_list, (XtPointer)*tmp,
								FALSE);
#endif /* ||| */
			to_args[num_args-1].value = (int)tmp;
		}
		XtFree((char *)res);
	}
}


static XukcIcrmProc
finished_ObjectGetValues(client_num, window, status, arg1, arg2)
Cardinal client_num;
Window window;
Boolean status;
XtPointer arg1, arg2;
{
	/* convert the arguments from Strings in to the correct types */
	convert_args(current_object, (ArgList)arg1, original_args,
						(Cardinal)arg2, FALSE);
	transfer_status = status;
	blocking_condition = FALSE;
}


static XukcIcrmProc
finished_blocked_transfer(client_num, window, status, arg1, arg2)
Cardinal client_num;
Window window;
Boolean status;
XtPointer arg1, arg2;
{
	transfer_status = status;
	blocking_condition = FALSE;
}


static Boolean
block_til_transfer_completed(client_num)
Cardinal client_num;
{
	blocking_condition = TRUE;
	XukcConditionalLoop(XukcDirtEditor, &blocking_condition);
	return (transfer_status);
}


/****** Module Exported Routines ******/

String *
ObjectGetActionList(obj, num_actions)
ObjectPtr obj;
Cardinal *num_actions;
{
	Cardinal ok, list;
	String *action_list;

	if (!obj->is_remote) {
		action_list = XukcWidgetClassActionList(
				XtWidgetToApplicationContext(obj->instance),
				GetClass(obj->class),
				num_actions);
		ok = 0;
		for (list = 0; list < *num_actions; list++) {
			/* remove dirt internal actions */
			if (strncmp("dirtACT", action_list[list], 7) == 0) {
				ok++;
				continue;
			}
			action_list[list-ok] = action_list[list];
		}
		*num_actions -= ok;
		return (action_list);
	}

	current_object = obj;
	comms_window = CurrentCommsWindow();
	transfer_status = FALSE;
	action_list = NULL;
	*num_actions = 0;
	
	ok = XukcRemoteActionList(XukcDirtEditor, comms_window, obj->instance,
				&action_list, num_actions,
				(XukcIcrmProc)finished_blocked_transfer);
	if (ok)
		ok = block_til_transfer_completed(XukcDirtEditor);
	return (action_list);
}


Boolean
ObjectGetValues(object, args, num_args, as_strings, values_freeable)
ObjectPtr object;
ArgList args;
Cardinal num_args;
Boolean as_strings;		/* results are returned as Strings */
Boolean values_freeable;	/* args[].addr can be freed if we don't need
				   them */
{
	Cardinal i;
	Boolean ok;

	if (!object->is_remote) {
		XtGetValues(object->instance, args, num_args);
		XFlush(XtDisplayOfObject(object->instance));
		if (as_strings)
			/* the strings returned should be freed when
			 * not needed anymore */
			convert_args(object, args, args, num_args, TRUE);
		return TRUE;
	}
	current_object = object;
	comms_window = CurrentCommsWindow();
	transfer_status = FALSE;

	if (!as_strings) {
		original_args = args;
		args = (ArgList)XtMalloc(sizeof(Arg) * num_args);
		for (i = 0; i < num_args; i++)
			XtSetArg(args[i], original_args[i].name, NULL);
	} else {
		for (i = 0; i < num_args; i++) {
			if (values_freeable)
				XtFree((char *)args[i].value);
			args[i].value = NULL;
		}
	}

	ok = XukcRemoteGetValues(XukcDirtEditor, comms_window, object->instance,
				args, num_args, (as_strings ?
				(XukcIcrmProc)finished_blocked_transfer :
				(XukcIcrmProc)finished_ObjectGetValues));
	if (ok)
		ok = block_til_transfer_completed(XukcDirtEditor);

	/* if as_strings == FALSE then the contents of args[?].value are
	 * freed automatically otherwise they are malloc'ed strings which
	 * should be freed when not needed anymore. */
	if (!as_strings) {
		XtFree((char *)args);
		XukcDestroyList(&free_list, TRUE);
	}
	return (ok);
}


Boolean
ObjectSetValues(object, args, num_args)
ObjectPtr object;
ArgList args;
Cardinal num_args;
{
	Cardinal i;
	Boolean ok;

	if (!object->is_remote) {
		XtSetValues(object->instance, args, num_args);
		DrainUIEventQueue();
		return TRUE;
	}
	current_object = object;
	comms_window = CurrentCommsWindow();
	transfer_status = FALSE;

	/* use a copy of the arguments .. as they may have been malloc'ed */
	original_args = args;
	args = (ArgList)XtMalloc(sizeof(Arg) * num_args);
	for (i = 0; i < num_args; i++)
		XtSetArg(args[i], original_args[i].name, NULL);

	/* convert the arguments to strings to send across */
	convert_args(object, original_args, args, num_args, TRUE);

	ok = XukcRemoteSetValues(XukcDirtEditor, comms_window, object->instance,
				    args, num_args,
				    (XukcIcrmProc)finished_blocked_transfer);
	if (ok)
		ok = block_til_transfer_completed(XukcDirtEditor);

	for (i = 0; i < num_args; i++)
		if (args[i].value != NULL)
			XtFree((char *)args[i].value);
	XtFree((char *)args);
	XukcDestroyList(&free_list, TRUE);
	return (ok);
}


Widget
ObjectWindowToWidget(object, window)
ObjectPtr object;
Window window;
{
	static Widget ret_w;

	if (!object->is_remote)
		return (XtWindowToWidget(XtDisplay(object->instance), window));

	current_object = object;
	comms_window = CurrentCommsWindow();
	transfer_status = FALSE;
	ret_w = NULL;

	if (!XukcRemoteWindowToWidget(XukcDirtEditor, comms_window,
	    window, &ret_w, (XukcIcrmProc)finished_blocked_transfer))
		return NULL;

	(void)block_til_transfer_completed(XukcDirtEditor);
	return (ret_w);
}


Boolean
ObjectConversion(object, from_type, from, to_type, to)
ObjectPtr object;
String from_type;
XrmValuePtr from;
String to_type;
XrmValuePtr to;
{
	static String retstr = NULL;
	Boolean ok;
	Boolean store_in_struct;

	if (retstr != NULL) {
		XtFree(retstr);
		retstr = NULL;
	}

	if (!object->is_remote)
		return (DoConversion(object->instance, from_type, from,
								to_type, to));
	current_object = object;
	comms_window = CurrentCommsWindow();
	transfer_status = FALSE;
	store_in_struct = (to->addr == NULL);

	ok = XukcRemoteConversion(XukcDirtEditor, comms_window,
				object->instance,
				from_type, from, to_type, &to,
				(XukcIcrmProc)finished_blocked_transfer);
	if (ok)
		ok = block_til_transfer_completed(XukcDirtEditor);

	if (!ok) {
		to->size = 0;
		to->addr = NULL;
	} else if (IsStringType(to_type)) {
		/* the result is a String so we must do some handwaving
		 * and take a copy of the string .. keeping a reference
		 * to this memory locally and then free the memory
		 * allocated by the ICRM call :  We do this to keep
		 * the semantics the same as the non-remote conversion
		 * procedure */
		if (store_in_struct) {
			retstr = XtNewString(to->addr);
			XtFree(to->addr);
			to->addr = retstr;
		} else {
			retstr = XtNewString(*(String *)to->addr);
			XtFree(*(String *)to->addr);
			to->addr = (XtPointer)&retstr;
		}
	}
	return (ok);
}


/* ||| To Be Implemented Properly ||| */
Boolean
ObjectAddCallback(object, callback, proc, client_data)
ObjectPtr object;
String callback;
void (*proc)();
XtPointer client_data;
{
	if (!object->is_remote) {
		XtAddCallback(object->instance, callback, proc, client_data);
		return TRUE;
	}
	return FALSE;
}


Boolean
ObjectRemoveCallback(object, callback, proc, client_data)
ObjectPtr object;
String callback;
void (*proc)();
XtPointer client_data;
{
	if (!object->is_remote) {
		XtRemoveCallback(object->instance, callback, proc, client_data);
		return TRUE;
	}
	return FALSE;
}


Boolean
NeedsRemoteConversion(from_type, to_type)
String from_type, to_type;
{
	if (IsStringType(to_type))
		return TRUE;
	if (strcmp(XtRCursor, to_type) == 0 ||
	    strcmp(XtRFontStruct, to_type) == 0 ||
	    strcmp(XtRTranslationTable, to_type) == 0 ||
	    strcmp(XtRPixmap, to_type) == 0 ||
	    strcmp(XtRWidget, to_type) == 0 ||
	    strcmp(XtRWidgetList, to_type) == 0 ||
	    strcmp(XtRBitmap, to_type) == 0)
		return TRUE;
	return FALSE;
}
