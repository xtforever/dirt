#ifndef lint
static char sccsid[] = "@(#)translations.c	1.2 (UKC) 5/10/92";
#endif /* !lint */

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

/* Modified from Xt/TMstate.c to allow dynamic editing of translations */
/* Richard Hesketh, rlh2@ukc.ac.uk : Feb'89 */
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


#define XK_LATIN1

#include <stdio.h>
#include <X11/StringDefs.h>

#include "dirt_structures.h"
#include <X11/Xaw/List.h>
#include <X11/Xukc/Error.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/memory.h>

extern String *ObjectGetActionList();
extern void BindCallback();
extern String GetClassName();
extern void NewResourceValue();

extern Widget WI_trans_shell;
extern Widget	WI_current_translations;
extern Widget	WI_trans_apply_button, WI_trans_hide_button;
extern Widget	WI_actions_view;
extern Widget	WI_events_view;
extern Widget	WI_trans_text;
extern Widget	WI_trans_error;

static Widget actions_list = NULL;
static Widget events_list = NULL;


/* all the events available in the translation manager */

static String event_names[] = {
  "<Btn1Down>", "<Btn1Motion>", "<Btn1Up>", "<Btn2Down>", "<Btn2Motion>",
  "<Btn2Up>", "<Btn3Down>", "<Btn3Motion>", "<Btn3Up>", "<Btn4Down>",
  "<Btn4Motion>", "<Btn4Up>", "<Btn5Down>", "<Btn5Motion>", "<Btn5Up>",
  "<BtnDown>", "<BtnMotion>", "<BtnUp>", "<ButtonPress>", "<ButtonRelease>",
  "<CirculateNotify>", "<CirculateRequest>", "<ClientMessage>",
  "<ColormapNotify>", "<ConfigureNotify>", "<ConfigureRequest>",
  "<CreateNotify>", "<Ctrl>", "<DestroyNotify>", "<EnterWindow>",
  "<EventTimer>", "<Expose>", "<FocusIn>", "<FocusOut>", "<GraphicsExpose>",
  "<GravityNotify>", "<Key>", "<KeyDown>", "<KeyPress>", "<KeyRelease>",
  "<KeyUp>", "<KeymapNotify>", "<LeaveWindow>", "<MapNotify>", "<MapRequest>",
  "<MappingNotify>", "<Meta>", "<Motion>", "<MotionNotify>", "<MouseMoved>",
  "<NoExpose>", "<PropertyNotify>", "<PtrMoved>", "<ReparentNotify>",
  "<ResizeRequest>", "<SelectionClear>", "<SelectionNotify>",
  "<SelectionRequest>", "<Shift>", "<Timer>", "<UnmapNotify>",
  "<VisibilityNotify>",
};

static XtPointer entry = NULL;
static ObjectPtr tw_obj = NULL;
static String resource_name = NULL;
static Boolean no_errors_found = TRUE;

/* externally called callback routines */

static void
found_error(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	no_errors_found = FALSE;
}


static int
compare_strings(a, b)
String a, b;
{
	return (strcmp(*(String *)a, *(String *)b));
}


static void
free_actions_list(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XukcDestroyList((Cardinal *)&client_data, FALSE);
}


static void
pick_and_paste(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	char paste_string[100];
	XawTextPosition insert_pos = XawTextGetInsertionPoint(WI_trans_text);
	XawListReturnStruct *picked_string = (XawListReturnStruct *)call_data;

	if ((Boolean)client_data)
		(void)sprintf(paste_string, " %s()", picked_string->string);
	else
		(void)strcpy(paste_string, picked_string->string);

	(void)XukcModifyText(WI_trans_text, paste_string, insert_pos, insert_pos);
	XawTextSetInsertionPoint(WI_trans_text, insert_pos + strlen(paste_string));
}


static void
create_trans_lists(obj)
ObjectPtr obj;
{
	Arg args[2];
	Cardinal list_length;
	String *action_names = NULL;

	action_names = ObjectGetActionList(obj, &list_length);
	if (actions_list == NULL) {
		XtSetArg(args[0], XtNlist, action_names);
		XtSetArg(args[1], XtNnumberStrings, list_length);
		actions_list = XtCreateManagedWidget("actionsList",
					listWidgetClass, WI_actions_view,
					(ArgList)args, 2);

		XtAddCallback(actions_list, XtNcallback, pick_and_paste,
							(XtPointer)TRUE);
	} else {
		XtRemoveAllCallbacks(actions_list, XtNdestroyCallback);
		XawListChange(actions_list, action_names, list_length, 0, TRUE);
	}

	XtAddCallback(actions_list, XtNdestroyCallback, free_actions_list,
						(XtPointer)action_names);

	/* this only gets made once */
	if (events_list == NULL) {
		XtSetArg(args[0], XtNlist, event_names);
		XtSetArg(args[1], XtNnumberStrings, XtNumber(event_names));
		events_list = XtCreateManagedWidget("events_list",
						listWidgetClass, WI_events_view,
						(ArgList)args, 2);

		XtAddCallback(events_list, XtNcallback, pick_and_paste,
							(XtPointer)FALSE);
	}
}


/***** Module Exported Routines *****/


void
InitializeTranslationsEditor()
{
	XtAddCallback(WI_trans_error, XtNerrorCallback, found_error,
							(XtPointer)NULL);
}


void
PopupTranslations(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Arg args[1];
	char mesg[200];
	String *p = (String *)client_data; /* 4 arguments are passed */
	XtTranslations default_value;

	/* ||| global variables ||| */
	entry = (XtPointer)p[0];
	tw_obj = (ObjectPtr)p[1];
	resource_name = (String)p[2];

	default_value = (XtTranslations)p[3];

	/* create the list of available events and actions */
	create_trans_lists(tw_obj);

	if (default_value != NULL) {
		XrmValue from, to;

		from.addr = (XtPointer)&default_value;
		from.size = sizeof(XtTranslations);
		to.addr = NULL;
		to.size = 0;

		if (!ObjectConversion(tw_obj, XtRTranslationTable, &from,
		    XtRString, &to) || to.addr == NULL)
			return;
		XukcNewTextWidgetString(WI_trans_text, (String)to.addr);
	} else {
		/* retrieve the current value of the resource */
		XtSetArg(args[0], resource_name, NULL);
		ObjectGetValues(tw_obj, (ArgList)args, 1, TRUE, FALSE);
		if (args[0].value == 0)
			args[0].value = (int)XtNewString("");
		XukcNewTextWidgetString(WI_trans_text, (String)args[0].value);
		/* we can free this because it was created by
		 * ObjectGetValues() */
		XtFree((String)args[0].value);
	}

	(void)sprintf(mesg, "\"%s\" of \"%s\" : (%s)", resource_name,
				tw_obj->name, GetClassName(tw_obj->class));
	XtSetArg(args[0], XtNlabel, mesg);
	XtSetValues(WI_current_translations, (ArgList)args, 1);
	XtPopup(WI_trans_shell, XtGrabNone);
}


void
ApplyTranslations(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	String tm_str;
	Cardinal eos;
	XtTranslations new_tm;
	Arg args[1];

	/* get the new translation string */
	XtSetArg(args[0], XtNstring, &tm_str);
	XtGetValues(WI_trans_text, (ArgList)args, 1);

	/* strip off trailing newlines to stop TT parser complaining */
	eos = strlen(tm_str);
	while (eos > 0 && tm_str[eos - 1] == '\n') {
		tm_str[eos - 1] = '\0';
		eos--;
	};

	no_errors_found = TRUE; /* global variable */

	XtSetArg(args[0], XtNhandleXtErrors, TRUE);
	XtSetValues(WI_trans_error, (ArgList)args, 1);

	new_tm = XtParseTranslationTable(tm_str);

	if (no_errors_found && tw_obj->is_remote) {
		/* then we must parse it remotely as well then
		 * pass on the pointer to this newly parsed
		 * translation string - phew */
		XrmValue from, to;

		from.addr = tm_str;
		from.size = strlen(tm_str);
		to.addr = NULL;
		to.size = 0;
		no_errors_found |= !ObjectConversion(tw_obj,
					XtRString, &from,
					XtRTranslationTable, &to);
		new_tm = *(XtTranslations *)to.addr;
	}

	XtSetArg(args[0], XtNhandleXtErrors, FALSE);
	XtSetValues(WI_trans_error, (ArgList)args, 1);

	if (no_errors_found)
		NewResourceValue(w, entry, (XtPointer)new_tm);
}
