#ifndef lint
static char sccsid[] = "@(#)undo.c	1.2 (UKC) 5/10/92";
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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/Med3Menu.h>
#include <X11/Xukc/memory.h>
#include "dirt_bindings.h"

static Boolean undo_initialized = FALSE;
static Cardinal undo_proc_list = 0;
static Cardinal undo_actions_list = 0;
static short last_action_done = NULL;
static Arg turn_off[] = {
	{ XtNsensitive, (XtArgVal)FALSE },
	{ XtNborderWidth, (XtArgVal)0 },
};
static Arg turn_on[] = {
	{ XtNsensitive, (XtArgVal)TRUE },
	{ XtNborderWidth, (XtArgVal)0 },
};

extern Widget WI_main_menu;

RegisterUndoProcedure(action, proc)
short action;
XtCallbackProc proc;
{
	Cardinal i;

	if (!undo_initialized) {
		XukcSetButtonValues((UKCMed3MenuWidget)WI_main_menu, UNDO,
					turn_off, XtNumber(turn_off));
		undo_initialized = TRUE;
	}

	if ((i = XukcAddObjectToList(&undo_actions_list, (XtPointer)action, TRUE)) > 0)
		XukcAddObjectToList(&undo_proc_list, (XtPointer)proc, FALSE);
	else /* already exists so just change it! */
		XukcChangeObjectOnList(undo_proc_list, i, (XtPointer)proc);
}


RegisterUndoAction(action)
short action;
{
	last_action_done = action;
	XukcSetButtonValues((UKCMed3MenuWidget)WI_main_menu, UNDO, turn_on, XtNumber(turn_on));
}


SayNothingToUndo(message)
String message;
{
	printf("Cannot %s : nothing to undo!\n", message);
}


UndoLastAction(w)
Widget w;
{
	Cardinal i;
	XtCallbackProc undo_proc;

	if (last_action_done == NULL) {
		SayNothingToUndo("UNDO");
		return;
	}

	i = XukcFindObjectOnList(undo_actions_list, (XtPointer)last_action_done);

	/* ||| remove ||| */
	if (i == 0)
		XtWarning("last action has no undo procedure");
	else {
		undo_proc = (XtCallbackProc)XukcGetObjectOnList(undo_proc_list, i);
		(*undo_proc)(w, (XtPointer)last_action_done, (XtPointer)NULL);
	}

	XukcSetButtonValues((UKCMed3MenuWidget)WI_main_menu, UNDO, turn_off, XtNumber(turn_off));
}


void
UndoAction(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	UndoLastAction(w);
}
