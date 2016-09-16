#ifndef lint
static char sccsid[] = "@(#)copy.c	1.3 (UKC) 5/17/92";
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
 * copy.c : contains the functions for copying instances of widgets
 */

#include <ctype.h> /* isdigit() */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

/* externally defined routines */
extern Boolean IsCompositeClass();
extern Boolean NoObjectsCreated();
extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern ObjectPtr CreateObjectRecord();
extern ObjectPtr CurrentlyEditing();
extern void ChangeSelection();
extern Widget ObjectWindowToWidget();
extern ObjectPtr NearestWidgetObject();
extern Window XukcSelectPoint();
extern WidgetClass GetClass();
extern XtResourceList GetAResource();
extern void LogResourceSetting();
extern Boolean IsStringType();
extern void CreateAndRealize();
extern void CheckForZeroDimensions();
extern Boolean IsValidParent();
extern void DrainUIEventQueue();
extern Boolean IsNotUniqueName();
extern void RemoveHashedName();
extern void AddHashedName();
extern void AddEventHandlers();
extern void AddDestroyCallback();
extern void AddCreatedObject();
extern void XukcCopyToArgVal();
extern void CreateInstanceList();
extern void DeleteButton();

void GetUniqueName();


static ArgList
args_from_list(name_list, value_list, num_args)
Cardinal name_list;
Cardinal value_list;
Cardinal *num_args;
{
	String *names;
	String *values;
	ArgList args;
	Cardinal i;

	*num_args = 0;

	if (name_list == 0)
		return ((ArgList)NULL);

	names = (String *)XukcGetListPointer(name_list, num_args);
	if (*num_args == 0)
		return ((ArgList)NULL);

	values = (String *)XukcGetListPointer(value_list, num_args);
	args = (ArgList)XtMalloc(sizeof(Arg) * *num_args);

	for (i = 0; i < *num_args; i++)
		XtSetArg(args[i], names[i], values[i]);

	return (args);
}


static void
fill_in_values(obj, args, num_args)
ObjectPtr obj;
ArgList args;
Cardinal num_args;
{
	Cardinal i;
	int idx;
	XtResourceList resl;
	Boolean junk;
	XrmValue from, to;

	if (num_args == 0)
		return;

	for (i = 0; i < num_args; i++) {
		resl = GetAResource(obj, args[i].name, FALSE, &idx, &junk);

		if (idx < 0)
			XtError("Dirt: internal error in fill_in_values()");

		if (IsStringType(resl[idx].resource_type)) {
			XtFree((char *)resl);
			continue;
		}
		from.addr = (XtPointer)args[i].value;
		from.size = strlen((char *)args[i].value) + 1;
		to.addr = NULL;
		to.size = 0;
		if (DoObjectConversion(obj, XtRString, &from,
		    resl[idx].resource_type, &to)) {
			XukcCopyToArgVal(to.addr, &(args[i].value),
						resl[idx].resource_size);
		} else
			XtError("Dirt: conversion error in fill_in_values()");
		XtFree((char *)resl);
	}
}


static void
copy_list(src_list, dest_list)
Cardinal src_list, *dest_list;
{
	String *list;
	Cardinal i, num;

	if (src_list == 0) {
		*dest_list = 0;
		return;
	}
	list = (String *)XukcGetListPointer(src_list, &num);

	for (i = 0; i < num; i++)
		(void)XukcAddObjectToList(dest_list, list[i] == NULL ?
						NULL : XtNewString(list[i]),
						FALSE);
}


static ObjectPtr
create_copy_of_object(src, dest_parent, x, y, use_position, moved, new_children)
ObjectPtr src, dest_parent;
Position x, y;
Boolean use_position, moved;
Cardinal *new_children;
{
	ObjectPtr new;
	ArgList args = NULL;
	Cardinal num_args = 0;

	if (src == NULL || dest_parent == NULL || src->is_resource)
		return NULL;

	new = CreateObjectRecord();

	new->class = src->class;
	new->is_resource = FALSE;
	new->is_remote = dest_parent->is_remote;
	new->parent = dest_parent;
	new->in_app = dest_parent->in_app;
	new->managed = FALSE;
	new->name = XtNewString(src->name);
	AddHashedName(new);
	if (!moved || dest_parent->in_app != src->in_app)
		GetUniqueName(new, new->name);

	copy_list(src->preset_name_list, &(new->preset_name_list));
	copy_list(src->preset_value_list, &(new->preset_value_list));
	copy_list(src->resource_name_list, &(new->resource_name_list));
	copy_list(src->resource_value_list, &(new->resource_value_list));
	new->constraint_name_list = 0;
	new->constraint_value_list = 0;
	new->sub_resources_list = 0;
	new->resource_flags = src->resource_flags;
	new->width = src->width;
	new->height = src->height;
	new->x = src->x;
	new->y = src->y;
	new->comments = XtNewString(src->comments);
	new->help = XtNewString(src->help);
	new->mark_num = 0;
	new->prev = NULL;
	new->next = NULL;

	args = args_from_list(new->preset_name_list, new->preset_value_list,
								&num_args);
	fill_in_values(src, args, num_args);

	CreateAndRealize(new, dest_parent, args, num_args, TRUE);

	if ((new->resource_flags & WIDTH_RES_LOCKED) == WIDTH_RES_LOCKED)
		XtVaSetValues(new->instance, XtNwidth, new->width, NULL);
	if ((new->resource_flags & HEIGHT_RES_LOCKED) == HEIGHT_RES_LOCKED)
		XtVaSetValues(new->instance, XtNheight, new->height, NULL);

	AddEventHandlers(new);

	if (args != NULL)
		XtFree((char *)args);
	args = args_from_list(new->resource_name_list, new->resource_value_list,
								&num_args);
	fill_in_values(new, args, num_args);
	if (num_args > 0) {
		XtSetValues(new->instance, args, num_args);
		XtFree((char *)args);
	}
	if (src->parent != NULL && src->parent->class == dest_parent->class) {
		/* copy over any constraint resources as we are doing a
		 * recursive copy and so they are valid here.
		 */
		copy_list(src->constraint_name_list,
						&(new->constraint_name_list));
		copy_list(src->constraint_value_list,
						&(new->constraint_value_list));
		args = args_from_list(new->constraint_name_list,
					new->constraint_value_list,
					&num_args);
		fill_in_values(new, args, num_args);
		if (num_args > 0) {
			XtSetValues(new->instance, args, num_args);
			XtFree((char *)args);
		}
	}

	if (use_position || !dest_parent->in_app)
		/* if the geometry change succeeds the new resource values
		 * will be automatically logged.
		 */
		XtVaSetValues(new->instance, XtNx, x, XtNy, y, NULL);

	CheckForZeroDimensions(new);

	(void)XukcAddObjectToList(new_children, (XtPointer)new, FALSE);
	new->managed = src->managed;
	if (new->managed && XtIsRectObj(new->instance) &&
	    IsCompositeClass(GetClass(new->parent->class)))
		XtManageChild(new->instance);

	AddDestroyCallback(new);
	return (new);
}


static ObjectPtr
pick_a_destination(x, y)
Position *x, *y;
{
	Widget picked = NULL;
	ObjectPtr selected;
	Window win;

	if (NoObjectsCreated())
		return;

	win = XukcSelectPoint(GetHeadObject()->instance, x, y);

	selected = GetStartOfList(TRUE);
	if (selected != NULL)
		picked = ObjectWindowToWidget(selected, win);
	if (picked == NULL && (selected = GetStartOfList(FALSE)) != NULL)
		picked = ObjectWindowToWidget(selected, win);

	if (picked != NULL)
		return (NearestWidgetObject(picked));
	return (ObjectPtr)NULL;
}


static void
recursive_copy_of_object(obj, dest, moved, new_children)
ObjectPtr obj, dest;
Boolean moved;
Cardinal *new_children;
{
	ObjectPtr new;

	if (obj == NULL)
		return;

	new = create_copy_of_object(obj, dest, 0, 0, FALSE,
					moved, new_children);
	recursive_copy_of_object(obj->children, new, moved, new_children);
	recursive_copy_of_object(obj->next, dest, moved, new_children);
}


static void
add_new_children(list)
Cardinal list;
{
	Cardinal num_children, i;
	ObjectPtr *children;

	if (list == NULL)
		return;

	children = (ObjectPtr *)XukcGetListPointer(list, &num_children);
	if (num_children == 0)
		return;

	for (i = 0; i < num_children; i++)
		AddCreatedObject(children[i]);

	XukcDestroyList(&list, FALSE);
}

/******************** Module Exported Routines *******************/

/* ARGSUSED */
void
CopyObject(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Position x, y;
	ObjectPtr obj, dest;
	Cardinal new_list;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "copyObject",
				"No Objects To Copy", 0);
		return;
	}
		
	obj = CurrentlyEditing();

	if (obj == NULL) {
		GiveWarning(w, "nothingChosen", "copyObject",
				"Copy which object? : please Pick one",
				0);
		return;
	}

	if ((dest = pick_a_destination(&x, &y)) == NULL)
		return;

	if (!IsValidParent(dest, obj->class)) {
		GiveWarning(w, "invalidParent", "copyObject",
			"Parent \"%s\" cannot accept a copy of \"%s\" object",
			2, dest->name, obj->name);
		return;
	}

	new_list = 0;

	if ((Boolean)client_data) {
		/* do a recursive copy */
		ObjectPtr new;

		new = create_copy_of_object(obj, dest, x, y,
					dest == obj->parent, FALSE, &new_list);
		recursive_copy_of_object(obj->children, new, FALSE, &new_list);
	} else
		(void)create_copy_of_object(obj, dest, x, y,
					dest == obj->parent, FALSE, &new_list);
	add_new_children(new_list);
	CreateInstanceList(GetStartOfList(dest->in_app));
	DrainUIEventQueue();
}


/* ARGSUSED */
void
MoveObject(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Position x, y;
	ObjectPtr obj, dest, new;
	Cardinal new_list;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "moveObject",
				"No Objects To Move", 0);
		return;
	}
		
	obj = CurrentlyEditing();

	if (obj == NULL) {
		GiveWarning(w, "nothingChosen", "moveObject",
				"Move which object? : please Pick one",
				0);
		return;
	}

	if (obj->is_remote) {
		GiveWarning(w, "illegalObject", "moveObject",
				"Cannot Move objects in external applications",
				0);
		return;
	}

	if ((dest = pick_a_destination(&x, &y)) == NULL)
		return;

	if (obj == dest) {
		GiveWarning(w, "sameObjects", "moveObject",
				"Cannot move an object on to itself",
				0);
		return;
	}

	if (obj->parent == dest) {
		/* no change in parents so just try to move the child */
		XtVaSetValues(obj->instance, XtNx, x, XtNy, y, NULL);
		return;
	}

	if (!IsValidParent(dest, obj->class)) {
		GiveWarning(w, "invalidParent", "moveObject",
				"Parent \"%s\" cannot accept \"%s\" object",
				2, dest->name, obj->name);
		return;
	}

	new_list = 0;
	new = create_copy_of_object(obj, dest, x, y, TRUE, TRUE, &new_list);
	recursive_copy_of_object(obj->children, new, TRUE, &new_list);
	add_new_children(new_list);
	DeleteButton(w, (XtPointer)"", call_data);
	if (obj->in_app != dest->in_app) /* optimize list redraw */
		CreateInstanceList(GetStartOfList(dest->in_app));
	DrainUIEventQueue();
}


/* take the new name and create a new unique name to replace */
void
GetUniqueName(obj, new_name)
ObjectPtr obj;
String new_name;
{
	String num, str, old;
	Cardinal n;
	char tmp[100];

	str = XtNewString(new_name);
	old = obj->name;
	RemoveHashedName(obj);
	obj->name = NULL;

	while (IsNotUniqueName(GetStartOfList(obj->in_app), str)) {
		num = str + strlen(str) - 1;
		while (isdigit(*num))
			num--;
		num++;
		n = atoi(num);
		*num = '\0';

		(void)sprintf(tmp, "%s%d", str, n + 1); 
		XtFree(str);
		str = XtNewString(tmp);
	}
	XtFree(old);
	obj->name = str;
	AddHashedName(obj);
}
