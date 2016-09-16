#ifndef lint
static char sccsid[] = "@(#)help.c	1.2 (UKC) 5/10/92";
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
 * help.c : give help on resource settings.
 */

#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/text.h>
#include "dirt_structures.h"
#include "resources.h"

#ifndef HELP_DATABASE
#define HELP_DATABASE "DirtHelpDB"
#endif /* HELP_DATABASE */

#ifndef FILE_SEARCH_VARIABLE
#define FILE_SEARCH_VARIABLE "XUSERFILESEARCHPATH"
#endif /* FILE_SEARCH_VARIABLE */

extern Widget WI_help_shell;
extern Widget WI_help_resource_title, WI_help_resource_text;
extern Widget WI_help_type_label, WI_help_type_text;
extern Widget WI_help_class_label, WI_help_class_text;

/* externally defined routines */

extern String GetClassVariableName();
extern Cardinal GetSuperClass();
extern ObjectPtr GetResourceObject();
extern void XukcChangeLabel();

extern char *getenv();

static String help_classname = NULL, help_vendor = NULL, help_class = NULL;
static String help_type = NULL, help_resource = NULL;
static XrmDatabase help_db = NULL;

static Boolean
open_help_database()
{
	String filename = NULL;
	String search_path, full_path = NULL;

	if (help_db != NULL)
		return TRUE;

	if ((search_path = (String)getenv(FILE_SEARCH_VARIABLE)) != NULL) {
		if (index(search_path, '%') == NULL) {
			full_path = (String)XtMalloc(strlen(search_path) + 4);
			(void)sprintf(full_path, "%s/%%N", search_path);
		} else
			full_path = XtNewString(search_path);
	}

	filename = XtResolvePathname(XtDisplay(WI_help_shell), NULL,
			HELP_DATABASE, NULL, full_path, NULL, 0, NULL);

	if (filename != NULL) {
		help_db = XrmGetFileDatabase(filename);
		XtFree(filename);
	}
	if (full_path != NULL)
		XtFree(full_path);
	return (help_db != NULL);
}


static void
show_help_found(w_class, resource, class, type, is_constraint)
String w_class, resource, class, type;
Boolean is_constraint;
{
	char label[500];

	/* popup the help dialog or update it if already popped up */
	XtPopup(WI_help_shell, XtGrabNone);

	if (is_constraint)
		(void)sprintf(label, "Constraint Resource = %s,   Class = %s",
				resource, class);
	else
		(void)sprintf(label, "Resource = %s,   Class = %s",
				resource, class);
	XukcChangeLabel(WI_help_resource_title, label, FALSE);
	(void)sprintf(label, "Type = %s", type);
	XukcChangeLabel(WI_help_type_label, label, FALSE);
	(void)sprintf(label, "Object Class = %s", w_class);
	XukcChangeLabel(WI_help_class_label, label, FALSE);

	XtVaSetValues(WI_help_resource_text, XtNeditType, XawtextEdit, NULL);
	XtVaSetValues(WI_help_type_text, XtNeditType, XawtextEdit, NULL);
	XtVaSetValues(WI_help_class_text, XtNeditType, XawtextEdit, NULL);

	XukcClearTextWidget(WI_help_resource_text);
	XukcClearTextWidget(WI_help_type_text);
	XukcClearTextWidget(WI_help_class_text);

	if (help_classname != NULL) {
		XukcAppendToTextWidget(WI_help_class_text, help_classname);
		XukcAppendToTextWidget(WI_help_class_text, "\n\n");
	}
	if (help_vendor != NULL) {
		XukcAppendToTextWidget(WI_help_class_text, "Part of the ");
		XukcAppendToTextWidget(WI_help_class_text, help_vendor);
		XukcAppendToTextWidget(WI_help_class_text, " Widget Set");
		XukcAppendToTextWidget(WI_help_class_text, "\n\n");
	}
	if (help_class != NULL)
		XukcAppendToTextWidget(WI_help_class_text, help_class);
	if (help_resource != NULL)
		XukcNewTextWidgetString(WI_help_resource_text, help_resource);
	if (help_type != NULL)
		XukcNewTextWidgetString(WI_help_type_text, help_type);

	XtVaSetValues(WI_help_resource_text, XtNeditType, XawtextRead, NULL);
	XtVaSetValues(WI_help_type_text, XtNeditType, XawtextRead, NULL);
	XtVaSetValues(WI_help_class_text, XtNeditType, XawtextRead, NULL);
}


/*********************** Module Exported Routines ***********************/

/* ARGSUSED */
void
GiveResourceHelp(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
#define HELP_LOOKUP(str) { \
	if (XrmGetResource(help_db, lookup, "", &ret_type, &value)) \
		str = (String)value.addr; \
	else \
		str = NULL; \
}
	String type, resource, lookup;
	Cardinal wc;
	String ret_type, class_name;
	XrmValue value;
	struct resource_entry *entry;
	ObjectPtr object;

	if (!open_help_database()) {
		GiveWarning(w, "noHelpAvailable", "giveResourceHelp",
			"No Help Database available ... sorry", 0);
		return;
	}

	entry = (struct resource_entry *)client_data;
	object = GetResourceObject(entry);
	wc = object->class;

	type = entry->type->resource_type;
	resource = entry->type->resource_name;

	class_name = GetClassVariableName(wc);
	lookup = XtMalloc(strlen(class_name) + strlen(resource) +
				strlen(type) + 11);

	(void)sprintf(lookup, "%s.FullName", class_name);
	HELP_LOOKUP(help_classname);
	(void)sprintf(lookup, "%s.Vendor", class_name);
	HELP_LOOKUP(help_vendor);
	(void)sprintf(lookup, "%s.Description", class_name);
	HELP_LOOKUP(help_class);
	(void)sprintf(lookup, "type%s", type);
	HELP_LOOKUP(help_type);
	if (entry->is_constraint)
		wc = object->parent->class;
	do {
		(void)sprintf(lookup, "%s.%s",
					GetClassVariableName(wc), resource);
		HELP_LOOKUP(help_resource);
		if (help_resource == NULL)
			wc = GetSuperClass(wc);
	} while (help_resource == NULL && wc > 0);
	XtFree(lookup);
	
	show_help_found(class_name, resource, entry->type->resource_class,
			type, entry->is_constraint);
}
