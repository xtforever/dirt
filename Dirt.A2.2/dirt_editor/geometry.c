#ifndef lint
static char sccsid[] = "@(#)geometry.c	1.2 (UKC) 5/10/92";
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
#include <X11/Shell.h>
#include <X11/Xukc/Value.h>
#include <X11/Xukc/Toggle.h>
#include "dirt_structures.h"

extern Widget WI_instance_managed, WI_instance_deferred;
extern Widget WI_x_pos_label, WI_x_pos_value;
extern Widget WI_y_pos_label, WI_y_pos_value;
extern Widget WI_width_label, WI_height_label;
extern Widget WI_width_value, WI_height_value, WI_border_value;
extern Widget WI_app_x_locked_toggle, WI_app_y_locked_toggle;
extern Widget WI_app_width_locked_toggle, WI_app_height_locked_toggle;
extern Widget WI_app_x_pos_value, WI_app_y_pos_value;
extern Widget WI_app_width_value, WI_app_height_value, WI_app_border_value;
extern Widget WI_new_x_pos_value, WI_new_y_pos_value;
extern Widget WI_new_width_value, WI_new_height_value, WI_new_border_value;

extern struct app_resources editor_resources;

extern ObjectPtr CurrentlyEditing();
extern void LogResourceSetting();
extern void SetResourceObjectValue();
extern Boolean IsShellClass();
extern Boolean IsCompositeClass();
extern WidgetClass GetClass();
extern ObjectPtr GetHeadObject();
extern String XukcCreateMeasurementString();
extern void AllowAnyGeometryChange();
extern void DrainUIEventQueue();
extern void EditAnInstance();
extern void ChangeSelection();
extern Boolean ObjectAddCallback();
extern Boolean ObjectRemoveCallback();

/* forward declarations */
XtEventHandler app_config_values();


static void
lock_change(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
#define CHANGE_LOCK(flag) (obj->resource_flags = new_state ? \
			   obj->resource_flags | flag : \
			   obj->resource_flags & ~flag)
	ObjectPtr obj;
	Boolean new_state;

	obj = (ObjectPtr)client_data;
	new_state = (Boolean)call_data;

	if (w == WI_x_pos_label)
		CHANGE_LOCK(X_RES_LOCKED);
	else if (w == WI_y_pos_label)
		CHANGE_LOCK(Y_RES_LOCKED);
	else if (w == WI_width_label)
		CHANGE_LOCK(WIDTH_RES_LOCKED);
	else if (w == WI_height_label)
		CHANGE_LOCK(HEIGHT_RES_LOCKED);
#undef CHANGE_LOCK
}


static void
dimension_change(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
#ifdef UKC_WIDGETS
	int old_geom = 0;
	Arg pargs[1];
#endif /* UKC_WIDGETS */
	Arg args[5];
	ObjectPtr obj;
	Dimension a_dimension;
	Position a_position;
	int value;

	if (client_data == NULL)
		return;

	value = (int)call_data;
	a_dimension = (Dimension)value;
	a_position = (Position)value;
	obj = (ObjectPtr)client_data;

	if (!NO_PARENT(obj) && IsCompositeClass(GetClass(obj->parent->class))) {
		if (w == WI_x_pos_value)
			XtSetArg(args[0], XtNx, a_position);
		else if (w == WI_y_pos_value)
			XtSetArg(args[0], XtNy, a_position);
		else if (w == WI_width_value)
			XtSetArg(args[0], XtNwidth, a_dimension);
		else if (w == WI_height_value)
			XtSetArg(args[0], XtNheight, a_dimension);
		else if (w == WI_border_value)
			XtSetArg(args[0], XtNborderWidth, a_dimension);
		else
			XtError("dimension_change() called by unknown widget");

#ifdef UKC_WIDGETS
		XtSetArg(pargs[0], XtNgeomStatus, &old_geom);
		ObjectGetValues(obj->parent, (ArgList)pargs, 1, FALSE, FALSE);
		XtSetArg(pargs[0], XtNgeomStatus, XukcTurnedOff);
		ObjectSetValues(obj->parent, (ArgList)pargs, 1);
#endif /* UKC_WIDGETS */
		
		ObjectSetValues(obj, (ArgList)args, 1);

#ifdef UKC_WIDGETS
		XtSetArg(pargs[0], XtNgeomStatus, old_geom);
		ObjectSetValues(obj->parent, (ArgList)pargs, 1);
#endif /* UKC_WIDGETS */
	} else {
		Dimension x, y;
		Position width, height, border_width;

		XtSetArg(args[0], XtNx, &x);
		XtSetArg(args[1], XtNy, &y);
		XtSetArg(args[2], XtNwidth, &width);
		XtSetArg(args[3], XtNheight, &height);
		XtSetArg(args[4], XtNborderWidth, &border_width);
		ObjectGetValues(obj, (ArgList)args, 5, FALSE, FALSE);

		if (w == WI_x_pos_value)
			x = a_position;
		else if (w == WI_y_pos_value)
			y = a_position;
		else if (w == WI_width_value)
			width = a_dimension;
		else if (w == WI_height_value)
			height = a_dimension;
		else if (w == WI_border_value)
			border_width = a_dimension;
		else
			XtError("dimension_change() called by unknown widget");

		AllowAnyGeometryChange(obj, TRUE);

		/* ||| How to cope with remote widgets?? ||| */
		if (!obj->is_remote) {
			XtMoveWidget(obj->instance, x, y);
			XtResizeWidget(obj->instance, width, height,
							border_width);
		}
		AllowAnyGeometryChange(obj, FALSE);
		DrainUIEventQueue();
	}
}


/* ARGSUSED */
static XtEventHandler
config_values(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
#define MEASUREMENT(dim) (editor_resources.pixels ? \
	((void)sprintf(tmp, "%d", dim), (String)tmp) : \
	XukcCreateMeasurementString(XtScreen(w), (int)dim, !editor_resources.inches))

	Arg args[1];
	char tmp[30];
	ObjectPtr obj = (ObjectPtr)client_data;

	if (event->type != ConfigureNotify)
		return;

	if (event->xconfigure.override_redirect)
		return;

	if (obj == CurrentlyEditing()) {
		XtSetArg(args[0], XtNvalue, event->xconfigure.x);
		XtSetValues(WI_x_pos_value, (ArgList)args, 1);

		XtSetArg(args[0], XtNvalue, event->xconfigure.y);
		XtSetValues(WI_y_pos_value, (ArgList)args, 1);

		XtSetArg(args[0], XtNvalue, event->xconfigure.height);
		XtSetValues(WI_height_value, (ArgList)args, 1);

		XtSetArg(args[0], XtNvalue, event->xconfigure.width);
		XtSetValues(WI_width_value, (ArgList)args, 1);

		XtSetArg(args[0], XtNvalue, event->xconfigure.border_width);
		XtSetValues(WI_border_value, (ArgList)args, 1);
	}
	if (!obj->is_remote)
		w = obj->instance;
	
	obj->x = event->xconfigure.x;
	obj->y = event->xconfigure.y;
	obj->width = event->xconfigure.width;
	obj->height = event->xconfigure.height;

	LogResourceSetting(obj, XtNborderWidth,
			MEASUREMENT(event->xconfigure.border_width), FALSE);
}


/* callback called when the "Managed" toggle button is toggled */
/* ARGSUSED */
static void
change_managed(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	ObjectPtr obj = (ObjectPtr)client_data;

	if (client_data == NULL || !XtIsRectObj(obj->instance))
		return;

	if (IsShellClass(GetClass(obj->class))) {
		if ((Boolean)call_data)
			XtPopup(obj->instance, XtGrabNone);
		else
			XtPopdown(obj->instance);
	} else {	
		if ((Boolean)call_data) {
			XtManageChild(obj->instance);
			if (CurrentlyEditing() == obj)
				EditAnInstance(obj, FALSE);
		} else
			XtUnmanageChild(obj->instance);
	}
	DrainUIEventQueue();
}


/* called when widget instance is deleted */
/* ARGSUSED */
static void
unlink_dying_widget(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	ChangeSelection((ObjectPtr)NULL);
}


static void
app_lock_change(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
#define CHANGE_LOCK(flag) (obj->resource_flags = new_state ? \
			   obj->resource_flags | flag : \
			   obj->resource_flags & ~flag)
	ObjectPtr obj;
	Boolean new_state;

	obj = (ObjectPtr)client_data;
	new_state = (Boolean)call_data;

	if (w == WI_app_x_locked_toggle)
		CHANGE_LOCK(X_RES_LOCKED);
	else if (w == WI_app_y_locked_toggle)
		CHANGE_LOCK(Y_RES_LOCKED);
	else if (w == WI_app_width_locked_toggle)
		CHANGE_LOCK(WIDTH_RES_LOCKED);
	else if (w == WI_app_height_locked_toggle)
		CHANGE_LOCK(HEIGHT_RES_LOCKED);
#undef CHANGE_LOCK
}


/* ARGSUSED */
static XtEventHandler
app_config_values(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	Arg args[2];
	char tmp[20];
	ObjectPtr obj = (ObjectPtr)client_data;

	if (event->type != ConfigureNotify)
		return;
	if (!obj->is_remote)
		w = obj->instance;

	XtSetArg(args[0], XtNvalue, event->xconfigure.x);
	XtSetValues(WI_app_x_pos_value, (ArgList)args, 1);

	XtSetArg(args[0], XtNvalue, event->xconfigure.y);
	XtSetValues(WI_app_y_pos_value, (ArgList)args, 1);

	XtSetArg(args[0], XtNvalue, event->xconfigure.height);
	XtSetValues(WI_app_height_value, (ArgList)args, 1);

	XtSetArg(args[0], XtNvalue, event->xconfigure.width);
	XtSetValues(WI_app_width_value, (ArgList)args, 1);

	XtSetArg(args[0], XtNvalue, event->xconfigure.border_width);
	XtSetValues(WI_app_border_value, (ArgList)args, 1);

	obj->x = event->xconfigure.x;
	obj->y = event->xconfigure.y;
	obj->width = event->xconfigure.width;
	obj->height = event->xconfigure.height;

	LogResourceSetting(obj, XtNborderWidth,
			MEASUREMENT(event->xconfigure.border_width), FALSE);
}


/* ARGSUSED */
static void
app_dimension_change(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Arg args[2];
	ObjectPtr obj;
	int value;
	Dimension a_dimension;
	Position a_position;
	Boolean old_resize;

	if (client_data == NULL)
		return;

	value = (int)call_data;
	a_dimension = (Dimension)value;
	a_position = (Position)value;
	obj = (ObjectPtr)client_data;

	XtSetArg(args[0], XtNallowShellResize, &old_resize);
	ObjectGetValues(obj, (ArgList)args, 1);

	if (w == WI_app_x_pos_value)
		XtSetArg(args[0], XtNx, a_position);
	else if (w == WI_app_y_pos_value)
		XtSetArg(args[0], XtNy, a_position);
	else if (w == WI_app_width_value)
		XtSetArg(args[0], XtNwidth, a_dimension);
	else if (w == WI_app_height_value)
		XtSetArg(args[0], XtNheight, a_dimension);
	else if (w == WI_app_border_value)
		XtSetArg(args[0], XtNborderWidth, a_dimension);
	else
		XtError("app_dimension_change() called by unknown widget");

	XtSetArg(args[1], XtNallowShellResize, TRUE);
	ObjectSetValues(obj, (ArgList)args, 2);
	XtSetArg(args[0], XtNallowShellResize, old_resize);
	ObjectSetValues(obj, (ArgList)args, 1);
}


/* called when the application we are building is deleted */
/* ARGSUSED */
static void
remove_app_callbacks(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	ObjectPtr app = (ObjectPtr)client_data;

	XtRemoveCallback(WI_app_x_pos_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtRemoveCallback(WI_app_y_pos_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtRemoveCallback(WI_app_height_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtRemoveCallback(WI_app_border_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);

	XtRemoveCallback(WI_app_x_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);
	XtRemoveCallback(WI_app_y_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);
	XtRemoveCallback(WI_app_width_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);
	XtRemoveCallback(WI_app_height_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);

	if (!app->is_remote)
		XtRemoveEventHandler(app->instance, StructureNotifyMask, FALSE,
					(XtEventHandler)app_config_values,
					(XtPointer)app);
}


/* ARGSUSED */
static void
remove_event_handlers(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	ObjectPtr cl = (ObjectPtr)client_data;

	if (cl == NULL)
		XtError("RemoveEventHandlers() called with NULL client data");

	XtRemoveEventHandler(w, StructureNotifyMask, FALSE,
				(XtEventHandler)config_values, client_data);
}


/* Handler for feedback widget that is used to show where a created
 * widget instance will be put.
 */
/* ARGSUSED */
static void
new_dimension_change(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Arg args[2];
	int value;
	Dimension a_dimension;
	Position a_position;
	Widget fw;

	if (client_data == NULL)
		return;

	value = (int)call_data;
	a_dimension = (Dimension)value;
	a_position = (Position)value;

	if (w == WI_new_x_pos_value)
		XtSetArg(args[0], XtNx, a_position);
	else if (w == WI_new_y_pos_value)
		XtSetArg(args[0], XtNy, a_position);
	else if (w == WI_new_width_value)
		XtSetArg(args[0], XtNwidth, a_dimension);
	else if (w == WI_new_height_value)
		XtSetArg(args[0], XtNheight, a_dimension);
	else if (w == WI_new_border_value)
		XtSetArg(args[0], XtNborderWidth, a_dimension);
	else
		XtError("new_dimension_change() called by unknown widget");

	fw = (Widget)client_data;
	XtSetArg(args[1], XtNallowShellResize, TRUE);
	XtSetValues(fw, (ArgList)args, 2);
	XtSetArg(args[0], XtNallowShellResize, FALSE);
	XtSetValues(fw, (ArgList)args, 1);
}


/* called when the current feedback widget is deleted */
/* ARGSUSED */
static void
remove_new_callbacks(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XtRemoveCallback(WI_new_x_pos_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtRemoveCallback(WI_new_y_pos_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtRemoveCallback(WI_new_height_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtRemoveCallback(WI_new_width_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtRemoveCallback(WI_new_border_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
}


/***** Exported Routines *****/


void
RemoveInstanceCallbacks(cl)
ObjectPtr cl;
{
	Arg args[1];

	if (cl == NULL)
		return;

	XtRemoveCallback(WI_x_pos_label, XtNcallback, lock_change,
							(XtPointer)cl);
	XtRemoveCallback(WI_y_pos_label, XtNcallback, lock_change,
							(XtPointer)cl);
	XtRemoveCallback(WI_height_label, XtNcallback, lock_change,
							(XtPointer)cl);
	XtRemoveCallback(WI_width_label, XtNcallback, lock_change,
							(XtPointer)cl);

	XtRemoveCallback(WI_x_pos_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtRemoveCallback(WI_y_pos_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtRemoveCallback(WI_height_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtRemoveCallback(WI_width_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtRemoveCallback(WI_border_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	if (!cl->is_remote && ((cl->parent != NULL
	    && IsCompositeClass(GetClass(cl->parent->class))) ||
	    IsShellClass(GetClass(cl->class)))) {
		XtVaSetValues(WI_instance_managed, XtNlabel, "Managed", NULL);
		XtRemoveCallback(WI_instance_managed, XtNcallback,
					change_managed, (XtPointer)cl);
	} else
		XtSetSensitive(WI_instance_managed, TRUE);
	(void)ObjectRemoveCallback(cl, XtNdestroyCallback, unlink_dying_widget,
								(XtPointer)cl);
	XtSetArg(args[0], XtNtoggleAddress, NULL);
	XtSetValues(WI_instance_managed, (ArgList)args, 1);
}


void
EditInstanceCallbacks(cl)
ObjectPtr cl;
{
	XtAddCallback(WI_x_pos_label, XtNcallback, lock_change, (XtPointer)cl);
	XtAddCallback(WI_y_pos_label, XtNcallback, lock_change, (XtPointer)cl);
	XtAddCallback(WI_height_label, XtNcallback, lock_change, (XtPointer)cl);
	XtAddCallback(WI_width_label, XtNcallback, lock_change, (XtPointer)cl);

	XtAddCallback(WI_x_pos_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtAddCallback(WI_y_pos_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtAddCallback(WI_height_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtAddCallback(WI_width_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	XtAddCallback(WI_border_value, XtNvalueCallback, dimension_change,
								(XtPointer)cl);
	if (!cl->is_remote && (IsCompositeClass(GetClass(cl->parent->class)) ||
	    IsShellClass(GetClass(cl->class)))) {
		XtVaSetValues(WI_instance_managed, XtNtoggleAddress,
				&cl->managed,
				XtNlabel, IsShellClass(XtClass(cl->instance))
				? "Popup" : "Managed", NULL);
		XtAddCallback(WI_instance_managed, XtNcallback, change_managed,
								(XtPointer)cl);
	} else
		XtVaSetValues(WI_instance_managed, XtNsensitive, FALSE,
						XtNstate, FALSE, NULL);

	XtVaSetValues(WI_instance_deferred, XtNtoggleAddress,
						&cl->deferred_creation, NULL);
	(void)ObjectAddCallback(cl, XtNdestroyCallback, unlink_dying_widget,
								(XtPointer)cl);
}


void
AddEventHandlers(cl)
ObjectPtr cl;
{
	if (cl->is_remote || !XtIsWidget(cl->instance))
		return; /* ||| */

	/* called when the window is mapped to update the geometry values */
	XtAddEventHandler(cl->instance, StructureNotifyMask, FALSE,
				(XtEventHandler)config_values, (XtPointer)cl);
	XtAddCallback(cl->instance, XtNdestroyCallback, remove_event_handlers,
								(XtPointer)cl);
}


void
AddAppEventHandlers(app)
ObjectPtr app;
{
#define LOCK(w,f) XtVaSetValues(w, XtNstate, \
	(Boolean)((app->resource_flags & (f)) == (f)), NULL)

	XtAddCallback(WI_app_x_pos_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtAddCallback(WI_app_y_pos_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtAddCallback(WI_app_height_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtAddCallback(WI_app_width_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);
	XtAddCallback(WI_app_border_value, XtNvalueCallback,
					app_dimension_change, (XtPointer)app);

	XtAddCallback(WI_app_x_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);
	XtAddCallback(WI_app_y_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);
	XtAddCallback(WI_app_width_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);
	XtAddCallback(WI_app_height_locked_toggle, XtNcallback,
					app_lock_change, (XtPointer)app);

	LOCK(WI_app_x_locked_toggle, X_RES_LOCKED);
	LOCK(WI_app_y_locked_toggle, Y_RES_LOCKED);
	LOCK(WI_app_width_locked_toggle, WIDTH_RES_LOCKED);
	LOCK(WI_app_height_locked_toggle, HEIGHT_RES_LOCKED);

	if (app->is_remote)
		return; /* ||| */
	XtAddEventHandler(app->instance, StructureNotifyMask, FALSE,
			  (XtEventHandler)app_config_values, (XtPointer)app);
	XtAddCallback(app->instance, XtNdestroyCallback, remove_app_callbacks,
							(XtPointer)app);
#undef LOCK
}


void
AddNewDimensionHandlers(w)
Widget w;
{
	XtAddCallback(WI_new_x_pos_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtAddCallback(WI_new_y_pos_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtAddCallback(WI_new_height_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtAddCallback(WI_new_width_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtAddCallback(WI_new_border_value, XtNvalueCallback,
				new_dimension_change, (XtPointer)w);
	XtAddCallback(w, XtNdestroyCallback, remove_new_callbacks,
							(XtPointer)w);
}
