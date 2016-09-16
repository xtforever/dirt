#ifndef lint
static char sccsid[] = "@(#)create.c	1.2 (UKC) 5/10/92";
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
 * create.c : contains the functions for creating new instances of widgets
 *	      and an application shell.
 */

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xukc/Toggle.h>
#include <X11/Xukc/ICRM.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Med3Menu.h>
#include "dirt_structures.h"
#include "dirt_bindings.h"

#define SETVALUE(w, v)	XtVaSetValues(w, XtNvalue, (v), NULL);

extern struct app_resources editor_resources;

/* UI Widget Tree */
extern Widget WI_dirt;
extern Widget WI_delete_dialog_shell;
extern Widget WI_delete_dialog_object;

/* Application Details */
extern Widget WI_app_new, WI_app_delete;
extern Widget WI_create_name_entry, WI_create_class_entry;
extern Widget WI_app_x_locked_toggle, WI_app_y_locked_toggle;
extern Widget WI_app_height_locked_toggle, WI_app_width_locked_toggle;
extern Widget WI_app_x_pos_value, WI_app_y_pos_value;
extern Widget WI_app_height_value, WI_app_width_value;
extern Widget WI_app_border_value, WI_app_resources_viewport;

/* Create Widget Instance */
extern Widget WI_new_create;
extern Widget WI_new_managed, WI_new_deferred;
extern Widget WI_new_name_entry, WI_new_class_entry;
extern Widget WI_new_parent_entry;
extern Widget WI_new_x_pos_label, WI_new_y_pos_label;
extern Widget WI_new_height_label, WI_new_width_label;
extern Widget WI_new_x_pos_value, WI_new_y_pos_value;
extern Widget WI_new_height_value, WI_new_width_value, WI_new_border_value;
extern Widget WI_pick_parent, WI_new_resources_viewport;

/* externally defined routines */
extern String XukcClassName();
extern void XukcChangeLabel();
extern Window XukcEnclosingWindow();
extern Window XukcSelectWindow();
extern void XukcDragBox();
extern void XukcDestroyList();
/* in instance_list.c */
extern void CreateInstanceList();
extern void ClearInstanceList();
/* in list.c */
extern WidgetClass GetClass();
extern void ResetClassChosen();
extern void SetClassChosen();
extern Boolean IsShellClass();
extern Boolean IsSubclass();
extern Boolean AcceptsObjects();
extern String GetDisplayedClassName();
extern Cardinal FindClass();
extern Boolean IsCompositeClass();
/* in geometry.c */
extern void AddAppEventHandlers();
extern void AddEventHandlers();
extern void AddNewDimensionHandlers();
/* in log.c */
extern void LogResourceSetting();
/* in dirt.c */
extern Widget InitializeApplicationShell();
extern void AfterApplicationShellCreated();
extern void DrainUIEventQueue();
extern XtAppContext GetUIContext();
/* in resources.c */
extern ArgList GetArgumentList();
extern void LogPresetArguments();
extern void CreateResourceTable();
/* in remote.c */
extern Widget ObjectWindowToWidget();
/* in selected.c */
extern ObjectPtr NearestWidgetObject();
extern void ChangeSelection();
extern void GridObjectHasDied();
/* in edit.c */
extern String CvtToVariableName();
extern ObjectPtr CurrentlyEditing();
extern void SetCurrentObject();
/* in connect.c */
extern void RemoveDyingWidgetConnections();
/* in copy.c */
extern void GetUniqueName();
/* in hash.c */
extern void AddHashedName();
extern void RemoveHashedName();

extern String XukcCreateMeasurementString();

/* forward declarations of module exported routines routines */
Boolean ObjectInApplication();
ObjectPtr CreateObjectRecord();
Boolean NoObjectsCreated();
Boolean IsValidParent();
void DeleteObject();
void DestroyObjectRecord();
void AddCreatedObject();
ObjectPtr SearchRecords();
ObjectPtr SearchRecordsForName();
ObjectPtr SearchRecordsForMarkNum();
ObjectPtr GetStartOfList();
ObjectPtr GetHeadObject();
void SetStartOfList();
String GetAppClassName();
Cardinal GetClassChosen();
void AddDestroyCallback();
void RealizeAllChildren();
void CheckForZeroDimensions();
void CreateAndRealize();
void DeleteObjectDialog();

/* forward declarations */
static void set_new_parent();
static void recursive_destroy();

char app_class[STRSIZE];	/* application class (global) */
static char app_name[STRSIZE];	/* application name (text widget buffer) */
static char new_name[STRSIZE];	/* new instance name */
static char new_parent[STRSIZE];/* new instance's parent's name */
static ObjectPtr parent_obj = NULL;

static ObjectListRec created_widgets;
static ObjectPtr created_app = NULL;
static ObjectPtr created_template = NULL;

static Boolean loop_condition = TRUE;
static Widget feedback = NULL;

static ObjectPtr error_object = NULL;
static XtErrorMsgHandler error_handler;


static XtErrorMsgHandler
deal_with_zero_dimension_error(name, type, class, defaultp, params, num_params)
String name, type, class, defaultp;
String *params;
Cardinal *num_params;
{
	if (error_object == NULL) {
		/* really an error .. call the proper handler */
		(*error_handler)(name, type, class, defaultp, params,
					num_params);
		exit(1);
	} else if (error_object->instance == NULL) {
		/* error during creation .. try to be intuitive and don't
		 * exit .. this might break though 8-( */
		(*error_handler)(name, type, class, defaultp, params,
					num_params);
		return;
	}

	if (error_object->instance->core.width == 0)
		error_object->instance->core.width =
			error_object->parent->instance->core.width;

	if (error_object->instance->core.height == 0)
		error_object->instance->core.height =
			error_object->parent->instance->core.height;
}


/* called when a widget instance dies to clean remove it from the list */
/* ARGSUSED */
static void
unlink_object(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if ((ObjectPtr)client_data == NULL)
		XtError("unlink_object() called with client_data == NULL");

	RemoveDyingWidgetConnections((ObjectPtr)client_data);
	DestroyObjectRecord((ObjectPtr)client_data);

	/* if the currently selected "new parent" has just been deleted
	 * then clear the "new parent" */
	set_new_parent((ObjectPtr)NULL, new_parent);
	XFlush(XtDisplayOfObject(w));
}


static Widget
find_parent(head, x1, y1, x2, y2)
Widget head;
int x1, y1, x2, y2;
{
	Window ww[5];
	Cardinal votes[5], i, j;
	Widget pw = NULL;
	Position tx1=x1, tx2=x1, ty1=y1, ty2=y2;

	/* find the windows for the 4 points of the dragged out rectangle
	 * plus the centre of the rectangle */
	ww[0] = XukcEnclosingWindow(head, XtScreen(head)->root, tx1, ty1);
	ww[1] = XukcEnclosingWindow(head, XtScreen(head)->root, tx1, ty2);
	ww[2] = XukcEnclosingWindow(head, XtScreen(head)->root, tx2, ty2);
	ww[3] = XukcEnclosingWindow(head, XtScreen(head)->root, tx2, ty1);
	ww[4] = XukcEnclosingWindow(head, XtScreen(head)->root,
							  (tx2-tx1)/2+tx1,
							  (ty2-ty1)/2+ty1);

	votes[0] = votes[1] = votes[2] = votes[3] = votes[4] = 0;

	/* see which points are in the same windows */
	for (i = 0; i < 5; i++)
		for (j = 0; j < 5; j++)
			votes[i] += (ww[i] == ww[j]);

	/* choose the window which contains the most points */
	j = 0;
	for (i = 0; i < 5; i++)
		if (votes[i] >= 3)
			j = i;

	pw = XtWindowToWidget(XtDisplay(head), ww[j]);
	if (pw == NULL)
		return (NULL);

	while (!XtIsSubclass(pw, compositeWidgetClass))
		pw = XtParent(pw);

	return (pw);
}


static void
create_a_widget(name, class, parent, x, y, width, height, border)
String name;
Cardinal class;
ObjectPtr parent;
Position x, y;
Dimension width, height;
Dimension border;
{
#define MEASUREMENT(dim) (editor_resources.pixels ? \
		((void)sprintf(tmp, "%d", dim), (String)tmp) : \
		XukcCreateMeasurementString(XtScreen(cl->parent->instance), \
		(int)dim, !editor_resources.inches))
	char tmp[100];
	ObjectPtr cl;
	Arg args[6];
	ArgList new_args;
	ArgList preset_args;
	Cardinal i, num_preset_args;

	if (!IsValidParent(parent, GetClassChosen())) {
		GiveWarning(WI_dirt, "invalidParent", "createObject",
			"\"%s\" cannot be a Parent to a \"%s\" Object",
			2, parent->name, GetDisplayedClassName(class));
		return;
	}

	cl = CreateObjectRecord();
	cl->class = class;
	cl->managed = TRUE;
	cl->parent = parent;
	cl->deferred_creation = FALSE;
	cl->is_remote = FALSE;
	cl->in_app = ObjectInApplication(cl);
	cl->name = XtNewString(name);
	AddHashedName(cl);
	GetUniqueName(cl, name);
	AddCreatedObject(cl);

	XtVaGetValues(WI_new_managed, XtNstate, &cl->managed, NULL);
	XtVaGetValues(WI_new_deferred, XtNstate, &cl->deferred_creation, NULL);

	preset_args = GetArgumentList(NEW_WIDGET_RESOURCES, &num_preset_args);

	i = 0;

	cl->resource_flags = 0;
	if (x == 0) {
		Boolean state;

		XtVaGetValues(WI_new_x_pos_label, XtNstate, &state, NULL);
		if (state) {
			int pos;

			XtVaGetValues(WI_new_x_pos_value, XtNvalue, &pos, NULL);
			cl->resource_flags |= X_RES_LOCKED;
			cl->x = (Position)pos;
			XtSetArg(args[i], XtNx, (Position)pos); i++;
		}
	} else {
		XtSetArg(args[i], XtNx, x); i++;
		cl->resource_flags |= X_RES_LOCKED;
		cl->x = x;
	}

	if (y == 0) {
		Boolean state;

		XtVaGetValues(WI_new_y_pos_label, XtNstate, &state, NULL);
		if (state) {
			int pos;

			XtVaGetValues(WI_new_y_pos_value, XtNvalue, &pos, NULL);
			cl->resource_flags |= Y_RES_LOCKED;
			cl->y = (Position)y;
			XtSetArg(args[i], XtNy, (Position)pos); i++;
		}
	} else {
		XtSetArg(args[i], XtNy, y); i++;
		cl->resource_flags |= Y_RES_LOCKED;
		cl->y = y;
	}

	if (width == 0) {
		Boolean state;

		XtVaGetValues(WI_new_width_label, XtNstate, &state, NULL);
		if (state) {
			int dim;

			XtVaGetValues(WI_new_width_value, XtNvalue, &dim, NULL);
			cl->resource_flags |= WIDTH_RES_LOCKED;
			cl->width = (Dimension)dim;
			XtSetArg(args[i], XtNwidth, (Dimension)dim); i++;
		}
	} else {
		XtSetArg(args[i], XtNwidth, width); i++;
		cl->resource_flags |= WIDTH_RES_LOCKED;
		cl->width = width;
	}

	if (height == 0) {
		Boolean state;

		XtVaGetValues(WI_new_height_label, XtNstate, &state, NULL);
		if (state) {
			int dim;

			XtVaGetValues(WI_new_height_value, XtNvalue, &dim, NULL);
			cl->resource_flags |= HEIGHT_RES_LOCKED;
			cl->height = (Dimension)dim;
			XtSetArg(args[i], XtNheight, (Dimension)dim); i++;
		}
	} else {
		XtSetArg(args[i], XtNheight, height); i++;
		cl->resource_flags |= HEIGHT_RES_LOCKED;
		cl->height = height;
	}

	LogResourceSetting(cl, XtNborderWidth, MEASUREMENT(border), FALSE);
	XtSetArg(args[i], XtNborderWidth, border); i++;

	new_args = XtMergeArgLists(preset_args, num_preset_args,
					args, i);
	CreateAndRealize(cl, parent, new_args, i + num_preset_args, TRUE);
	LogPresetArguments(cl, preset_args, num_preset_args);

	XtFree((char *)preset_args);
	XtFree((char *)new_args);

	AddEventHandlers(cl);
	CreateInstanceList(GetStartOfList(cl->in_app));
	AddDestroyCallback(cl);
}


static ObjectPtr
create_application(name, class, width, height)
String name, class;
Dimension width, height;
{
	char tmp[100];
	ObjectPtr cl;
	Cardinal num_preset_args;
	int border;
	ArgList preset_args;
	Boolean flag;

	cl = CreateObjectRecord();
	cl->class = FindClass(TOP_SHELL_CLASS);
	cl->managed = TRUE;
	cl->parent = &created_widgets;
	cl->name = XtNewString(name);
	AddHashedName(cl);
	cl->is_remote = FALSE;
	cl->in_app = TRUE;
	cl->deferred_creation = FALSE;

	/* retrieve the default values set by the user for the border,
	 * height and width.
	 */
	if (width == 0) {
		XtVaGetValues(WI_app_width_value, XtNvalue, &border, NULL);
		width = border;
	}
	if (height == 0) {
		XtVaGetValues(WI_app_height_value, XtNvalue, &border, NULL);
		height = border;
	}
	XtVaGetValues(WI_app_border_value, XtNvalue, &border, NULL);

	if (width == 0 || height == 0)
		return (NULL);

	preset_args = GetArgumentList(APPLICATION_RESOURCES, &num_preset_args);

	LogResourceSetting(cl, XtNborderWidth, MEASUREMENT(border), FALSE);

	cl->instance = XtAppCreateShell(name, class,
					TOP_SHELL_CLASS,
					XtDisplay(created_widgets.instance),
					preset_args, num_preset_args);

	LogPresetArguments(cl, preset_args, num_preset_args);

	XtVaGetValues(WI_app_x_locked_toggle, XtNstate, &flag, NULL);
	if (flag) cl->resource_flags |= X_RES_LOCKED;
	XtVaGetValues(WI_app_y_locked_toggle, XtNstate, &flag, NULL);
	if (flag) cl->resource_flags |= Y_RES_LOCKED;
	XtVaGetValues(WI_app_width_locked_toggle, XtNstate, &flag, NULL);
	if (flag) cl->resource_flags |= WIDTH_RES_LOCKED;
	XtVaGetValues(WI_app_height_locked_toggle, XtNstate, &flag, NULL);
	if (flag) cl->resource_flags |= HEIGHT_RES_LOCKED;

	XtVaSetValues(cl->instance, XtNwidth, width, XtNheight, height,
					XtNborderWidth, border, NULL);
	XtRealizeWidget(cl->instance);

	XtFree((char *)preset_args);

	AddCreatedObject(cl);
	AddAppEventHandlers(cl);
	CreateInstanceList(cl);
	AddDestroyCallback(cl);
	CreateResourceTable(APPLICATION_RESOURCES, cl, cl->class, FALSE);
	return (cl);
}


static void
prompt_and_create_widget(wc)
Cardinal wc;
{
	int x, y, width, height, border;
	Position nx = 0, ny = 0;
	Widget pw = NULL;

	/* let the user drag out a box to denote the size and position of
	 * the newly created widget */
	XukcDragBox(created_widgets.instance, &x, &y, &width, &height,
			(void (*)())NULL);

	/* determine which parent this widget is to be placed in */
	if (parent_obj == NULL) {
		if (created_app != NULL)
			pw = find_parent(created_app->instance, x, y,
							width, height);
		if (pw == NULL && created_template != NULL)
			pw = find_parent(created_template->instance,
							x, y, width, height);
		if (pw == NULL)
			return;
	} else
		pw = parent_obj->instance;

	/* calculate the dragged out size : these will be zero if not dragged*/
	width -= x;
	height -= y;

	/* if the parent was already chosen then set the x and y to be
	 * zero.
	 */
	if (parent_obj == NULL) {
		XtTranslateCoords(pw, (Position)0, (Position)0, &nx, &ny);
		x -= nx;
		y -= ny;
	} else
		x = y = 0;


	XtVaGetValues(WI_new_border_value, XtNvalue, &border, NULL);

	create_a_widget(new_name, wc, SearchRecords(&created_widgets, pw),
			(Position)x, (Position)y,
			(Dimension)width, (Dimension)height,
			(Dimension)border);
	if (feedback != NULL) {
		XtDestroyWidget(feedback);
		XFlush(XtDisplayOfObject(feedback));
		feedback = NULL;
	}
}


static void
set_new_parent(obj, parent_str)
ObjectPtr obj;
String parent_str;
{
	if (obj == NULL)
		obj = SearchRecordsForName(GetHeadObject(), parent_str);

	if (obj != NULL) {
		parent_obj = obj;
		(void)strcpy(new_parent, obj->name);
	} else if (*parent_str == '\0') {
		parent_obj = NULL;
		*new_parent = '\0';
	}

	if (parent_obj != NULL &&
	    !IsCompositeClass(GetClass(parent_obj->class))) {
		parent_obj = NULL;
		*new_parent = '\0';
		GiveWarning(WI_new_parent_entry, "invalidParent",
			"nonCompositeParent",
			"Object \"%s\" is not a valid parent (not a Composite)",
			1, obj->name);
	}

	XukcNewTextWidgetString(WI_new_parent_entry, new_parent);
}


/* ARGSUSED */
static void
picked_parent(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Widget picked = NULL;
	ObjectPtr selected;
	Window win;

	if (NoObjectsCreated())
		return;
	
	win = XukcSelectWindow(GetHeadObject()->instance);

	if (created_app != NULL)
		picked = ObjectWindowToWidget(created_app, win);
	if (picked == NULL && created_template != NULL)
		picked = ObjectWindowToWidget(created_template, win);
	if (picked != NULL) {
		selected = NearestWidgetObject(picked);
		if (selected != NULL)
			set_new_parent(selected, (String)NULL);
	}
}


/* called when an object dies */
/* ARGSUSED */
static void
update_list(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	CreateInstanceList(GetStartOfList((Boolean)client_data));
}


/**** Module Exported Routines ****/


/* ARGSUSED */
void
DeleteApplication(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (created_app == NULL)
		return;

	SetCurrentObject(created_app);
	DeleteObjectDialog(w, client_data, call_data);
}


/* ARGSUSED */
void
NewApplication(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	int x, y, width, height;

	if (*app_name == '\0' || *app_class == '\0') {
		XBell(XtDisplay(w), 50);
		return;
	}

	XukcDragBox(created_widgets.instance, &x, &y, &width, &height,
			(void (*)())NULL);
	width -= x;
	height -= y;

	created_app = create_application(app_name, app_class,
				(Dimension)width, (Dimension)height);

	/* change the mode settings */
	AfterApplicationShellCreated(created_app);

	XFlush(XtDisplay(created_widgets.instance));
}


/* ARGSUSED */
void
NewObject(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (created_widgets.children == NULL ||
	    *new_name == '\0' || GetClassChosen() == 0) {
		XBell(XtDisplay(w), 50);
		return;
	}

	if (parent_obj != NULL && !IsValidParent(parent_obj, GetClassChosen())){
		GiveWarning(w, "invalidParent", "createObject",
			"\"%s\" cannot be a Parent to a \"%s\" Object",
			2, parent_obj->name,
			GetDisplayedClassName(GetClassChosen()));
		return;
	}

	prompt_and_create_widget(GetClassChosen());
}


/* ARGSUSED */
void
ResetNewObject(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* reset the locking toggle buttons */
	XtVaSetValues(WI_new_x_pos_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_new_y_pos_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_new_width_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_new_height_label, XtNstate, FALSE, NULL);

	SETVALUE(WI_new_x_pos_value, 0);
	SETVALUE(WI_new_y_pos_value, 0);
	SETVALUE(WI_new_height_value, 300);
	SETVALUE(WI_new_width_value, 500);
	SETVALUE(WI_new_border_value, 1);

	XukcClearTextWidget(WI_new_name_entry);
	XukcClearTextWidget(WI_new_parent_entry);

	new_parent[0] = '\0';
	XukcChangeLabel(WI_new_class_entry, "", FALSE);

	ResetClassChosen();
}


void
InitializeCreation()
{
	Arg args[4];

	/* set default values for application and instance entries */
	SETVALUE(WI_app_x_pos_value, 0);
	SETVALUE(WI_app_y_pos_value, 0);
	SETVALUE(WI_app_height_value, 300);
	SETVALUE(WI_app_width_value, 500);
	SETVALUE(WI_app_border_value, 1);
	SETVALUE(WI_new_x_pos_value, 0);
	SETVALUE(WI_new_y_pos_value, 0);
	SETVALUE(WI_new_height_value, 300);
	SETVALUE(WI_new_width_value, 500);
	SETVALUE(WI_new_border_value, 1);

	/* reset the locking toggle buttons */
	XtVaSetValues(WI_new_x_pos_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_new_y_pos_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_new_width_label, XtNstate, FALSE, NULL);
	XtVaSetValues(WI_new_height_label, XtNstate, FALSE, NULL);

	XukcBindButton((UKCMed3MenuWidget)WI_pick_parent, FROM_APPLICATION,
				picked_parent, (XtPointer)NULL);

	/* create a dummy connection to the display we are developing the
	 * user interfaces on.  Its needed as the absolute root of the
	 * interface and template hierarchies for loading.
	 */
	XtSetArg(args[0], XtNx, -10);
	XtSetArg(args[1], XtNy, -10);
	XtSetArg(args[2], XtNwidth, 1);
	XtSetArg(args[3], XtNheight, 1);
	created_widgets.instance = InitializeApplicationShell(WI_dirt,
						  overrideShellWidgetClass,
						  "_dirtConnection", "_Dirt",
						  (ArgList)args, 4);
	XtRealizeWidget(created_widgets.instance);
	XtUnmapWidget(created_widgets.instance);

	created_widgets.class = 0;
	created_widgets.is_resource = created_widgets.is_remote = FALSE;
	created_widgets.name = NULL;
	created_widgets.managed = FALSE;
	created_widgets.in_app = FALSE;
	created_widgets.preset_name_list = 0;
	created_widgets.preset_value_list = 0;
	created_widgets.resource_name_list = 0;
	created_widgets.resource_value_list = 0;
	created_widgets.constraint_name_list = 0;
	created_widgets.constraint_value_list = 0;
	created_widgets.sub_resources_list = 0;
	created_widgets.mark_num = 0;
	created_widgets.children = NULL;
	created_widgets.parent = NULL;
	created_widgets.prev = NULL;
	created_widgets.next = NULL;
}


void
ChangeStartObjectRealization(is_realized)
Boolean is_realized;
{
	/* we unrealize all the widgets when loading in a new spec.
	 * then we realize them all again.
	 */
	if (is_realized)
		XtRealizeWidget(created_widgets.instance);
	else
		XtUnrealizeWidget(created_widgets.instance);
}


String
GetAppClassName()
{
	return (app_class);
}


void
UpdateApplicationNames(w)
Widget w;
{
	String tmp;

	tmp = XukcGetTextWidgetString(w);
	/* change of Application name means the head of the tree must be
	 * updated. */
	if (w == WI_create_name_entry) {
		ObjectPtr start_ptr = GetStartOfList(TRUE);
		(void)strcpy(app_name, tmp);
		if (start_ptr != NULL) {
			RemoveHashedName(start_ptr);
			XtFree((char *)start_ptr->name);
			start_ptr->name = XtNewString(app_name);
			CreateInstanceList(start_ptr);
			AddHashedName(start_ptr);
		}
	} else {
		if (w == WI_new_name_entry)
			(void)strcpy(new_name, tmp);
		else {
			if (w == WI_new_parent_entry)
				set_new_parent((ObjectPtr)NULL, tmp);
			else
				(void)strcpy(app_class, tmp);
		}
	}
	/* this frees the string returned in tmp */
	XawAsciiSourceFreeString(XawTextGetSource(w));
}


/* take an externally defined region and a widget class and update
 * the create popup dialog to reflect these new pre-creation choices.
 */
void
UpdatePrecreateChoices(wc, x1, y1, x2, y2)
WidgetClass wc;
Position x1, y1, x2, y2;
{
	Widget parent;
	ObjectPtr pobj;
	Position tmp_x, tmp_y;
	int p;

	if (x1 == -9999) {
		/* just select the widget class */
		CreateResourceTable(NEW_WIDGET_RESOURCES, (ObjectPtr)NULL,
					FindClass(wc), FALSE);
		XukcChangeLabel(WI_new_class_entry,
					CLASS_DISPLAYED(FindClass(wc)-1),
					FALSE);
		SetClassChosen(FindClass(wc));
		return;
	}

	if (NoObjectsCreated())
		return;

	parent = NULL;
	if (created_app != NULL)
		parent = find_parent(created_app->instance, x1, y1, x2, y2);
	if (parent == NULL && created_template != NULL)
		parent = find_parent(created_template->instance,
					x1, y1, x2, y2);
	if (parent == NULL ||
	    (pobj = SearchRecords(GetHeadObject(), parent)) == NULL)
		return; /* couldn't find a valid parent */

	set_new_parent(pobj, (String)NULL);

	CreateResourceTable(NEW_WIDGET_RESOURCES, (ObjectPtr)NULL,
				FindClass(wc), FALSE);
	XukcChangeLabel(WI_new_class_entry, CLASS_DISPLAYED(FindClass(wc)-1),
				FALSE);
	SetClassChosen(FindClass(wc));
	XtTranslateCoords(parent, 0, 0, &tmp_x, &tmp_y);
	p = x1 - tmp_x;
	SETVALUE(WI_new_x_pos_value, p);
	p = y1 - tmp_y;
	SETVALUE(WI_new_y_pos_value, p);
	p = x2-x1;
	SETVALUE(WI_new_width_value, p);
	p = y2-y1;
	SETVALUE(WI_new_height_value, p);
	if (feedback != NULL)
		XtDestroyWidget(feedback);
	feedback = XtVaCreateWidget("classChosen",
					ukcLabelWidgetClass, parent,
					XtNlabel, XukcClassName(wc),
					XtNx, (x1 - tmp_x),
					XtNy, (y1 - tmp_y),
					XtNwidth, (x2-x1),
					XtNheight, (y2-y1),
					NULL);
	if (XtIsComposite(parent))
		XtManageChild(feedback);
	AddNewDimensionHandlers(feedback);
	XFlush(XtDisplayOfObject(feedback));
}


void
RealizeAllChildren(obj)
ObjectPtr obj;
{
	if (obj == NULL)
		return;

	error_object = obj;
	error_handler = XtAppSetErrorMsgHandler(GetUIContext(),
			     (XtErrorMsgHandler)deal_with_zero_dimension_error);
	if (XtIsShell(obj->instance))
		XtPopup(obj->instance, XtGrabNone);
	else if (XtIsWidget(obj->instance))
		XtRealizeWidget(obj->instance);

	if (XtIsWidget(obj->instance) && obj->parent != NULL &&
	    !XtIsComposite(obj->parent->instance))
		XtMapWidget(obj->instance);

	error_object = NULL;
	(void)XtAppSetErrorMsgHandler(GetUIContext(),
					(XtErrorMsgHandler)error_handler);
	RealizeAllChildren(obj->next);
	RealizeAllChildren(obj->children);
}


/* If this object has been created with a zero width or height
 * we must change that dimension so that Xt does not produce an
 * error.  We give it its parents dimensions.
 */
void
CheckForZeroDimensions(obj)
ObjectPtr obj;
{
	Dimension width, height;

	XtVaGetValues(obj->instance, XtNwidth, &width, XtNheight, &height,
			NULL);
	if (width == 0)
		XtVaGetValues(obj->parent->instance, XtNwidth, &width, NULL);
	if (height == 0)
		XtVaGetValues(obj->parent->instance, XtNheight, &height, NULL);
	XtVaSetValues(obj->instance, XtNwidth, width, XtNheight, height, NULL);
}


void
CreateAndRealize(obj, parent, args, num_args, realize_now)
ObjectPtr obj, parent;
ArgList args;
Cardinal num_args;
Boolean realize_now;
{
	WidgetClass wc = GetClass(obj->class);
	Boolean is_shell = IsShellClass(wc);

	/* look for a constructor if one exists */
	if (CLASS_CONSTRUCTOR(obj->class-1) != NO_CONSTRUCTOR) {
		error_object = obj;
		error_handler = XtAppSetErrorMsgHandler(GetUIContext(),
					(XtErrorMsgHandler)
					   deal_with_zero_dimension_error);
		obj->instance = (*CLASS_CONSTRUCTOR(obj->class-1))(
				parent->instance, obj->name, args, num_args);
		error_object = NULL;
		(void)XtAppSetErrorMsgHandler(GetUIContext(),
					      (XtErrorMsgHandler)error_handler);
		if (is_shell) {
			if (realize_now) {
				CheckForZeroDimensions(obj);
				XtPopup(obj->instance, XtGrabNone);
			}
		} else if (obj->managed && XtIsRectObj(obj->instance)
		  && XtIsComposite(parent->instance))
			XtManageChild(obj->instance);
	} else {
		if (is_shell) {
			obj->instance = XtCreatePopupShell(obj->name, wc,
					parent->instance, args, num_args);
			if (realize_now) {
				CheckForZeroDimensions(obj);
				XtPopup(obj->instance, XtGrabNone);
			}
		} else {
			error_object = obj;
			error_handler = XtAppSetErrorMsgHandler(GetUIContext(),
						(XtErrorMsgHandler)
						deal_with_zero_dimension_error);

			obj->instance = XtCreateWidget(obj->name, wc,
					parent->instance, args, num_args);
			if (obj->managed && XtIsRectObj(obj->instance)
			    && XtIsComposite(parent->instance))
				XtManageChild(obj->instance);

			error_object = NULL;
			(void)XtAppSetErrorMsgHandler(GetUIContext(),
					     (XtErrorMsgHandler)error_handler);
		}
	}
	if (realize_now)
		DrainUIEventQueue();
}


/****** Creation list public access routines *******/


Boolean
ObjectInApplication(cl)
ObjectPtr cl;
{
	if (cl->parent == &created_widgets)
		return (FALSE);

	if (cl->parent != NULL)
		return (cl->parent->in_app);

	return (FALSE);
}


ObjectPtr
CreateObjectRecord()
{
	ObjectPtr clr;

	clr = XtNew(ObjectListRec);
	clr->class = 0;
	clr->is_resource = clr->is_remote = FALSE;
	clr->remote_done = FALSE;
	clr->instance = NULL;
	clr->name = NULL;
	clr->managed = FALSE;
	clr->deferred_creation = FALSE;
	clr->in_app = TRUE;
	clr->resource_flags = 0;
	clr->x = clr->y = 0;
	clr->width = clr->height = 0;
	clr->preset_name_list = 0;
	clr->preset_value_list = 0;
	clr->resource_name_list = 0;
	clr->resource_value_list = 0;
	clr->constraint_name_list = 0;
	clr->constraint_value_list = 0;
	clr->sub_resources_list = 0;
	clr->comments = XtNewString("");
	clr->help = XtNewString("");
	clr->mark_num = 0;
	clr->children = NULL;
	clr->parent = NULL;
	clr->prev = NULL;
	clr->next = NULL;

	return (clr);
}


void
AddCreatedObject(new_obj)
ObjectPtr new_obj;
{
	if (new_obj->parent != NULL) {
		ObjectPtr tmp = NULL;

		if (new_obj->parent->children == NULL)
			new_obj->parent->children = new_obj;
		else {
			tmp = new_obj->parent->children;
			for (; tmp->next != NULL;) 
				tmp = tmp->next;
			tmp->next = new_obj;
		}
		new_obj->prev = tmp;
	}
}


/* destroy an object instance record for the user interface */
void
DestroyObjectRecord(to_destroy)
ObjectPtr to_destroy;
{
	if (to_destroy == NULL || to_destroy->name == NULL)
		return;

	if (to_destroy->children != NULL)
		recursive_destroy(to_destroy->children);
		
	if (to_destroy->prev == NULL) { /* start of child list? */
		if (to_destroy->parent != NULL)
			to_destroy->parent->children = to_destroy->next;
	} else
		to_destroy->prev->next = to_destroy->next;
	if (to_destroy->next != NULL)
		to_destroy->next->prev = to_destroy->prev;

	RemoveHashedName(to_destroy);
	XtFree((char *)to_destroy->name);
	to_destroy->name = NULL;
	to_destroy->parent = NULL;
	XukcDestroyList(&(to_destroy->preset_name_list), TRUE);
	XukcDestroyList(&(to_destroy->preset_value_list), TRUE);
	XukcDestroyList(&(to_destroy->resource_name_list), TRUE);
	XukcDestroyList(&(to_destroy->resource_value_list), TRUE);
	XukcDestroyList(&(to_destroy->constraint_name_list), TRUE);
	XukcDestroyList(&(to_destroy->constraint_value_list), TRUE);
	XukcDestroyList(&(to_destroy->sub_resources_list), TRUE);
	XtFree((char *)to_destroy->comments);
	XtFree((char *)to_destroy->help);
	XtFree((char *)to_destroy);
}


static void
recursive_destroy(to_destroy)
ObjectPtr to_destroy;
{
	if (to_destroy == NULL)
		return;

	recursive_destroy(to_destroy->children);
	recursive_destroy(to_destroy->next);
	DestroyObjectRecord(to_destroy);
}


void
DeleteObject(to_destroy)
ObjectPtr to_destroy;
{
	if (to_destroy == NULL)
		return;

	if (to_destroy == CurrentlyEditing())
		ChangeSelection((ObjectPtr)NULL);

	/* the destroy callbacks remove all the structures and cleans
	 * everything up for you! */
	if (to_destroy->is_remote) {
		Boolean in_app = to_destroy->in_app;

		recursive_destroy(to_destroy->children);
		DestroyObjectRecord(to_destroy);
		CreateInstanceList(GetStartOfList(in_app));
	} else {
		XtAddCallback(to_destroy->instance, XtNdestroyCallback,
				update_list, (XtPointer)to_destroy->in_app);
		XtDestroyWidget(to_destroy->instance);
	}
}


/* ARGSUSED */
void
DeleteButton(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	ObjectPtr to_destroy = CurrentlyEditing();

	loop_condition = FALSE;
	XtPopdown(WI_delete_dialog_shell);

	if ((client_data != NULL && *(String)client_data != '\0')
	    || to_destroy == NULL)
		return;

	if (to_destroy == created_app || to_destroy == created_template)
		SetStartOfList((ObjectPtr)NULL, to_destroy->in_app);
	else
		DeleteObject(CurrentlyEditing());
}


/* ARGSUSED */
void
DeleteObjectDialog(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Position x, y;
	Dimension width, height, nw, nh;
	char object_label[100];
	ObjectPtr obj;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "deleteObject",
			"No Objects To Delete", 0);
		return;
	}

	obj = CurrentlyEditing();

	if (obj == NULL) {
		GiveWarning(w, "nothingChosen", "deleteObject",
				"Delete which object? : please Pick one",
				0);
		return;
	}

	XtVaGetValues(WI_dirt, XtNwidth, &width, XtNheight, &height, NULL);
	XtVaGetValues(WI_dirt, XtNwidth, &nw, XtNheight, &nh, NULL);

	XtTranslateCoords(WI_dirt, 0, 0, &x, &y);
	x += (width - nw)/2;
	y += (height - nh)/2;

	XtVaSetValues(WI_delete_dialog_shell, XtNx, x, XtNy, y, NULL);

	(void)sprintf(object_label, "Object = %s", obj->name);
	XtVaSetValues(WI_delete_dialog_object, XtNlabel, object_label, NULL);
	
	/* put up a dialog box to confirm the deletion */
	XtPopup(WI_delete_dialog_shell, XtGrabExclusive);

	loop_condition = TRUE;
	XukcConditionalLoop(XukcDirtEditor, &loop_condition);
	/* Yes or No button has been pressed */
}


void
AddDestroyCallback(obj)
ObjectPtr obj;
{
	XtAddCallback(obj->instance, XtNdestroyCallback, unlink_object,
								(XtPointer)obj);
}


ObjectPtr
GetStartOfList(is_application)
Boolean is_application;
{
	return (is_application ? created_app : created_template);
}


ObjectPtr
GetHeadObject()
{
	return (&created_widgets);
}


Boolean
NoObjectsCreated()
{
	return (created_app == NULL && (created_template == NULL ||
		created_template->children == NULL));
}
	

void
SetStartOfList(cl, is_application)
ObjectPtr cl;
Boolean is_application;
{
	ChangeSelection((ObjectPtr)NULL);

	if (is_application) {
		ObjectPtr to_destroy = created_app;

		created_app = NULL;
		DeleteObject(to_destroy);
		created_app = cl;
	} else {
		if (created_template != NULL) {
			/* delete all the template's children.
			 * we cannot delete the template record by destroying
			 * the widget instance because its instance is
			 * part of Dirt itself.
			 */
			ObjectPtr children = created_template->children;

			while (children != NULL) {
				if (children->next == NULL) {
					XtAddCallback(children->instance,
						XtNdestroyCallback,
						unlink_object,
						(XtPointer)created_template);
					XtAddCallback(children->instance,
						XtNdestroyCallback,
						update_list, (XtPointer)FALSE);
				}
				XtDestroyWidget(children->instance);
				if (children->next == NULL)
					XFlush(XtDisplayOfObject(
						children->instance));
				children = children->next;
			}
			/* now inform the Grid drawing routine so that it
			 * can clean up if necessary. */
			GridObjectHasDied(created_template);
		}
		created_template = cl;
	}

	if (cl != NULL) {
		CreateInstanceList(cl);
		cl->parent = &created_widgets;
		AddCreatedObject(cl);
	} else {
		ClearInstanceList(is_application);
		if (is_application) {
			CreateResourceTable(APPLICATION_RESOURCES,
				(ObjectPtr)NULL,
				FindClass(applicationShellWidgetClass),
				FALSE);
			XukcClearTextWidget(WI_create_name_entry);
			XukcClearTextWidget(WI_create_class_entry);
		}
	}
}


ObjectPtr
SearchRecords(entry, w)
ObjectPtr entry;
Widget w;
{
	ObjectPtr tmp;

	if (entry == NULL)
		return (NULL);

	if (entry->instance == w)
		return (entry);
	else {
		tmp = NULL;
		if (entry->children != NULL)
			tmp = SearchRecords(entry->children, w);
		if (tmp == NULL)
			return (SearchRecords(entry->next, w));
	}
	return (tmp);
}


ObjectPtr
SearchRecordsForMarkNum(entry, num)
ObjectPtr entry;
Cardinal num;
{
	ObjectPtr tmp;

	if (entry == NULL)
		return (NULL);

	if (entry->mark_num == num)
		return (entry);
	else {
		tmp = NULL;
		if (entry->children != NULL)
			tmp = SearchRecordsForMarkNum(entry->children, num);
		if (tmp == NULL)
			return (SearchRecordsForMarkNum(entry->next, num));
	}
	return (tmp);
}


ObjectPtr
SearchRecordsForName(entry, name)
ObjectPtr entry;
String name;
{
	ObjectPtr tmp;

	if (entry == NULL)
		return (NULL);

	if (entry->name != NULL && strcmp(entry->name, name) == 0)
		return (entry);
	else {
		tmp = NULL;
		if (entry->children != NULL)
			tmp = SearchRecordsForName(entry->children, name);
		if (tmp == NULL)
			return (SearchRecordsForName(entry->next, name));
	}
	return (tmp);
}


/* A complex name is "objectName(level).objectName(level){etc}"
 * A compounds name is "objectName.objectName{etc}"
 * A simple name is "objectName"
 */
ObjectPtr
SearchRecordsForComplexName(entry, name)
ObjectPtr entry;
String name;
{
	String descendent;
	Cardinal level;
	ObjectPtr tmp;

	if (entry == NULL)
		return (NULL);

	descendent = index(name, '(');
	if (descendent == NULL) {
		if ((descendent = index(name, '.')) != NULL) {
			/* search for compound name */
			char *str = XtMalloc(strlen(name) + 2);
			Widget w;

			(void)sprintf(str, "*%s", name);
			w = XtNameToWidget(entry->children->instance,
							descendent);
			XtFree(str);
			return (SearchRecords(entry, w));
		} else
		    /* assume simple naming style and use different search */
			return (SearchRecordsForName(entry, name));
	}

	if ((level = atoi(descendent + 1)) == 0) {
		GiveWarning(WI_dirt, "badObjectName",
			"searchRecordsForComplexName",
			"Dirt: internal warning, illegal name (bad level #) passed to SearchRecordsForComplexName(): %s",
			1, name);
		return (NULL);
	}

	entry = entry->children;
	if (entry == NULL)
		return (NULL);

	while (entry != NULL && level > 1) {
		entry = entry->next;
		level--;
	}

	*descendent = '\0';

	if (entry == NULL) {
		GiveWarning(WI_dirt, "invalidObjectName",
			"searchRecordsForComplexName",
			"Dirt: internal warning, invalid name (incorrect level #) passed to SearchRecordsForComplexName(): %s",
			1, name);
		return (NULL);
	}

	if (strcmp(entry->name, name) != 0)
		return (NULL);

	name = descendent + 1;
	while (*name && *name != '.')
		name++;

	if (*name == '\0')
		return (entry);

	tmp = SearchRecordsForComplexName(entry, name + 1);
	return (tmp);
}


Boolean
IsValidParent(parent, class)
ObjectPtr parent;
Cardinal class;
{
	if (parent->is_remote)
		return (FALSE);

	if (!IsCompositeClass(GetClass(parent->class)))
		return (FALSE);

	if (IsSubclass(GetClass(class), coreWidgetClass))
		return (TRUE); /* is a widget */

	/* must be an object so check that parent can accept objects */
	return (AcceptsObjects(GetClass(parent->class),
			       (ObjectClass)GetClass(class)));
}
