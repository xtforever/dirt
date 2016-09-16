#ifndef lint
static char sccsid[] = "@(#)wedit.c	1.1 (UKC) 5/10/92";
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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/text.h>
#include "dirt_structures.h"
#include "resources.h"

/* externally defined routines */
extern void CreateResourceTable();
extern Cardinal NewResourceList();
extern void ReleaseResourceList();
extern ObjectPtr CreateObjectRecord();
extern ObjectPtr SearchRecords();
extern ObjectPtr GetHeadObject();
extern String UniqueWidgetName();
extern void RenameWidgetInstance();
extern void WcCreateNamedChildren();
extern Widget WcFullNameToWidget();
extern void XukcChangeLabel();
extern Cardinal FindClass();
extern void AddCreatedObject();
extern void AddDestroyCallback();
extern void XukcDirtAddLayoutConnections();
extern void FreeMemoryCB();

extern Widget WI_dirt;
extern Widget WI_widget_layout_shell;
extern Widget  WI_widget_layout;
extern Widget   WI_widget_entry_label;
extern Widget   WI_widget_hide;
extern Widget   WI_widget_entry;
extern Widget   WI_widget_resources_label;
extern Widget   WI_widget_resources_viewport;


static void
popdown_widget(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	w = (Widget)client_data;

	if (!w->core.being_destroyed)
		XtPopdown(w);
}


static void
destroy_widget(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	w = (Widget)client_data;

	/* if NULL then assume its already been destroyed */
	if (w != NULL)
		XtDestroyWidget(w);
}


static ObjectPtr
get_sub_object(object, sw)
ObjectPtr object;
Widget sw;
{
	ObjectPtr sub_object = NULL;

	if ((sub_object = SearchRecords(GetHeadObject(), sw)) != NULL)
		return (sub_object);

	/* ||| if object is remote then we should already know about the
         * ||| sub-object so don't create a record for it. ||| */
	sub_object = object->children;
	for (; sub_object != NULL && sub_object->instance != sw;)
		sub_object = sub_object->next;

	if (sub_object == NULL) {
		/* This object is not transient, it stays around as long as its
		 * parent does.  It is used to hold resource information
		 * for any popup resource lists that reference it.
		 */
		sub_object = CreateObjectRecord();
		sub_object->parent = object;
		sub_object->instance = sw;
		sub_object->is_remote = FALSE;
		sub_object->is_resource = TRUE;
		sub_object->in_app = object->in_app;
		sub_object->name = XtNewString(XtName(sw));
		sub_object->class = FindClass(XtClass(sw));
		AddCreatedObject(sub_object);
		AddDestroyCallback(sub_object);
	}
	return (sub_object);
}


/**** Module Exported Routines *****/


void
NewWidgetValue(tw, lw, object, widget, button, entry, list_number, override)
Widget tw, lw;			/* text widget , label widget */
ObjectPtr object;
Widget widget, button;
struct resource_entry *entry;
Cardinal list_number;
Boolean override;
{
	ObjectPtr sub_object;
	Widget sw, ws;
	Arg args[1];
	char tmp[100];
	String full_name;

	if (widget == NULL) {
		sw = NULL;
		XtSetArg(args[0], entry->type->resource_name, &sw);
		ObjectGetValues(object, (ArgList)args, 1, FALSE, FALSE);
	} else
		sw = widget;

	if (sw != NULL) {
		sub_object = get_sub_object(object, sw);
		if (sub_object->class == 0) {
			/* class of sub-object not found */
			(void)sprintf(tmp, "Unknown Object Class in \"%s\"",
							sub_object->name);
			XukcChangeLabel(lw, tmp, TRUE);
			return;
		}
		(void)sprintf(tmp, "\"%s\" resources", sub_object->name);
		XukcNewTextWidgetString(tw, sub_object->name);
	} else {
		(void)sprintf(tmp, "%s resources", entry->type->resource_name);
		XukcClearTextWidget(tw);
	}
	XukcChangeLabel(lw, tmp, TRUE);
	if (sw == NULL)
		full_name = XtNewString("NULL");
	else
		full_name = UniqueWidgetName(sw);
	XukcChangeLabel(button, full_name, TRUE);
	XtFree(full_name);

	if (sw != NULL && ((sw != (Widget)entry->value) || override)) {
		CreateResourceTable(list_number, sub_object, sub_object->class,
					FALSE);
		/* make sure this popup is popped down when the currently
		 * displayed widget is destroyed.
		 */
		ws = lw;
		while (!XtIsShell(ws))
			ws = XtParent(ws);

		XtAddCallback(sw, XtNdestroyCallback, popdown_widget,
				(XtPointer)ws);
	}
}


void
UpdateWidgetResource(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	struct resource_entry *entry_ptr;
	ObjectPtr obj;
	Widget widget;
	XtPointer *pass_on;
	int res;

	(void)sscanf(params[0], "%ld", &pass_on);

	obj = (ObjectPtr)pass_on[0];
	res = *(int *)pass_on[3];
	entry_ptr = (struct resource_entry *)pass_on[4];
	widget = *(Widget *)pass_on[6];

	NewWidgetValue(w, widget, (ObjectPtr)obj, (Widget)NULL,
			(Widget)pass_on[7],
			entry_ptr, (Cardinal)res, event->type == KeyPress);
}


/* Create a new resource list for a Widget resource being edited.  We
 * also create the popup shell and layout for it.
 */
void
CreateEditWidget(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	static String strTrans = "<Key>Return: dirtACTUpdateTextResource(%d) dirtACTUpdateWidgetResource(%d) \n\
				  <LeaveWindow>: dirtACTUpdateTextResource(%d) dirtACTUpdateWidgetResource(%d)";
	static int shell_num = 0;
	char tmp[300];
	ObjectPtr object;
	int *been_created;
	Widget widget;
	XtPointer *pass_on, entry_ptr;
	Boolean destroyed;

	pass_on = (XtPointer *)client_data;
	object = (ObjectPtr)pass_on[0];
	widget = (Widget)pass_on[2];
	been_created = (int *)pass_on[3];
	entry_ptr = pass_on[4];
	destroyed = (w != (Widget)pass_on[7]);

	if (*been_created < 0) {
		/* create a new copy of the sub-object editing dialog */
		WcCreateNamedChildren(WI_dirt, "widgetLayoutShell");

		WI_widget_layout_shell = WcFullNameToWidget(WI_dirt,
							 "*widgetLayoutShell");
		(void)sprintf(tmp, "widgetLayoutShell%d", shell_num++);
		RenameWidgetInstance(WI_widget_layout_shell, tmp);

		WI_widget_layout = WcFullNameToWidget(WI_widget_layout_shell,
							".widgetLayout");
		WI_widget_resources_viewport =
				WcFullNameToWidget(WI_widget_layout,
						".widgetResourcesViewport");
		WI_widget_resources_label =
				WcFullNameToWidget(WI_widget_layout,
						".widgetResourcesLabel");
		WI_widget_hide = WcFullNameToWidget(WI_widget_layout,
						".widgetHide");
		WI_widget_entry = WcFullNameToWidget(WI_widget_layout,
						".widgetEntry");
		WI_widget_entry_label = WcFullNameToWidget(WI_widget_layout,
						".widgetEntryLabel");
		XukcDirtAddLayoutConnections();

		*been_created = NewResourceList(WI_widget_resources_viewport);
		XtAddCallback(WI_widget_hide, XtNcallback, popdown_widget,
					(XtPointer)WI_widget_layout_shell);
	
		if (*((Widget *)pass_on[5]) == NULL) {
			/* no editing of the widget value allowed */
			XtUninstallTranslations(WI_widget_entry);
			XtVaSetValues(WI_widget_entry, XtNdisplayCaret, FALSE,
					NULL);
		} else {
			(void)sprintf(tmp, strTrans, entry_ptr, pass_on,
					entry_ptr, pass_on);
			XtOverrideTranslations(WI_widget_entry,
					XtParseTranslationTable(tmp));
		}
		*((Widget *)pass_on[5]) = WI_widget_entry;
		*((Widget *)pass_on[6]) = WI_widget_resources_label;

		/* when the button in the resource table that pops this
		 * resource list up dies then we must destroy all of this
		 * popup and associated data */
		XtAddCallback(w, XtNdestroyCallback, ReleaseResourceList,
					(XtPointer)been_created);
		XtAddCallback(w, XtNdestroyCallback, FreeMemoryCB,
					(XtPointer)been_created);
		XtAddCallback(w, XtNdestroyCallback, destroy_widget,
					(XtPointer)WI_widget_layout_shell);
	} else {
		WI_widget_entry = *(Widget *)pass_on[5];
		WI_widget_resources_label = *(Widget *)pass_on[6];
	}

	if (widget == NULL || (!destroyed && widget != NULL))
		NewWidgetValue(WI_widget_entry, WI_widget_resources_label,
				object, widget, (Widget)pass_on[7],
				(struct resource_entry *)entry_ptr,
				(Cardinal)*been_created, TRUE);
	else
		XukcChangeLabel((Widget)pass_on[7], "NULL", FALSE);

	WI_widget_layout_shell = WI_widget_entry;
	while (!XtIsShell(WI_widget_layout_shell))
		WI_widget_layout_shell = XtParent(WI_widget_layout_shell);
	if (!destroyed)
		XtPopup(WI_widget_layout_shell, XtGrabNone);
	else
		XtPopdown(WI_widget_layout_shell);
}
