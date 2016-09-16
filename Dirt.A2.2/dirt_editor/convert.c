#ifndef lint
static char sccsid[] = "@(#)convert.c	1.4 (UKC) 5/17/92";
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

#include <stdio.h>
#include <ctype.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/Toggle.h>
#include <X11/Xukc/Value.h>
#include <X11/Xukc/RowCol.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Converters.h>
#ifdef MOTIF_WIDGETS
#include <X11/Xukc/MotifConv.h>
#endif /* MOTIF_WIDGETS */
#include "dirt_structures.h"
#include "conv_rep.h"

#define PRIMARY_FONT "PRIMARY_FONT"
#define PRIMARY_COLOR "PRIMARY_COLOR"

#define NULL_WIDGET_NAME "NULL"

extern ObjectPtr SearchRecords();
extern Boolean NoObjectsCreated();
extern Window XukcSelectWindow();
extern Widget ObjectWindowToWidget();
extern ObjectPtr NearestWidgetObject();
extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern void DrainUIEventQueue();
extern Boolean NeedsRemoteConversion();
extern void XukcChangeLabel();
extern void XukcCopyToArgVal();
/* in translations.c */
extern void PopupTranslations();
/* in wedit.c */
extern void NewWidgetValue();
extern void CreateEditWidget();
/* in resources.c */
extern void UpdateSimpleTextResource();
extern void NewResourceValue();
extern void SaveInitialValue();
extern XtResourceList GetAResource();
/* in list.c */
extern Boolean IsReadOnly();
extern Cardinal FindClass();
/* in log.c */
extern String UniqueWidgetName();
extern void LogResourceSetting();

extern Widget WI_main_layout; /* ||| used by DoObjectConversion() ||| */

static void remove_destroy_callback();
Boolean IsStringType();
static void RemoveDyingWidgetIdUse();
void RememberWidgetIdUse();
void ForgetOldWidgetIdUse();
static void ForgetWidgetId();

static Boolean conv_inited = FALSE;


/***************************************************************************
 **** WARNING: The following routines need to be added to whenever new  ****
 ****          resource types are added via new widget classes/sets   	****
 ****          For each new type the following routine must be added:  	****
 ****                                                                  	****
 ****  i)  Current Value Display routine.				****
 ****									****
 ****	    This routine takes 7 arguments :-				****
 ****		     Widget dpw; (id of parent to put new widget in)	****
 ****                Widget id; (the widget the resource is in)		****
 ****		     XtResourceList entry; (full definition of resource)****
 ****                Cardinal i; (index into rep_defaults[] for type)	****
 ****                XrmValue *val; (place for current value)		****
 ****		     XtPointer entry_ptr; (entry in dirt resource table)****
 ****		     Boolean read_only; (TRUE == non editable)          ****
 ****									****
 ****	    It returns a Widget id. of a new created widget that	****
 ****	    displays the current value of the resource type in a	****
 ****	    <suitable> way .. it must allow the user to edit the value.	****
 ****	    Any new value should be notified to the program via a	****
 ****	    call to :-							****
 ****		void							****
 ****		NewResourceValue(w, client_data, call_data)		****
 ****		Widget w;						****
 ****		XtPointer client_data, call_data;			****
 ****       which can be used as a callback procedure.  The client_data ****
 ****	    must be set to the "entry_ptr" in the current resource list.****
 ****	    The call_data must be set to the new value, in the proper   ****
 ****	    representing type.						****
 ****									****
 ****	    Any used memory or created resources should be all removed  ****
 ****       when the returned widget is destroyed (which it will be !-) ****
 ****									****
 ***************************************************************************/

/***************************************************************************
 *  All new added resource conversion routines added here please vvvvvvvvv */

/*  End of added resource conversion/display routines ^^^^^^^^^^^^^^^^^^^^ *
 ***************************************************************************/

static String
get_string_from_value(object, entry, v)
ObjectPtr object;
XtResourceList entry;
XrmValue *v;
{
	XrmValue to;

	if (v->addr == NULL)
		return NULL;

	to.size = 0;
	to.addr = NULL;
	if (DoObjectConversion(object, entry->resource_type, v, XtRString, &to)) {
		if (to.addr != NULL)
			to.addr = XtNewString(to.addr);
		return ((String)to.addr);
	}
	return NULL;
}


/* ARGSUSED */
void
FreeMemoryCB(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XtFree((char *)client_data);
}


/********************* built-in display routines *************************/

/* ARGSUSED */
static Widget display_translations(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Widget ret_w;
	String *pass_on;

	if (read_only || IsReadOnly(object, entry))
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));
	ret_w = XtVaCreateManagedWidget("transButton", ukcCommandWidgetClass,
							dpw, NULL);
	pass_on = (String *)XtMalloc(sizeof(String) * 4);
	pass_on[0] = (String)entry_ptr;
	pass_on[1] = (String)object;
	pass_on[2] = entry->resource_name;
	if (object->instance != dpw)
		pass_on[3] = NULL;
	else
		pass_on[3] = (String)(*(XtTranslations *)val->addr);

	XtAddCallback(ret_w, XtNcallback,
			PopupTranslations, (XtPointer)pass_on);
	XtAddCallback(ret_w, XtNdestroyCallback,
			FreeMemoryCB, (XtPointer)pass_on);
	return (Widget)ret_w;
}


/* ARGSUSED */
static void
update_label_entry(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XukcNewTextWidgetString((Widget)client_data, (String)call_data);
}


/* ARGSUSED */
static void
remove_edit_callbacks(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XtRemoveAllCallbacks((Widget)client_data, XtNeditCallback);
	XtRemoveCallback((Widget)client_data, XtNdestroyCallback,
				remove_destroy_callback, (XtPointer)w);
}


/* ARGSUSED */
static void
remove_destroy_callback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XtRemoveCallback((Widget)client_data, XtNdestroyCallback,
				remove_edit_callbacks, (XtPointer)w);
}


/* ARGSUSED */
static Widget display_string(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	static String strTrans = "<LeaveWindow>: dirtACTChangeBorderColor() dirtACTUpdateTextResource(%d) \n\
		  <EnterWindow>: dirtACTChangeBorderColor(set)";
	char tmp[200];
	String str;
	Widget edit_w;


/* ||| leakage test ||| */
#ifndef TEST_TEXT_LEAKAGE
	if (read_only || IsReadOnly(object, entry))
#endif
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));

	(void)sprintf(tmp, strTrans, entry_ptr);
	str = get_string_from_value(object, entry, val);
	edit_w = XtCreateManagedWidget("stringEdit", asciiTextWidgetClass,
							dpw, (ArgList)NULL, 0);
	if (str != NULL)
		XukcNewTextWidgetString(edit_w, str);
	XtOverrideTranslations(edit_w, XtParseTranslationTable(tmp));
	if (str != NULL)
		XtFree(str);

	/* a special case : just for UKCLabel/Command/Toggle widgets
	 * which allow editing of the name directly .. so trap any updates. */
	if (!object->is_remote &&
	    XtIsSubclass(object->instance, ukcLabelWidgetClass) &&
	    strcmp(entry->resource_name, XtNlabel) == 0) {
			XtAddCallback(object->instance, XtNeditCallback,
					UpdateSimpleTextResource, entry_ptr);
			XtAddCallback(object->instance, XtNeditCallback,
					update_label_entry, edit_w);
			XtAddCallback(object->instance, XtNdestroyCallback,
					remove_destroy_callback,
					(XtPointer)edit_w);
			XtAddCallback(edit_w, XtNdestroyCallback,
					remove_edit_callbacks,
					(XtPointer)object->instance);
	}

	return (Widget)edit_w;
}


/* ARGSUSED */
static Widget display_callbacks(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	static String strTrans = "<KeyPress>Return: dirtACTUpdateCallbackResource(%d) \n\
		  <LeaveWindow>: dirtACTChangeBorderColor() dirtACTUpdateCallbackResource(%d) \n\
		  <EnterWindow>: dirtACTChangeBorderColor(set)";
	char tmp[200];
	String str;
	Widget edit_w;

/* ||| remove ||| */
#ifndef TEST_TEXT_LEAKAGE
	if (read_only || IsReadOnly(object, entry))
#endif
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));

	(void)sprintf(tmp, strTrans, entry_ptr, entry_ptr);
	str = get_string_from_value(object, entry, val);
	edit_w = XtCreateManagedWidget("callbackEdit", asciiTextWidgetClass,
							dpw, (ArgList)NULL, 0);
	if (str != NULL)
		XukcNewTextWidgetString(edit_w, str);
	XtOverrideTranslations(edit_w, XtParseTranslationTable(tmp));
	if (str != NULL)
		XtFree(str);

	return (Widget)edit_w;
}


/* ARGSUSED */
static Widget display_boolean(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Widget tw;

	if (read_only || IsReadOnly(object, entry))
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));

	tw = XtVaCreateManagedWidget("booleanToggle", ukcToggleWidgetClass,
					dpw,
					XtNstate, *(Boolean *)val->addr,
					NULL);
	XtAddCallback(tw, XtNcallback, NewResourceValue, entry_ptr);
	return (Widget)tw;
}


/* ARGSUSED */
static Widget display_unsigned_value(dpw, object, entry, i, val,
						entry_ptr, read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Arg args[1];
	Widget vw;

/* ||| remove ||| */
#ifndef TEST_TEXT_LEAKAGE
	if (read_only || IsReadOnly(object, entry))
#endif
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));
	args[0].name = XtNvalue;
	if (val->size == sizeof(unsigned int))
		args[0].value = *(unsigned int *)val->addr;
	else if (val->size == sizeof(unsigned short))
		args[0].value = *(unsigned short *)val->addr;
	else if (val->size == sizeof(unsigned char))
		args[0].value = *(unsigned char *)val->addr;
	else
		XukcCopyToArgVal(val->addr, &(args[0].value), val->size);

	vw = XtCreateManagedWidget("editableUnsignedValue", ukcValueWidgetClass,
							dpw, (ArgList)args, 1);
	XtAddCallback(vw, XtNvalueCallback, NewResourceValue, entry_ptr);
	return (Widget)vw;
}


/* ARGSUSED */
static Widget display_signed_value(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Arg args[1];
	Widget vw;

/* ||| remove ||| */
#ifndef TEST_TEXT_LEAKAGE
	if (read_only || IsReadOnly(object, entry))
#endif
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));
	args[0].name = XtNvalue;
	if (val->size == sizeof(int))
		args[0].value = *(int *)val->addr;
	else if (val->size == sizeof(short))
		args[0].value = *(short *)val->addr;
	else if (val->size == sizeof(char))
		args[0].value = *(char *)val->addr;
	else
		XukcCopyToArgVal(val->addr, &(args[0].value), val->size);

	vw = XtCreateManagedWidget("editableSignedValue", ukcValueWidgetClass,
							dpw, (ArgList)args, 1);
	XtAddCallback(vw, XtNvalueCallback, NewResourceValue, entry_ptr);
	return (Widget)vw;
}


/* ARGSUSED */
static Widget display_enumerated(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Widget bw, tw;
	Cardinal j;
	EnumList eptr;
	Boolean state;

	if (rep_defaults[i].enum_array == NULL ||
	    IsReadOnly(object, entry))
		return (display_read_only(dpw, object, entry, i, val,
						entry_ptr, read_only));

	bw = XtCreateManagedWidget("enumeratedBox",
					ukcRowColWidgetClass,
					dpw, (ArgList)NULL, 0);
	eptr = rep_defaults[i].enum_array;
	for (j = 0, tw = NULL; j < rep_defaults[i].num_enums; j++) {
		switch (val->size) {
			case sizeof(char):
				state = *(char *)val->addr ==
							(char)eptr[j].value;
				break;
			case sizeof(short):
				state = *(short *)val->addr ==
							(short)eptr[j].value;
				break;
			case sizeof(int):
				state = *(int *)val->addr == eptr[j].value;
				break;
		}
			
		tw = XtVaCreateManagedWidget("enumeratedToggle",
				ukcToggleWidgetClass, bw,
				XtNradioToggle, tw,
				XtNlabel, eptr[j].name,
				XtNradioValue, eptr[j].value,
				XtNstate, state,
				NULL);
		XtAddCallback(tw, XtNcallback, NewResourceValue, entry_ptr);
		if (read_only)
			XtUninstallTranslations(tw);
	}
	return (Widget)bw;
}


/* ARGSUSED */
static Widget display_list(dpw, object, entry, i, val, entry_ptr, read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
#if 1
	return (display_read_only(dpw, object, entry, i, val,
						entry_ptr, read_only));
#else
	Widget ret_w;

	if (IsReadOnly(object, entry))
		return (display_read_only(dpw, object, entry, i, val,
						entry_ptr, read_only));
	return (Widget)ret_w;
#endif
}


static void
chain_destroy(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (*(int *)client_data == -1)
		XtFree(client_data);
}


/* ARGSUSED */
static Widget display_widget_list(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Widget ret_w, *edit_widget, *label_widget, ew = NULL;
	WidgetList wl = NULL;
	XtPointer *pass_on;
	int *been_created, j;
	Cardinal num_wl = 0;
	Arg args[1];

	if (val->addr != NULL)
		wl = *(WidgetList *)val->addr;

	if (wl == NULL)
		return ((Widget)XtVaCreateManagedWidget("readOnly",
			ukcLabelWidgetClass, dpw, XtNlabel, "NULL", NULL));

	/* We have to do this because we don't know the length of WidgetList */
	XtSetArg(args[0], XtNnumChildren, &num_wl);
	(void)ObjectGetValues(object, (ArgList)args, 1, FALSE, FALSE);

	ret_w = XtVaCreateManagedWidget("widgetListBox",
					ukcRowColWidgetClass,
					dpw, NULL);

	for (j = 0; j < num_wl; j++) {
		String name;

		if (!object->is_remote && strncmp(XtName(wl[j]), "_dirt", 5) == 0)
			/* ignore internally added children */
			continue;
		name = UniqueWidgetName(wl[j]);
		ew = XtVaCreateManagedWidget("widgetButton",
					ukcCommandWidgetClass,
					ret_w, XtNlabel, name, NULL);
		XtFree(name);
		been_created = XtNew(int);
		edit_widget = XtNew(Widget);
		label_widget = XtNew(Widget);
		/* we don't want to be able to edit these */
		*edit_widget = NULL;
		pass_on = (XtPointer *)XtMalloc(sizeof(XtPointer) * 8);
		pass_on[0] = (XtPointer)object;
		pass_on[1] = (XtPointer)entry;
		pass_on[2] = (XtPointer)wl[j];
		pass_on[3] = (XtPointer)been_created;
		pass_on[4] = entry_ptr;
		pass_on[5] = (XtPointer)edit_widget;
		pass_on[6] = (XtPointer)label_widget;
		pass_on[7] = (XtPointer)ew;
		XtAddCallback(ew, XtNcallback, CreateEditWidget,
							(XtPointer)pass_on);
		XtAddCallback(ew, XtNdestroyCallback, FreeMemoryCB,
							(XtPointer)pass_on);
		XtAddCallback(ew, XtNdestroyCallback, chain_destroy,
						(XtPointer)been_created);
		XtAddCallback(ew, XtNdestroyCallback, FreeMemoryCB,
						(XtPointer)edit_widget);
		XtAddCallback(ew, XtNdestroyCallback, FreeMemoryCB,
						(XtPointer)label_widget);
		*been_created = -1;
	}

	return (Widget)ret_w;
}


/**************** Widget Resources *****************/

void
PickAnObject(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	/* allow the user to interactively choose an object to be given
	 * as the setting for a Widget type resource.
	 */
	Widget picked = NULL;
	Window win;
	ObjectPtr selected;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "pickAnObject",
				"No Objects To Pick", 0);
		return;
	}

	win = XukcSelectWindow(GetHeadObject()->instance);

	selected = GetStartOfList(TRUE);
	if (selected != NULL)
		picked = ObjectWindowToWidget(selected, win);
	if (picked == NULL && (selected = GetStartOfList(FALSE)) != NULL)
		picked = ObjectWindowToWidget(selected, win);

	if (picked != NULL) {
		selected = NearestWidgetObject(picked);
		if (selected != NULL) {
			XtVaSetValues(w, XtNlabel, selected->name,
					 XtNediting, TRUE,
					 NULL);
		}
	}
}


static void
update_widget_resource(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Arg args[1];
	XtPointer *pass_on = (XtPointer *)client_data;
	ObjectPtr obj = (ObjectPtr)pass_on[0];
	XtResourceList entry = (XtResourceList)pass_on[1];
	int been_created = *(int *)pass_on[3];
	XtPointer entry_ptr = pass_on[4];
	Widget ew = *(Widget *)pass_on[5];
	Widget lw = *(Widget *)pass_on[6];
	Widget nw;
	XrmValue from, to;
	String new_name = (String)call_data;
	String old_name;

	if (client_data == NULL)
		return; /* ||| should this be an error ? ||| */

	from.addr = new_name;
	from.size = strlen(new_name);
	to.addr = (XtPointer)&nw;
	to.size = sizeof(Widget);

	/* get original value for use */
	XtSetArg(args[0], entry->resource_name, NULL);
	ObjectGetValues(obj, (ArgList)args, 1, TRUE, FALSE);
	old_name = XtNewString((String)args[0].value);

	if (*new_name == '\0' || strcmp(new_name, "NULL") == 0) {
		/* special name denoting a NULL Widget reference */
		NewResourceValue(w, pass_on[4], (XtPointer)NULL);
		if (been_created > -1)
			NewWidgetValue(ew, lw, obj, (Widget)NULL,
					(Widget)pass_on[7],
					(struct resource_entry *)entry_ptr,
!                                       (Cardinal)been_created, TRUE);
	} else if (ObjectConversion(obj, XtRString, &from,
	    entry->resource_type, &to)) {
		NewResourceValue(w, entry_ptr, (XtPointer)*(Widget *)to.addr);
		if (been_created > -1)
			NewWidgetValue(ew, lw, obj, (Widget)NULL,
					(Widget)pass_on[7],
					(struct resource_entry *)entry_ptr,
					(Cardinal)been_created, TRUE);
	} else {
		/* conversion failed so display the original value */
		XtVaSetValues(w, XtNlabel, old_name, NULL);
		XtFree(old_name);
		return;
	}

	if (!obj->is_remote) {
		Widget widget_id;

		widget_id = XtNameToWidget(XtParent(obj->instance), old_name);

		ForgetOldWidgetIdUse(obj->instance, entry->resource_name,
					widget_id);
		XtVaGetValues(obj->instance, entry->resource_name,
					&widget_id, NULL);
		RememberWidgetIdUse(obj->instance, entry->resource_name,
					widget_id);
	}
	XtFree(old_name);
#ifdef XAW
#include <X11/Xaw/Form.h>
	/* gross hack for coping with the Athena Form Widget */
	if (obj->parent->class == FindClass(formWidgetClass)
	    && XtIsRectObj(obj->instance)) {
		/* This call ought to work but it doesn't .. more junk code! */
		/* XawFormDoLayout(obj->parent->instance, TRUE); */
		XtUnmanageChild(obj->instance);
		XtManageChild(obj->instance);
		DrainUIEventQueue();
	}
#endif /* XAW */
}


/* ARGSUSED */
static Widget display_widget(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Widget ret_w, *edit_widget, *label_widget, ew = NULL;
	XtPointer *pass_on;
	int *been_created;
	Boolean not_read_only;
	String name;

	not_read_only = !(IsReadOnly(object, entry) || read_only);
	if (val->addr != NULL)
		ew = *(Widget *)val->addr;
	name = (ew == NULL) ? XtNewString("NULL") : UniqueWidgetName(ew);

	ret_w = XtVaCreateManagedWidget("widgetButton", ukcCommandWidgetClass,
					dpw, XtNlabel, name,
					XtNeditable, not_read_only,
					NULL);
	XtFree(name);

	/* shrink the label if it is read-only .. do this here to
	 * override the defaults in Dirt.h */
	XtVaSetValues(ret_w, XtNresize, !not_read_only, NULL);

	been_created = XtNew(int);
	edit_widget = XtNew(Widget);
	label_widget = XtNew(Widget);
	*edit_widget = (Widget)not_read_only;
	pass_on = (XtPointer *)XtMalloc(sizeof(XtPointer) * 8);
	pass_on[0] = (XtPointer)object;
	pass_on[1] = (XtPointer)entry;
	pass_on[2] = (XtPointer)NULL;  /* Widget in WidgetList or NULL */
	pass_on[3] = (XtPointer)been_created;
	pass_on[4] = entry_ptr;
	pass_on[5] = (XtPointer)edit_widget;
	pass_on[6] = (XtPointer)label_widget;
	pass_on[7] = (XtPointer)ret_w;
	
	XtAddCallback(ret_w, XtNcallback, CreateEditWidget,
							(XtPointer)pass_on);
	if (not_read_only)
		XtAddCallback(ret_w, XtNeditCallback, update_widget_resource,
							(XtPointer)pass_on);
	XtAddCallback(ret_w, XtNdestroyCallback, FreeMemoryCB,
							(XtPointer)pass_on);
	XtAddCallback(ret_w, XtNdestroyCallback, chain_destroy,
						(XtPointer)been_created);
	XtAddCallback(ret_w, XtNdestroyCallback, FreeMemoryCB,
						(XtPointer)edit_widget);
	XtAddCallback(ret_w, XtNdestroyCallback, FreeMemoryCB,
						(XtPointer)label_widget);
	*been_created = -1;

	return (Widget)ret_w;
}


/****************** Font Resources *****************/

/* ARGSUSED */
static void
show_new_font(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XtArgVal new_font_str;

	if ((Boolean)client_data && call_data != NULL)
		/* we have the new font name in call_data */
		new_font_str = (XtArgVal)call_data;
	else
		XtVaGetValues(w, XtNlabel, &new_font_str, NULL);

	XtVaSetValues(w, XtVaTypedArg, XtNfont, XtRString,
				new_font_str, STRLEN((char *)new_font_str),
				NULL);
}


/* ARGSUSED */
static void
upload_selection_value(w, client_data, selection, type, value, length, format)
Widget w;
XtPointer client_data;
Atom *selection, *type;
XtPointer value;
unsigned long *length;
int *format;
{
	if (*type == XT_CONVERT_FAIL || value == NULL)
		return;

	XukcChangeLabel(w, (String)value, FALSE);
	UpdateSimpleTextResource(w, client_data, (XtPointer)NULL);
	show_new_font(w, (XtPointer)TRUE, (XtPointer)value);
}


/* ARGSUSED */
static void
get_font_selection(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	static Atom _XA_PRIMARY_FONT = NULL;

	if (_XA_PRIMARY_FONT == NULL)
		_XA_PRIMARY_FONT = XInternAtom(XtDisplay(w),
						PRIMARY_FONT, FALSE);

	XtGetSelectionValue(w, _XA_PRIMARY_FONT,
				XA_STRING, upload_selection_value,
				(XtPointer)client_data,
				XtLastTimestampProcessed(XtDisplay(w)));
}


/* ARGSUSED */
static Widget display_font(dpw, object, entry, i, val, entry_ptr,
				read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	String str;
	Widget rw;

	if (IsReadOnly(object, entry))
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));
	str = get_string_from_value(object, entry, val);
	if (str == NULL)
		str = XtNewString("-misc-fixed-bold-r-normal-*-15-*-*-*-c-*-iso8859-1");
	rw = XtVaCreateManagedWidget("fontButton", ukcCommandWidgetClass, dpw,
					XtNlabel, str,
					XtVaTypedArg, XtNfont, XtRString,
					str, STRLEN(str), NULL);
	XtAddCallback(rw, XtNeditCallback, UpdateSimpleTextResource, entry_ptr);
	XtAddCallback(rw, XtNeditCallback, show_new_font, (XtPointer)TRUE);
	XtAddCallback(rw, XtNcallback, get_font_selection, entry_ptr);

	XtFree(str);
	return (Widget)rw;
}


/*********** Colour Resources **********/

static void
upload_selected_colour(w, client_data, selection, type, value, length, format)
Widget w;
XtPointer client_data;
Atom *selection, *type;
XtPointer value;
unsigned long *length;
int *format;
{
	if (*type == XT_CONVERT_FAIL || value == NULL)
		return;

	XukcChangeLabel(w, (String)value, FALSE);
	UpdateSimpleTextResource(w, client_data, (XtPointer)NULL);
}


static void
get_colour_selection(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	static Atom _XA_PRIMARY_COLOR = NULL;

	if (_XA_PRIMARY_COLOR == NULL)
		_XA_PRIMARY_COLOR = XInternAtom(XtDisplay(w),
						PRIMARY_COLOR, FALSE);

	XtGetSelectionValue(w, _XA_PRIMARY_COLOR,
				XA_STRING, upload_selected_colour,
				(XtPointer)client_data,
				XtLastTimestampProcessed(XtDisplay(w)));
}


/* ARGSUSED */
static Widget display_colour(dpw, object, entry, i, val, entry_ptr,
					read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	String str;
	Widget rw;

	if (IsReadOnly(object, entry))
		return (display_read_only(dpw, object, entry, i, val,
							entry_ptr, read_only));
	str = get_string_from_value(object, entry, val);
	if (str == NULL)
		str = XtNewString(XtDefaultForeground);

	rw = XtVaCreateManagedWidget("colourButton", ukcCommandWidgetClass, dpw,
					XtNlabel, str, NULL);
	XtAddCallback(rw, XtNeditCallback, UpdateSimpleTextResource, entry_ptr);
	XtAddCallback(rw, XtNcallback, get_colour_selection, entry_ptr);

	XtFree(str);
	return (Widget)rw;
}

/****** ReadOnly resources ******/

/* ARGSUSED */
static Widget display_read_only(dpw, object, entry, i, val, entry_ptr,
								read_only)
Widget dpw;
ObjectPtr object;
XtResourceList entry;
Cardinal i;
XrmValue *val;
XtPointer entry_ptr;
Boolean read_only;
{
	Arg args[1];
	String str;
	Widget rw;
	Cardinal j;

	str = get_string_from_value(object, entry, val);
	j = 0;
	if (str != NULL) {
		XtSetArg(args[j], XtNlabel, str); j++;
	}
	rw = XtCreateManagedWidget("readOnly", ukcLabelWidgetClass,
							dpw, (ArgList)args, j);
	if (str != NULL)
		XtFree(str);
	return (Widget)rw;
}


/***************************************************************************/


static void
initialize_convert()
{
	Cardinal i = XtNumber(rep_defaults);

	for (; i > 0; i--)
		rep_defaults[i-1].quark
			= XrmStringToQuark(rep_defaults[i-1].type);
	conv_inited = TRUE;
}


static Cardinal
find_conversion(resource_type)
String resource_type;
{
	Cardinal i;
	XrmQuark to_quark = XrmStringToQuark(resource_type);

	for (i = 0; i < XtNumber(rep_defaults) && to_quark != rep_defaults[i].quark;)
		i++;
#ifdef DEBUG
	if (i == XtNumber(rep_defaults))
		printf("missing resource type : XtR%s\n", resource_type);
#endif /* DEBUG */
	return i;
}


/******** Module Exported Routines ********/


Boolean
ConversionExists(resource_type)
String resource_type;
{
	if (!conv_inited)
		initialize_convert();

	return (find_conversion(resource_type) < XtNumber(rep_defaults));
}


/* thin layer on top of XtConvertAndStore() */
Boolean
DoConversion(w, from_type, from, to_type, to)
Widget w;
String from_type;
XrmValue *from;
String to_type;
XrmValue *to;
{
	if (!conv_inited)
		initialize_convert();

	if (strcmp(from_type, to_type) == 0 &&
	    strcmp(from_type, XtRString) == 0 && from->addr == NULL) {
		to->addr = (XtPointer)"";
		to->size = 1;
		return TRUE;
	}

	/* if "w" is an object not a widget, find its nearest
	 * widget ancestor */
	if (!XtIsWidget(w)) {
		/* ||| HAZARD: semi-public routine in Xt/Intrinsic.c ||| */
		extern Widget _XtWindowedAncestor();

		w = _XtWindowedAncestor(w);
	}

	if (w == NULL) {
		/* ||| change message ||| */
		XtWarning("DIRT internal error: found object with no widget ancestor in DoConversion()");
		abort(1);
	}

	return (XtConvertAndStore(w, from_type, from, to_type, to));
}


Boolean
DoObjectConversion(object, from_type, from, to_type, to)
ObjectPtr object;
String from_type;
XrmValue *from;
String to_type;
XrmValue *to;
{
	if (object->is_remote) {
		if (NeedsRemoteConversion(from_type, to_type))
			return (ObjectConversion(object, from_type, from,
								to_type, to));
		else /* quicker to do this locally */
			return (DoConversion(WI_main_layout, from_type, from,
								to_type, to));
	}
	return (DoConversion(object->instance, from_type, from, to_type, to));
}


Widget
EditableResource(resource_table, object, class, entry, current_value,
						entry_ptr, force_read_only)
Widget resource_table;
ObjectPtr object;
Cardinal class;
XtResourceList entry;
XtPointer current_value;
XtPointer entry_ptr;
Boolean force_read_only;
{
	Cardinal i;
	XrmValue val;
	Widget retw = NULL;

	if (!conv_inited)
		initialize_convert();

	i = find_conversion(entry->resource_type);

	/* no string conversion found so generate an error */
	if (i == XtNumber(rep_defaults))
		XtError("unable to find conversion in EditableResource()");
		/* should Never happen */

	if (IsStringType(entry->resource_type)) {
		val.addr = *(String *)current_value;

		if (val.addr == NULL) {
			val.addr = "";
			val.size = 1;
		} else
			val.size = STRLEN((char *)val.addr);
	} else {
		val.addr = (XtPointer)current_value;
		val.size = entry->resource_size;
	}

	SaveInitialValue(entry_ptr, &val);

	if (rep_defaults[i].display_proc != NULL)
		retw = (*rep_defaults[i].display_proc)(resource_table,
			     object, entry, i, &val, entry_ptr,
			     force_read_only);
	else
		XtError("A .display_proc must exist for each resource type");
	return (retw);
}


Boolean
IsStringType(type)
String type;
{
	if (type == NULL)
		return False;
	return (strcmp(type, XtRString) == 0);
}


/* ARGSUSED */
Boolean
_dirtCvtWidgetToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	static String widget_name = NULL;
	Widget tmp;
	Cardinal i = 0;
	Cardinal size;

        if (*num_args != 0)
		XtAppErrorMsg(XtDisplayToApplicationContext(display),
			"wrongParameters", "cvtWidgetToString",
			"DirtError",
			"Widget to String needs no extra parameters",
			(String *)NULL, (Cardinal *)&i);

	tmp = *(Widget *)fromVal->addr;

	if (widget_name != NULL)
		XtFree(widget_name);

	if (tmp != NULL)
		widget_name = UniqueWidgetName(tmp);
	else
		widget_name = XtNewString("");

	if (widget_name == NULL) /* we shouldn't have caught this */
		return FALSE;

	size = strlen(widget_name) + 1;
	if (toVal->addr != NULL) { 
		if (toVal->size < size) {
			toVal->size = size;
			return FALSE;
		}
		(void)strcpy((char *)toVal->addr, widget_name);
		toVal->size = size;
	} else {
		toVal->size = size;
		toVal->addr = (XtPointer)&widget_name;
	}
	return TRUE;
}


/* ARGSUSED */
Boolean
_dirtCvtStringToWidget(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	static Widget widget = NULL;
	String complex_name;
	Cardinal i = 0;

        if (*num_args != 1)
		XtAppErrorMsg(XtDisplayToApplicationContext(display),
		   "wrongParameters", "cvtStringToWidget", "DirtError",
		   "String to Widget conversion needs parent widget argument",
		   (String *)NULL, (Cardinal *)&i);

	complex_name = (String)fromVal->addr;

	widget = XtNameToWidget(*(Widget *)args[0].addr, complex_name);

	if (widget == NULL && *complex_name != '\0' &&
	    strcmp(complex_name, NULL_WIDGET_NAME) != 0) {
		XtDisplayStringConversionWarning(display, complex_name,
								XtRWidget);
		return (FALSE);
	}

	if (toVal->addr != NULL) { 
		if (toVal->size < sizeof(Widget)) {
			toVal->size = sizeof(Widget);
			return FALSE;
		}
		*(Widget *)(toVal->addr) = widget;
	} else {
		toVal->size = sizeof(Widget);
		toVal->addr = (XtPointer)&widget;
	}
	return (TRUE);
}


/*************************************************************/

/* These routines are used to implement a simple for of constraint
 * management of Widget identifiers.  When a Widget type resource has
 * its current value changed we add callbacks to the destroyCallback
 * lists on both the new Widget and the Widget on which the resource is
 * set.  This is done so that when either Widget is destroyed any invalid
 * use of a now out of date Widget is removed before it is used.  We
 * remove the bad Widget id use by setting the original resource to NULL.
 * Use of these routines stops Widgets like the form widget from using a
 * bad Widget id and thus core dumping!
 */

static void
RemoveDyingWidgetIdUse(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	ObjectPtr obj;
	struct widget_ref *ref = (struct widget_ref *)client_data;
	String res_name = XrmQuarkToString(ref->res_name);

	XtRemoveCallback(ref->wid, XtNdestroyCallback, ForgetWidgetId,
							(XtPointer)w);
	XtVaSetValues(ref->wid, res_name, (Widget)NULL, NULL);
	if ((obj = SearchRecords(GetHeadObject(), ref->wid)) != NULL) {
		Boolean is_constraint;
		XtResourceList res_l;
		int num;

		res_l = GetAResource(obj, res_name, FALSE,
							&num, &is_constraint);
		if (num >= 0)
			LogResourceSetting(obj, res_name, "", is_constraint);
		XtFree(res_l);
	}
	XtFree(ref);
}


void
ForgetOldWidgetIdUse(w, res_name, wid)
Widget w;
String res_name;
Widget wid;
{
	XrmQuark res_quark = XrmStringToQuark(res_name);
	XtCallbackList dcb;

	if (wid == NULL)
		return;

	XtRemoveCallback(w, XtNdestroyCallback, ForgetWidgetId,
							(XtPointer)wid);
	XtVaGetValues(wid, XtNdestroyCallback, &dcb, NULL);
	while (dcb != NULL) {
		if (dcb->callback == RemoveDyingWidgetIdUse) {
			struct widget_ref *new_ref =
					(struct widget_ref *)dcb->closure;
			if (new_ref->wid == w &&
			    new_ref->res_name == res_quark) {
				XtRemoveCallback(wid, XtNdestroyCallback,
					dcb->callback, dcb->closure);
				XtFree(new_ref);
				return;
			}
		}
		dcb++;
	}
}


void
RememberWidgetIdUse(w, res_name, wid)
Widget w;
String res_name;
Widget wid;
{
	struct widget_ref *new_ref;

	if (wid == NULL)
		return;

	new_ref = XtNew(struct widget_ref);
	new_ref->res_name = XrmStringToQuark(res_name);
	new_ref->wid = w;
	XtAddCallback(wid, XtNdestroyCallback, RemoveDyingWidgetIdUse,
			(XtPointer)new_ref);
	XtAddCallback(w, XtNdestroyCallback, ForgetWidgetId,
			(XtPointer)wid);
}


static void
ForgetWidgetId(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Widget wid = (Widget)client_data;
	XtCallbackList dcb;

	XtVaGetValues(wid, XtNdestroyCallback, &dcb, NULL);
	while (dcb != NULL) {
		if (dcb->callback == RemoveDyingWidgetIdUse) {
			struct widget_ref *new_ref =
					(struct widget_ref *)dcb->closure;
			if (new_ref->wid == w) {
				XtRemoveCallback(wid, XtNdestroyCallback,
					dcb->callback, dcb->closure);
				XtFree(new_ref);
				return;
			}
		}
		dcb++;
	}
}
