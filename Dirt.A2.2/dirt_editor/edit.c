#ifndef lint
static char sccsid[] = "@(#)edit.c	1.2 (UKC) 5/10/92";
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

/*
 * edit.c : contains the functions for editing instances of widgets
 */

#include <ctype.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Toggle.h>
#include "dirt_structures.h"

#ifndef VARIABLE_PREFIX
#	define VARIABLE_PREFIX "WI_"
#	define VARIABLE_PREFIX_LEN 3
#else
#	define VARIABLE_PREFIX_LEN strlen(VARIABLE_PREFIX)
#endif /* VARIABLE_PREFIX */

/* UI Widget Tree */
extern Widget WI_create_pressup, WI_create_hide;
extern Widget WI_create_shell, WI_instance_shell;
/* Edit Widget Instance */
extern Widget WI_instance_managed;
extern Widget WI_instance_resources_viewport;
extern Widget WI_instance_name_entry, WI_instance_class_entry;
extern Widget WI_instance_parent_entry, WI_instance_hide, WI_instance_pressup;
extern Widget WI_x_pos_label, WI_y_pos_label;
extern Widget WI_height_label, WI_width_label;
extern Widget WI_x_pos_value, WI_y_pos_value;
extern Widget WI_height_value, WI_width_value, WI_border_value;

extern WidgetClass GetClass();
extern String GetDisplayedClassName();
extern void GetUniqueName();
extern ObjectPtr GetStartOfList();
extern void UpdateApplicationNames();
extern void CreateResourceTable();
extern void DeleteResourceTable();
extern void EditInstanceCallbacks();
extern Boolean IsShellClass();
extern void XukcChangeLabel();
extern void RemoveInstanceCallbacks();
extern void CreateInstanceList();

char instance_name[STRSIZE];

static ObjectPtr current_widget = NULL;
static Boolean can_manage = TRUE;

void UpdateString();

#define SETVALUE(w, v)	XtVaSetValues(w, XtNvalue, v, NULL);

static void
reset_edit()
{
	/* set default values for instance entries */
	SETVALUE(WI_x_pos_value, 0);
	SETVALUE(WI_y_pos_value, 0);
	SETVALUE(WI_height_value, 0);
	SETVALUE(WI_width_value, 0);
	SETVALUE(WI_border_value, 0);

	XtVaSetValues(WI_x_pos_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_y_pos_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_width_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_height_label, XtNstate, FALSE, NULL);

	XukcClearTextWidget(WI_instance_name_entry);
	XukcChangeLabel(WI_instance_class_entry, "", FALSE);
	XukcChangeLabel(WI_instance_parent_entry, "", FALSE);
}

/**** Module Exported Routines ****/


void
InitializeEdit()
{
	/* set default mode */
	current_widget = NULL;
	reset_edit();
}


ObjectPtr
CurrentlyEditing()
{
	return (current_widget);
}


void
SetCurrentObject(obj)
ObjectPtr obj;
{
	current_widget = obj;
}


void
EditAnInstance(to_edit, no_popup)
ObjectPtr to_edit;
Boolean no_popup;
{
	RemoveInstanceCallbacks(current_widget);
	if (to_edit == NULL || to_edit == GetStartOfList(TRUE)) {
		reset_edit();
		current_widget = NULL;
		DeleteResourceTable(INSTANCE_RESOURCES);
		/* popdown the edit popup */
		XtCallCallbacks(WI_instance_hide, XtNcallback, (XtPointer)NULL);
		if (to_edit != NULL)
			/* popup the application details dialog */
			XtPopup(WI_create_shell, XtGrabNone);
	} else {
		Arg args[5];
		Dimension width = 500, height = 300, border_width = 1;
		Position x = 0, y = 0;

		/* popdown the application widget popup */
		if (current_widget == GetStartOfList(TRUE))
			XtCallCallbacks(WI_create_hide, XtNcallback,
					(XtPointer)NULL);
		current_widget = to_edit;
		CreateResourceTable(INSTANCE_RESOURCES, to_edit, 0, FALSE);
		XtSetArg(args[0], XtNx, &x);
		XtSetArg(args[1], XtNy, &y);
		XtSetArg(args[2], XtNwidth, &width);
		XtSetArg(args[3], XtNheight, &height);
		XtSetArg(args[4], XtNborderWidth, &border_width);
		ObjectGetValues(to_edit, (ArgList)args, 5, FALSE, FALSE);

		/* load the new fields to edit */
		SETVALUE(WI_x_pos_value, x);
		SETVALUE(WI_y_pos_value, y);
		SETVALUE(WI_height_value, height);
		SETVALUE(WI_width_value, width);
		SETVALUE(WI_border_value, border_width);
		XukcNewTextWidgetString(WI_instance_name_entry, to_edit->name);
		XukcChangeLabel(WI_instance_class_entry,
				GetDisplayedClassName(to_edit->class), FALSE);
		XukcChangeLabel(WI_instance_parent_entry,
					to_edit->parent->name, FALSE);
		XtVaSetValues(WI_x_pos_label, XtNstate,
				(to_edit->resource_flags & X_RES_LOCKED), NULL);
		XtVaSetValues(WI_y_pos_label, XtNstate,
				(to_edit->resource_flags & Y_RES_LOCKED), NULL);
		XtVaSetValues(WI_width_label, XtNstate,
			(to_edit->resource_flags & WIDTH_RES_LOCKED), NULL);
		XtVaSetValues(WI_height_label, XtNstate,
			(to_edit->resource_flags & HEIGHT_RES_LOCKED), NULL);

		if (to_edit->is_remote
		    || !(XtIsComposite(to_edit->parent->instance)
		    || IsShellClass(XtClass(to_edit->instance)))) {
			can_manage = FALSE;
			XtSetSensitive(WI_instance_managed, FALSE);
		} else {
			XtSetSensitive(WI_instance_managed, TRUE);
			can_manage = TRUE;
		}
		EditInstanceCallbacks(to_edit);
	}
}


String
CvtToVariableName(str)
String str;
{
	/* convert any upper case character to an underscore followed by
	 * lower case version */
	Cardinal i = strlen(str) + 1;
	Cardinal j, k;
	String ret_str;

	/* find the number of upper case characters in str */
	for (k = 0, j = i; j > 0; j--)
		k += isupper(str[j-1]);

	ret_str = XtMalloc(i + k + VARIABLE_PREFIX_LEN);
	(void)strcpy(ret_str, VARIABLE_PREFIX);

	if (k == 0) { /* no change necessary */
		(void)strcat(ret_str, str);
		return (ret_str);
	}

	for (k = VARIABLE_PREFIX_LEN, j = 0; j < i; j++)
		if (isupper(str[j])) {
			ret_str[k++] = '_';
			ret_str[k++] = str[j] - 'A' + 'a';
		} else
			ret_str[k++] = str[j];
	ret_str[k-1] = '\0';
	return (ret_str);
}


void
RenameWidgetInstance(w, new_name)
Widget w;
String new_name;
{
	/* there ought to be a way to rename a widget using the XtSetValues()
	 * interface ... instead we have to do this ourselves. */
	if (strcmp(new_name, w->core.name)) {
		w->core.name = XtNewString(new_name);
		w->core.xrm_name = XrmStringToQuark(w->core.name);
	}
}


/* ARGSUSED */
/* update the instance or application name from the associated text widget */
void
UpdateString(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal num_params;
{
	if (!XtIsSensitive(w))
		return;
	if (w == WI_instance_name_entry) {
		if (current_widget != NULL) {
			String tmp;

			tmp = XukcGetTextWidgetString(w);
#if 1
			/* ||| instance really needed anymore??? ||| */
			(void)strcpy(instance_name, tmp);
#endif
			GetUniqueName(current_widget, tmp);
			if (!current_widget->is_remote)
				RenameWidgetInstance(current_widget->instance,
						current_widget->name);
			CreateInstanceList(GetStartOfList(current_widget->in_app));
			XukcNewTextWidgetString(w, current_widget->name);
		}
	} else 
		UpdateApplicationNames(w);
}


/* Allow or disallow overridding the Intrinsics geometry management.
 * Geometry management is overridden by pretending that the object is
 * is unmanaged without actually doing the XtUnmanageChild() which would
 * cause the window to be unmapped which would screen flicker.
 */
void
AllowAnyGeometryChange(obj, allow)
ObjectPtr obj;
Boolean allow;
{
	if (obj->is_remote)
		return; /* we can't do anything about remote objects yet */

	if (XtIsWidget(obj->instance))
		obj->instance->core.managed = allow ? FALSE : obj->managed;
	else if (XtIsRectObj(obj->instance))
		((RectObj)obj->instance)->rectangle.managed = allow
						? FALSE : obj->managed;
	/* ObjectClass widgets cannot be managed so do nothing for them */
}


/* Dive straight inside a composite/constraint parent widget and call its
 * resize layout procedure.  As no resize is taking place this is done
 * so any children whose geometry has changed are re-layed out.
 */
void
CallParentLayoutProc(w)
Widget w;
{
	CompositeWidget parent = (CompositeWidget)w;

	if (XtIsComposite(w))
		(*((CompositeWidgetClass)parent->core.widget_class)->composite_class.change_managed)(parent);
}
