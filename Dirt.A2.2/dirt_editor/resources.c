#ifndef lint
static char sccsid[] = "@(#)resources.c	1.2 (UKC) 5/10/92";
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
#include <X11/Xukc/Command.h>
#include <X11/Xukc/RowCol.h>
#include <X11/Xukc/text.h>
#include "dirt_structures.h"
#include "resources.h"


/* externally defined routines */
extern void XukcChangeLabel();
extern XtPointer XukcGetObjectOnList();
extern void XukcCopyToArgVal();
extern Cardinal XukcAddObjectToList();
extern void XukcRemoveEntryOnList();
/* in list.c */
extern WidgetClass GetClass();
extern Boolean IsShellClass();
extern Boolean IgnoreClassResource();
/* in convert.c */
extern Boolean ConversionExists();
extern Widget EditableResource();
extern Boolean IsStringType();
/* in create.c */
extern ObjectPtr CreateObjectRecord();
extern ObjectPtr SearchRecords();
extern ObjectPtr GetHeadObject();
/* in help.c */
extern void GiveResourceHelp();
/* in dirt.c */
extern void DisallowEditing();
extern void AllowEditing();
/* in stubs.c */
extern void RemoveDefaultCallbacks();
/* in wc_extra.c */
extern void RemoveWcCallbacks();
/* in log.c */
extern void LogResourceSetting();
extern void RemoveResourceSetting();

extern Widget WI_trans_shell;
/* ||| if no instance exists use this for any extra conversion args ||| */
extern Widget WI_main_layout;

extern struct app_resources editor_resources;

void DeleteResourceTable();

static struct resource_list_details *resource_lists = NULL;
static Cardinal num_resource_lists = 0, free_resource_lists = 0;
static ObjectPtr dummy_object = NULL;

/******** internal error handlers that ignore errors/warnings *******/

static Boolean error_happened;

static void
ignore_msg_warnings(name, type, class, default_m, params, num_params)
String name, type, class, default_m, *params;
Cardinal *num_params;
{
        error_happened = TRUE;
}


static void
ignore_warnings(warning)
String warning;
{
        error_happened = TRUE;
}


static int
ignore_errors(dpy, errorp)
Display *dpy;
XErrorEvent *errorp;
{
        error_happened = TRUE;
        return 0;
}

/*********************************************************************/

static int
by_name(a, b)
char *a, *b;
{
	int r1;
	struct resource_entry *e1, *e2;

	e1 = (struct resource_entry *)a;
	e2 = (struct resource_entry *)b;

	if (editor_resources.sort_by_type) {
		r1 = strcmp(NAME(e1, resource_type), NAME(e2, resource_type));

		if (r1 == 0)
			return (strcmp(NAME(e1, resource_name),
				       NAME(e2, resource_name)));
	} else {
		r1 = strcmp(NAME(e1, resource_name), NAME(e2, resource_name));

		if (r1 == 0)
			return (strcmp(NAME(e1, resource_type),
				       NAME(e2, resource_type)));
	}
	return (r1);
}


static void
initialize_widget_class(class)
Cardinal class;
{
	XtInitializeWidgetClass(GetClass(class));
}


static void
release_storage(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtFree((char *)client_data);
}


/* The code in here is as hairy as a rabid badger .. be careful in there 8-) */
static XrmValuePtr
get_default_value(w, res_entry)
Widget w;
XtResourceList res_entry;
{
	static String ret_value = NULL; /* copy from this for XtRImmediate */
	static XrmValue value;  /* returns a pointer to this, so copy it! */
	XrmValue from;
	static int (*old_error_handler)();
	static void (*old_warning)();
	static void (*old_msg_warning)();

	value.size = 0;
	value.addr = NULL;
	from.size = sizeof(XtPointer);
	from.addr = (XtPointer)res_entry->default_addr;
	if (ret_value != NULL) {
		XtFree((char *)ret_value);
		ret_value = NULL;
	}

	if (strcmp(XtRImmediate, res_entry->default_type) == 0) {
		ret_value = XtMalloc(res_entry->resource_size + 1);
		bcopy((char *)&res_entry->default_addr, ret_value,
						(int)res_entry->resource_size);
		value.addr = (XtPointer)ret_value;
		value.size = res_entry->resource_size;
	} else if (strcmp(res_entry->default_type, XtRCallProc) == 0) {
		/* call the procedure conversion routine ...?? */
		(*(XtResourceDefaultProc)res_entry->default_addr)
				(w, res_entry->resource_offset, &value);
		if (value.size == 0)
			value.size = res_entry->resource_size;
	} else { /* do the standard conversion */
		if (IsStringType(res_entry->default_type)) {
			if (from.addr == NULL)
				from.addr = "";
			from.size = STRLEN(from.addr);
		}

		error_happened = FALSE;
		old_error_handler = XSetErrorHandler(ignore_errors);
		old_msg_warning = XtAppSetWarningMsgHandler(
                                        XtWidgetToApplicationContext(w),
                                        ignore_msg_warnings);
		old_warning = XtAppSetWarningHandler(
					XtWidgetToApplicationContext(w),
                                        ignore_warnings);

		if (!DoConversion(w, res_entry->default_type, &from,
					res_entry->resource_type, &value))
			value.addr = NULL;

		(void)XSetErrorHandler(old_error_handler);
		(void)XtAppSetWarningMsgHandler(XtWidgetToApplicationContext(w),
                                                old_msg_warning);
		(void)XtAppSetWarningHandler(XtWidgetToApplicationContext(w),
                                                old_warning);

		if (IsStringType(res_entry->resource_type)) {
			if (value.addr != NULL) {
				String *tmp;

				value.size = res_entry->resource_size;
				value.addr = XtNewString(value.addr);
				tmp = XtNew(String);
				*tmp = (String)value.addr;
				value.addr = (XtPointer)tmp;
			} else
				value.size = 0;
		} else {
			if (value.addr != NULL) {
				value.size = res_entry->resource_size;
				ret_value = XtMalloc(value.size);
				bcopy((char *)value.addr, ret_value, value.size);
				value.addr = ret_value;
			} else
				value.size = 0;
		}
	}
	/* value.addr should be copied before get_default_value() is called
	 * again .. otherwise it will be trashed. */
	return (&value);
}


/****** Module Exported Routines ******/


/* place the newly freed resource list entry back on to the
 * free_resource_lists list. */
void
ReleaseResourceList(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	int res_table = *(int *)client_data;

	if (res_table >= 0) {
		DeleteResourceTable((Cardinal)res_table);
		(void)XukcAddObjectToList(&free_resource_lists,
						(XtPointer)res_table, FALSE);
		*(int *)client_data = -1;
	}
}


/* Create a blank entry in the list of resource lists .. ready for use
 * by a sub-object being edited. */
Cardinal
NewResourceList(viewport)
Widget viewport;
{
	Cardinal i = 0;

	if (free_resource_lists == 0)
		return (0);  /* no space left .. so refuse the request */

	/* update the list of free resource list entries */
	i = (Cardinal)XukcGetObjectOnList(free_resource_lists, 1);
	XukcRemoveEntryOnList(&free_resource_lists, 1);

	resource_lists[i].resources_viewport = viewport;

	return (i);
}


/* get the list of resources for the widget who class is or will be
 * 'cl->class'.  Search the list for 'name',
 * as either an 'XtN' resource name (use_class = FALSE) or an 'XtC' class
 * name (use_class = TRUE).
 * RETURNS: the whole resource list and modifies resource_number to contain
 *          the subscript number of the found resource, this is -1 if the
 *          name wasn't found.  This list should be freed after use.
 */
XtResourceList
GetAResource(cl, name, use_class, resource_number, is_constraint)
ObjectPtr cl;
String name;
Boolean use_class;
int *resource_number;
Boolean *is_constraint;
{
	XtResourceList list, clist = NULL;
	Cardinal num_res, num_cons = 0;

	if (cl->instance == NULL)
		initialize_widget_class(cl->class);
	XtGetResourceList(GetClass(cl->class), &list, &num_res);
	if (!NO_PARENT(cl)) {
		/* get any constraint resources required by the parent */
		if (cl->is_resource && !cl->is_remote
		    && cl->instance != NULL) {
			ObjectPtr rp;
			/* check the real widget instances parent */
			rp = SearchRecords(GetHeadObject(),
						XtParent(cl->instance));
			if (rp->class != NULL)
				XtGetConstraintResourceList(GetClass(rp->class),
							&clist, &num_cons);
		} else if (cl->parent->class != NULL)
			XtGetConstraintResourceList(GetClass(cl->parent->class),
							&clist, &num_cons);
	}

	for (; num_res > 0; num_res--)
		if (strcmp(use_class ? list[num_res-1].resource_class 
				: list[num_res-1].resource_name, name) == 0)
			break;

	if (num_res == 0) {
		XtFree((char *)list);
		*is_constraint = TRUE;
		for (; num_cons > 0; num_cons--)
			if (strcmp(use_class ? clist[num_cons-1].resource_class
				: clist[num_cons-1].resource_name, name) == 0)
				break;
		if (num_cons == 0)
			*resource_number = -1;
		else
			*resource_number = num_cons-1;
		return (clist);
	} else {
		XtFree((char *)clist);
		*is_constraint = FALSE;
		*resource_number = num_res-1;
	}
	return (list);
}


void
DeleteResourceTable(num)
Cardinal num;
{
	/* dis-allow any editing of translations whilst updating the
	 * resource lists */
	XtPopdown(WI_trans_shell);

	if (resource_lists[num].resources_table == NULL)
		return;

	resource_lists[num].object = NULL;
	XtDestroyWidget(resource_lists[num].resources_table);
	resource_lists[num].resources_table = NULL;
	XtFree((char *)resource_lists[num].res);
	resource_lists[num].res = NULL;
	XtFree((char *)resource_lists[num].cres);
	resource_lists[num].cres = NULL;
	XtFree((char *)resource_lists[num].list);
	resource_lists[num].list = NULL;
	resource_lists[num].list_length = 0;
}


Cardinal
FindResourceList(entry)
struct resource_entry *entry;
{
	Widget res_table;
	Boolean not_found = TRUE;
	Cardinal list_num = 0;

	res_table = XtParent(entry->resource_name);
	for (; not_found && list_num < MAX_LISTS; list_num++)
		if (resource_lists[list_num].resources_table == res_table)
			not_found = FALSE;
	if (not_found)
		XtError("DIRT! internal error: cannot find resources_table in FindResourceList()");
	return (list_num-1);
}


void
CreateResourceTable(resource_list_number, object, class, force_read_only)
Cardinal resource_list_number;
ObjectPtr object;
Cardinal class;
Boolean force_read_only;
{
	int num_valid;
	Cardinal i, j, max_type, max_name;
	Cardinal on_res, on_cres;
	Cardinal ignore_mask = NULL;
	char format[100];
	ArgList vals;
	char tmp[1000];

	DeleteResourceTable(resource_list_number);
	if (object != NULL) {
		RLF(object) = object;
		class = object->class;
		ignore_mask = IGNORE;
	} else {
		initialize_widget_class(class);
		ignore_mask = IGNORE | EDIT_ONLY | READ_ONLY;
	}

	on_res = on_cres = 0;
	/* get a list of all the resources for this object */
	XtGetResourceList(GetClass(class), &RLF(res), &on_res);
	if (RLF(object) != NULL && !NO_PARENT(object)) {
		/* get any constraint resources required by the parent */
		if (object->is_resource && !object->is_remote
		    && object->instance != NULL) {
			ObjectPtr rp;
			/* check the real widget instances parent */
			rp = SearchRecords(GetHeadObject(),
						XtParent(object->instance));
			XtGetConstraintResourceList(GetClass(rp->class),
							&RLF(cres), &on_cres);
		} else
			XtGetConstraintResourceList(
					GetClass(object->parent->class),
					&RLF(cres), &on_cres);
	}

	/* find the number of resources we can actually change */
	for (num_valid = 0, i = 0; i < on_res; i++)
		if (ConversionExists(RLF(res[i]).resource_type) &&
		    !IgnoreClassResource(class, RLF(res[i]).resource_name,
		    RLF(res[i]).resource_class, RLF(res[i]).resource_type,
		    ignore_mask))
			num_valid++;
	for (i = 0; i < on_cres; i++)
		if (ConversionExists(RLF(cres[i]).resource_type) &&
		    !IgnoreClassResource(object->parent->class,
		    RLF(cres[i]).resource_name, RLF(cres[i]).resource_class,
		    RLF(cres[i]).resource_type, ignore_mask))
			num_valid++;

	/* no available resources so just destroy the old resource table */
	if (num_valid == 0)
		return;
	else
		/* new resource list of editable resources */
		RLF(list) = (struct resource_entry *)
			XtMalloc(sizeof(struct resource_entry) * num_valid);
	RLF(list_length) = num_valid;
	max_type = max_name = 0;

	/* assign the entry numbers to the resource list */
	for (j = i = 0; i < on_res; i++) {
		if (ConversionExists(RLF(res[i]).resource_type) &&
		    !IgnoreClassResource(class, RLF(res[i]).resource_name,
		    RLF(res[i]).resource_class, RLF(res[i]).resource_type,
		    ignore_mask)) {
			int len;

			RLF(list[j]).type = RLF(res + i);
			RLF(list[j]).value_set = FALSE;
			RLF(list[j]).is_constraint = FALSE;

			len = strlen(NAME((&RLF(list[j])), resource_type));
			if (len > max_type)
				max_type = len;
			len = strlen(NAME((&RLF(list[j])), resource_name));
			if (len > max_name)
				max_name = len;
			j++;
		}
	}
	for (i = 0; j < num_valid && i < on_cres; i++) {
		if (ConversionExists(RLF(cres[i]).resource_type) &&
		    !IgnoreClassResource(object->parent->class,
		    RLF(cres[i]).resource_name, RLF(cres[i]).resource_class,
		    RLF(cres[i]).resource_type, ignore_mask)) {
			int len;

			RLF(list[j]).type = RLF(cres + i);
			RLF(list[j]).value_set = FALSE;
			RLF(list[j]).is_constraint = TRUE;
	
			len = strlen(NAME((&RLF(list[j])), resource_type));
			if (len > max_type)
				max_type = len;
			len = strlen(NAME((&RLF(list[j])), resource_name));
			if (len > max_name)
				max_name = len;
			j++;
		}
	}

	/* sort the list by TYPE followed by NAME in alphabetical order */
	qsort(RLF(list), RLF(list_length), sizeof(struct resource_entry),
								      by_name);

	RLF(resources_table) = XtVaCreateWidget("resourcesTable",
						ukcRowColWidgetClass,
						RLF(resources_viewport),
						NULL);

	vals = (ArgList)XtMalloc(sizeof(Arg) * RLF(list_length));

	if (object != NULL) {
		char *tmp;
		/* get the current values of an object to be edited */
		if (object->is_remote)
			DisallowEditing();
		for (i = 0; i < RLF(list_length); i++) {
			tmp = XtMalloc(10);
			bzero(tmp, 10);
			XtSetArg(vals[i],
				NAME((&RLF(list[i])), resource_name),
				tmp);
		}
		ObjectGetValues(object, vals, RLF(list_length), FALSE, TRUE);
	} else {
		for (i = 0; i < RLF(list_length); i++) {
			XrmValuePtr val;
			char *tmp;

			val = get_default_value(RLF(resources_table),
						RLF(list[i]).type);
			if (val->addr == NULL) {
				tmp = XtMalloc(sizeof(XtPointer));
				bzero(tmp, sizeof(XtPointer));
			} else {
				tmp = XtMalloc(val->size);
				XtBCopy(val->addr, tmp, val->size);
			}
			XtSetArg(vals[i], NAME((&RLF(list[i])), resource_name), tmp);
		}

		if (dummy_object == NULL) {
			object = dummy_object = CreateObjectRecord();
			object->name = XtNewString("undefined");
			object->instance = GetHeadObject()->instance;
		} else
			object = dummy_object;

		object->class = class;
		RLF(object) = object;
	}

	/* format string for the full resource name */
	if (editor_resources.sort_by_type)
		(void)sprintf(format, "%%-%ds : %%-%ds =", max_type, max_name);
	else
		(void)sprintf(format, "%%-%ds : %%-%ds =", max_name, max_type);

	/* create the rows and columns of the available resources */
	/* each row has 2 columns, the "resource Type, Name, and equals sign"
         * and the current value in a suitable, editable, widget form. */
	for (max_name = 0, i = 0; i < RLF(list_length); i++) {
		if (editor_resources.sort_by_type)
			(void)sprintf(tmp, format,
					NAME((&RLF(list[i])), resource_type),
					NAME((&RLF(list[i])), resource_name));
		else
			(void)sprintf(tmp, format,
					NAME((&RLF(list[i])), resource_name),
					NAME((&RLF(list[i])), resource_type));

		RLF(list[i]).resource_name = XtVaCreateManagedWidget(
					"resourceName", ukcCommandWidgetClass,
					RLF(resources_table), XtNlabel, tmp,
					NULL);
		if (RLF(list[i]).is_constraint) {
			/* This resource is a constraint resource
			 * so display it in resource video to highlight it.
			 */
			Pixel fg, bg;

			XtVaGetValues(RLF(list[i]).resource_name,
					XtNforeground, &bg,
					XtNbackground, &fg, NULL);
			XtVaSetValues(RLF(list[i]).resource_name,
					XtNforeground, fg,
					XtNbackground, bg,
					XtNhighlightColor, fg,
					NULL);
		}
		/* when button is pressed give some context help */
		XtAddCallback(RLF(list[i]).resource_name, XtNcallback,
				GiveResourceHelp, (XtPointer)&(RLF(list[i])));
		RLF(list[i]).resource_value = EditableResource(
						RLF(resources_table),
						object,
						class,
						RLF(list[i]).type,
						(XtPointer)vals[i].value,
						(XtPointer)&RLF(list[i]),
						force_read_only);
		XtFree((char *)vals[i].value);
	}
	XtManageChild(RLF(resources_table));
	XtVaSetValues(RLF(resources_table), XtNx, 0, XtNy, 0, NULL);
	XtFree((char *)vals);
	if (object->is_remote)
		AllowEditing();
}


void
SaveInitialValue(entry, value)
XtPointer entry;
XrmValuePtr value;
{
	struct resource_entry *res = (struct resource_entry *)entry;
	
	if (value->addr == NULL) {
		res->value = NULL;
		return;
	}

	if (IsStringType(res->type->resource_type)) {
		Widget w;

		w = resource_lists[FindResourceList(res)].resources_table;
		res->value = XtNewString(value->addr);
		XtAddCallback(w, XtNdestroyCallback, release_storage,
				res->value);
	} else
		XtBCopy(value->addr, &res->value, value->size);
}


/* This Callback is used by different widgets in the a resource list to pass
 * the new value for a specific resource on to either the widget instance
 * being edited or saved in a variable to be used as a preset argument.
 * "client_data" should be a pointer to the resource_entry in a specific
 * resource list.  "call_data" should be the new value of the resource_entry.
 * If a widget instance is being edited the new value is also logged in the
 * instance's associated Creation object.
 */
void
NewResourceValue(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XrmValue to;
	XrmValue from;
	Cardinal list_num = 0;
	unsigned long swaptest = 1;
	struct resource_entry *entry = (struct resource_entry *)client_data;


	list_num = FindResourceList(entry);

	if (resource_lists[list_num].object != NULL
	    && resource_lists[list_num].object != dummy_object) {
		/* update the instance and log the change */
		Arg args[1];
		Boolean okay_to_save = FALSE;
		XtPointer current_value;
		XtPointer aligned_data = call_data;

		XtSetArg(args[0], entry->type->resource_name, &current_value);
		(void)ObjectGetValues(resource_lists[list_num].object,
					(ArgList)args, 1, FALSE, FALSE);

		if (IsStringType(entry->type->resource_type)) {
			if (current_value != NULL && call_data != NULL &&
			    strcmp((char *)current_value, (char *)call_data)
			    == 0)
				return;
			else {
				XtSetArg(args[0], entry->type->resource_name,
							call_data);
				ObjectSetValues(resource_lists[list_num].object,
							(ArgList)args, 1);
			}
		} else {
			if (!*(char *)&swaptest)
				switch (entry->type->resource_size) {
					case 1: /* Boolean or other char */
						aligned_data = (XtPointer)
						     ((int)aligned_data << 24);
						break;
					case 2: /* short */
						aligned_data = (XtPointer)
						     ((int)aligned_data << 16);
						break;
					default:
						break;
				};
			if (bcmp(&current_value, &aligned_data,
			    entry->type->resource_size) == 0)
				return;
			else {
				XtSetArg(args[0], entry->type->resource_name,
							call_data);
				ObjectSetValues(resource_lists[list_num].object,
							(ArgList)args, 1);
			}
		}

		if (!IsStringType(entry->type->resource_type)) {
			from.size = entry->type->resource_size;
			from.addr = (XtPointer)&aligned_data;
			to.size = 0;
			to.addr = NULL;
			okay_to_save = DoObjectConversion(
					resource_lists[list_num].object,
					entry->type->resource_type, &from,
					XtRString, &to);
		} else {
			to.addr = (String)call_data;
			okay_to_save = TRUE;
		}

		if (okay_to_save)
			LogResourceSetting(resource_lists[list_num].object,
						entry->type->resource_name,
						(String)to.addr,
						entry->is_constraint);
	}
	entry->value_set = TRUE;
	entry->value = call_data;
}


/* Initializes the internals of the three available resource lists.  The
 * three Widget IDs passed are the viewport widgets that the resource tables
 * are created as the child of.
 */
void
InitializeResourceLists(app_res, create_res, edit_res)
Widget app_res, create_res, edit_res;
{
	Cardinal i = 0;

	resource_lists = (struct resource_list_details *)
				XtMalloc(sizeof(struct resource_list_details)
				* MAX_LISTS);
	num_resource_lists = 3;
	free_resource_lists = 0;

	for (i = 0; i < MAX_LISTS; i++) {
		resource_lists[i].object = NULL;
		resource_lists[i].resources_table = NULL;
		resource_lists[i].res = NULL;
		resource_lists[i].cres = NULL;
		resource_lists[i].list = NULL;
		resource_lists[i].list_length = 0;
	}
	resource_lists[APPLICATION_RESOURCES].resources_viewport = app_res;
	resource_lists[NEW_WIDGET_RESOURCES].resources_viewport = create_res;
	resource_lists[INSTANCE_RESOURCES].resources_viewport = edit_res;

	/* mark all the other entries as being "free" */
	for (i = 3; i < MAX_LISTS; i++)
		(void)XukcAddObjectToList(&free_resource_lists, (XtPointer)i,
								FALSE);
}


/* Return the object in a resource entry .. used to find the parent */
ObjectPtr
GetResourceObject(entry)
struct resource_entry *entry;
{
	Cardinal resource_list_number;

	resource_list_number = FindResourceList(entry);
	return (RLF(object));
}


/* looks through a specific resource list and builds up an argument list
 * to be used by XtSetValues() or XtCreateWidget() of any preset resources,
 * (values that are different from the defaults).
 * RETURNS: a pointer to the head of the argument list.  num_set contains
 *          the number of arguments in this list.  The list should be freed
 *	    after use.
 */
ArgList
GetArgumentList(resource_list_number, num_set)
Cardinal resource_list_number, *num_set;
{
	ArgList args;
	Cardinal num_res = RLF(list_length);
	Cardinal i, j;

	/* find number of resources set */
	for (*num_set = i = 0; i < num_res; i++)
		if (RLF(list[i].value_set))
			(*num_set)++;

	if (*num_set == 0)
		return (NULL);

	args = (ArgList)XtMalloc(sizeof(Arg) * *num_set);

	/* copy over the set resources */
	for (j = i = 0; i < num_res; i++)
		if (RLF(list[i].value_set)) {
			args[j].name = RLF(list[i].type->resource_name);
			XukcCopyToArgVal((char *)&(RLF(list[i].value)),
					&(args[j].value),
					RLF(list[i].type->resource_size));
			j++;
		};

	return (args);
}


/* Given a list of XtSetValues() arguments convert the resource values to
 * strings and log them in the resource settings list for the given object.
 */
void
LogPresetArguments(object, args, num_args)
ObjectPtr object;
ArgList args;
Cardinal num_args;
{
	XtResourceList entry;
	XrmValue to;
	XrmValue from;
	Cardinal i, num;
	Widget w;
	Boolean is_constraint;

	if (num_args == 0)
		return;

	if (object->instance == NULL)
		XtError("Cannot LogPresetArguments() on a NULL object");

	for (i = 0; i < num_args; i++) { 
		entry = GetAResource(object, args[i].name, FALSE, (int *)&num,
					&is_constraint);
		if (entry == NULL) /* ||| change this message ||| */
			XtError("cannot find resource");

		if (!IsStringType(entry[num].resource_type)) {
			from.size = entry[num].resource_size;
			from.addr = (XtPointer)&args[i].value;
			to.size = 0;
			to.addr = NULL;
			if (DoObjectConversion(object,
			    entry[num].resource_type, &from, XtRString, &to)) {
				w = object->instance;
				object->instance = NULL;
				LogResourceSetting(object, args[i].name,
							(String)to.addr,
							is_constraint);
				object->instance = w;
			}
		} else {
			w = object->instance;
			object->instance = NULL;
			LogResourceSetting(object, args[i].name,
						(String)args[i].value,
						is_constraint);
			object->instance = w;
		}
		XtFree((char *)entry);
	}
}


/* called by routines in geometry.c to notify a resource_list UKCValue widget
 * that its value has changed and must be updated via XtSetValues() */
void
SetResourceObjectValue(resource_list_number, res_name, value)
Cardinal resource_list_number;
String res_name;
int value;
{
	Cardinal num_res = RLF(list_length);
	Cardinal i;

	/* find the resource to set */
	for (i = 0; i < num_res; i++)
		if (strcmp(RLF(list[i].type->resource_name), res_name) == 0)
			break;

	if (i == num_res) {
		XtWarning("unable to find resource in SetResourceObjectValue()");
		return;
	}
	XtVaSetValues(RLF(list[i].resource_value), XtNvalue, value, NULL);
}


/* called whenever the text in a text widget has been changed and is deemed
 * to be valid, ie. the user moves the cursor out of the text widget or
 * presses <Return>.   As UpdateTextResource() is a translation table action
 * it takes one parameter .. the address of a pointer to the resource_entry
 * that we are editing.  This gives information as to the type of the
 * resource and whether we need to change the edited string to this type.
 * Once converted NewResourceValue() is called to register the change */
void
UpdateTextResource(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	static String new_value = NULL;
	XtPointer addr;
	struct resource_entry *entry;
	XrmValue to, from;
	Cardinal list_num;

	if (*num_params != 1) /* ||| better error message ||| */
		XtError("bad number of parameters to UpdateTextResource()");

	new_value = XukcGetTextWidgetString(w);
	(void)sscanf(params[0], "%ld", &addr);
	entry = (struct resource_entry *)addr;

	/* need to convert from string to right representation?? */
	if (!IsStringType(entry->type->resource_type)) {
		list_num = FindResourceList(entry);
		to.size = 0;
		to.addr = NULL;
		from.size = STRLEN(new_value);
		from.addr = new_value;

		if (resource_lists[list_num].object == NULL) {
			if (!DoConversion(WI_main_layout, XtRString, &from,
			    entry->type->resource_type, &to))
				to.addr = NULL;
		} else {
			if (!DoObjectConversion(resource_lists[list_num].object,
			    XtRString, &from, entry->type->resource_type, &to))
				to.addr = NULL;
		}

		if (to.addr == NULL) { /* conversion failed so undo it! */
			from.addr = (XtPointer)&entry->value;
			from.size = entry->type->resource_size;
			to.addr = NULL;

			if (resource_lists[list_num].object == NULL) {
				if (DoConversion(WI_main_layout,
				    entry->type->resource_type, &from,
				    XtRString, &to))
					XukcNewTextWidgetString(w, (String)to.addr);
			} else {
				if (DoObjectConversion(
				    resource_lists[list_num].object,
				    entry->type->resource_type, &from,
				    XtRString, &to))
					XukcNewTextWidgetString(w, (String)to.addr);
			}
		} else {
			XukcCopyToArgVal(to.addr, (XtArgVal *)&from.addr,
						entry->type->resource_size);
			NewResourceValue(w, addr, (XtPointer)from.addr);
		}
	} else
		NewResourceValue(w, addr, (XtPointer)XtNewString(new_value));

	/* free internal copy of string returned by GetValues */
	XawAsciiSourceFreeString(XawTextGetSource(w));
}


/* same as UpdateTextResource() but works with ukcLabelWidgets (or subclasses)
 * that operate as simple single line edit widgets. */
void
UpdateSimpleTextResource(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	static String new_value = NULL;
	String tmp;
	struct resource_entry *entry;
	XrmValue to, from;
	Cardinal list_num;

	if (new_value != NULL)
		XtFree((char *)new_value);
	XtVaGetValues(w, XtNlabel, &tmp, NULL);
	new_value = XtNewString(tmp);
	entry = (struct resource_entry *)client_data;

	/* need to convert from string to right representation?? */
	if (!IsStringType(entry->type->resource_type)) {
		list_num = FindResourceList(entry);
		to.size = 0;
		to.addr = NULL;
		from.size = STRLEN(new_value);
		from.addr = new_value;

		if (resource_lists[list_num].object == NULL) {
			if (!DoConversion(WI_main_layout, XtRString, &from,
			    entry->type->resource_type, &to))
				to.addr = NULL;
		} else {
			if (!DoObjectConversion(resource_lists[list_num].object,
			    XtRString, &from, entry->type->resource_type, &to))
				to.addr = NULL;
		}

		if (to.addr == NULL) { /* conversion failed so undo it! */
			from.addr = (XtPointer)&entry->value;
			from.size = entry->type->resource_size;
			to.addr = NULL;

			if (resource_lists[list_num].object == NULL) {
				if (DoConversion(WI_main_layout,
				    entry->type->resource_type, &from,
				    XtRString, &to))
					XukcChangeLabel(w, (String)to.addr, TRUE);
			} else if (DoObjectConversion(
				    resource_lists[list_num].object,
				    entry->type->resource_type, &from,
				    XtRString, &to))
					XukcChangeLabel(w, (String)to.addr, TRUE);
		} else {
			XukcCopyToArgVal(to.addr, (XtArgVal *)&from.addr,
						entry->type->resource_size);
			NewResourceValue(w, client_data, (XtPointer)from.addr);
		}
	} else
		NewResourceValue(w, client_data,
					(XtPointer)XtNewString(new_value));
}


/* same as UpdateTextResource() but for callback lists */
void
UpdateCallbackResource(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	static String new_value = NULL;
	XtPointer addr;
	struct resource_entry *entry;
	XrmValue to, from;
	Cardinal list_num;
	XtCallbackList new_callbacks = NULL;
	XtCallbackList old_callbacks = NULL;

	if (*num_params != 1) /* ||| better error message ||| */
		XtError("bad number of parameters to UpdateCallbackResource()");

	new_value = XukcGetTextWidgetString(w);
	(void)sscanf(params[0], "%ld", &addr);
	entry = (struct resource_entry *)addr;

	/* need to convert from string to callback list */

	list_num = FindResourceList(entry);
	to.addr = (XtPointer)&new_callbacks;
	to.size = sizeof(XtCallbackList);
	from.addr = new_value;
	from.size = STRLEN(new_value);

	if (resource_lists[list_num].object == NULL) {
		if (!DoConversion(WI_main_layout, XtRString, &from,
		    entry->type->resource_type, &to))
			to.addr = NULL;
	} else {
		if (!DoObjectConversion(resource_lists[list_num].object,
		    XtRString, &from, entry->type->resource_type, &to))
			to.addr = NULL;
	}

	if (to.addr == NULL) { /* conversion failed so undo it! */
		from.addr = (XtPointer)&entry->value;
		from.size = entry->type->resource_size;
		to.addr = NULL;

		if (resource_lists[list_num].object == NULL) {
			if (DoConversion(WI_main_layout,
			    entry->type->resource_type, &from,
			    XtRString, &to))
				XukcNewTextWidgetString(w, (String)to.addr);
		} else {
			if (DoObjectConversion(
			    resource_lists[list_num].object,
			    entry->type->resource_type, &from,
			    XtRString, &to))
				XukcNewTextWidgetString(w, (String)to.addr);
		}
		return;
	}

	if (resource_lists[list_num].object == NULL ||
	    resource_lists[list_num].object->is_remote) {
		if (*new_value)
			NewResourceValue(w, addr,
				(XtPointer)new_callbacks);
		return;
	}

	/* get the old callback list, remove all the callbacks that point
	 * to the default callback handler in stubs.c
	 * then add all the new ones ... then store it as a string. Phew!
	 */
	XtVaGetValues(resource_lists[list_num].object->instance,
			entry->type->resource_name,
			&old_callbacks, NULL);

	if (old_callbacks != NULL) {
		RemoveDefaultCallbacks(resource_lists[list_num].object,
				entry->type->resource_name,
				old_callbacks);
		RemoveWcCallbacks(resource_lists[list_num].object,
				entry->type->resource_name,
				old_callbacks);

		if (*new_value == '\0') {
			RemoveResourceSetting(resource_lists[list_num].object,
						entry->type->resource_name,
						entry->is_constraint);
			return;
		}

		while (new_callbacks != NULL && new_callbacks->callback != NULL)
		{
			XtAddCallback(
				resource_lists[list_num].object->instance,
				entry->type->resource_name,
				new_callbacks->callback,
				new_callbacks->closure);
			new_callbacks++;
		}
		LogResourceSetting(resource_lists[list_num].object,
					entry->type->resource_name,
					new_value, entry->is_constraint);
	} else if (*new_value)
		NewResourceValue(w, addr, (XtPointer)new_callbacks);

	/* free internal copy of string returned by GetValues */
	XawAsciiSourceFreeString(w);
}
