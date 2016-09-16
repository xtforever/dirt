#ifndef lint
static char sccsid[] = "@(#)vampire.c	1.2 (UKC) 5/10/92";
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

/* Perform the vampire tapping capability of the Dirt! editor which allows
 * us to suck out the contents of the a running program that way we can
 * edit existing programs
 */

#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Layout.h>
#include <X11/Xukc/ICRM.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

struct _missing_resources {
	String name;
	String value;
	ObjectPtr object;
};


extern Widget WI_dirt;
extern Widget WI_create_name_entry;
extern Widget WI_create_class_entry;

/* externally defined routines */
extern ObjectPtr SearchRecordsForName();
extern ObjectPtr CreateObjectRecord();
extern ObjectPtr GetHeadObject();
extern void AddCreatedObject();
extern Cardinal GetClassFromClassName();
extern Boolean IgnoreClassResource();
extern Boolean IsReadOnly();
extern void AddAppEventHandlers();
extern void AddEventHandlers();
extern WidgetClass GetClass();
extern void StoreConnection();
extern Boolean ConversionExists();
extern Window XukcSelectWindow();
/* in dirt.c */
extern void DisallowEditing();
extern void AllowEditing();
/* in convert.c */
extern Boolean IsStringType();
/* in list.c */
extern Boolean IsShellClass();
/* in log.c */
extern void LogResourceSetting();

static void remote_get_all_values();

/* Module-Wide Variables */
static RemoteHierarchy hierarchy = NULL;
static XtResourceList res = NULL, cres = NULL;
static Cardinal num_res, num_cres;
static Window comms_window = NULL;
static XukcIcrmProc finished_call = NULL;
static ObjectPtr current_get_values_object = NULL;
static ObjectPtr *new_head = NULL;

static Arg args[1000];		/* arguments for the currently read in widget */
static Cardinal num_args = 0;	/* number of arguments in args[] */
static ObjectPtr new_start = NULL;

/* temporary holding variables whilst reading in widget description */
static String class;		/* widget class */
static String res_name;		/* name of a resource to set */
static String res_value;	/* value of a resource to set */
static String arg_name;		/* name of an argument to set */
static String arg_value;	/* value of an argument to set */
static ObjectPtr new_object;	/* the newly created widget */
static String direct_name;

#define CAUGHT_EOT 1
#define CAUGHT_ERR 2
#define CONTINUE_AFTER_ERROR 3

/************************************************************************/

/* see if the resource value returned by RemoteGetValues is the same as the
 * widget class's default value .. if it is then ignore it.  We have to
 * convert the value to a string to do the comparison */
static Boolean
check_arg_for_removal(obj, entry, arg/*, def_arg */)
ObjectPtr obj;
XtResourceList entry;
ArgList arg/*, def_arg*/;
{
	XrmValue from, to, tmpv;
/* ||| hazard warning : use of create_name_entry ||| */
	Widget w = WI_create_name_entry;

	if (IgnoreClassResource(obj->class, entry->resource_name,
	    entry->resource_class, entry->resource_type,
	    (IGNORE | READ_ONLY)) ||
	    (strcmp(entry->resource_class, XtCReadOnly) == 0))
		return TRUE;

	/* The remote conversion failed for some reason so ignore it! */
	if (arg->value == NULL)
		return TRUE;

	from.addr = NULL;
	if (!IsStringType(entry->default_type)) {
		if (strcmp(entry->default_type, XtRCallProc) == 0) {
			/* call the procedure conversion routine ...?? */
			/* ||| this is of course completely wrong 8-) ||| */
			((void (*)())(entry->default_addr))
			    (w, entry->resource_offset, &tmpv);
			from.addr = (XtPointer)&tmpv.addr;
		}
		if (from.addr != NULL ||
		    strcmp(XtRImmediate, entry->default_type) == 0) {
			from.size = entry->resource_size;
			if (from.addr == NULL)
				from.addr = (XtPointer)&entry->default_addr;
			to.size = 0;
			to.addr = NULL;
			if (!DoConversion(w, entry->resource_type, &from,
			    XtRString, &to))
				return FALSE;
		} else
			return TRUE;
	} else if (entry->default_addr == NULL)
		return FALSE;
	else {
		/* convert the default value into the right type ..
		 * then convert it back again.  This gets rid of the
		 * XtExtDefault* strings and gives the real string
		 * that we can compare.
		 */
		from.addr = entry->default_addr;
		from.size = strlen(from.addr) + 1;
		to.size = 0;
		to.addr = NULL;
		if (!DoConversion(w, XtRString, &from, entry->resource_type,
		    &to))
			return FALSE;

		from.addr = XtMalloc(to.size);
		bcopy((char *)to.addr, (char *)from.addr, (int)to.size);
		from.size = to.size;

		to.size = 0;
		to.addr = NULL;

		if (!DoConversion(w, entry->resource_type, &from, XtRString,
		    &to)) {
			XtFree(from.addr);
			return FALSE;
		}
		XtFree(from.addr);
	}
	return (strcmp((char *)arg->value, (char *)to.addr) == 0);
}


/* remove any resource values returned by RemoteGetValues() that are the
 * widget class default values.  We only want to record the actual
 * resource values set for this widget instance
 */
static Cardinal
strip_out_default_values(obj, constraint_list, resl, cresl,
					num_resl, num_cresl, args, num_args)
ObjectPtr obj;
Cardinal *constraint_list;
XtResourceList resl, cresl;
Cardinal num_resl, num_cresl;
ArgList args;
Cardinal num_args;
{
	Cardinal i, j;
#if 0
	ArgList def_args;
	Cardinal num_defs;

	def_args = create_default_args_list(obj, resl, cresl, num_resl,
						num_cresl, &num_defs);
#endif

	/* constraint list contains TRUE or FALSE to signify if resource
	 * is a constraint resource */
	for (j = 0; j < num_args; j++)
		(void)XukcAddObjectToList(constraint_list, (XtPointer)FALSE,
						FALSE);

	/* compare normal resources and strip out resources whose value
	 * is equal to that of the default value */
	for (i = 0; i < num_resl; i++) {
		for (j = 0; j < num_args; j++)
			if (strcmp(resl[i].resource_name, args[j].name)
			    == 0) {
				if (check_arg_for_removal(obj, &resl[i],
				    &args[j])/*, &def_args[j]*/) {
					Arg tmp_arg;

					(void)XukcRemoveEntryOnList(
							constraint_list, j + 1);
					tmp_arg = args[j];
					for (; j < num_args - 1; j++)
						args[j] = args[j + 1];
					num_args--;
					args[num_args] = tmp_arg;
				}
				break;
			}
	}

	for (i = 0; i < num_cresl; i++) {
		for (j = 0; j < num_args; j++)
			if (strcmp(cresl[i].resource_name, args[j].name)
			    == 0) {
				if (check_arg_for_removal(obj, &resl[i],
				    &args[j]/*, &def_args[j]*/)) {
					Arg tmp_arg;

					(void)XukcRemoveEntryOnList(
							constraint_list, j + 1);
					tmp_arg = args[j];
					for (; j < num_args - 1; j++)
						args[j] = args[j + 1];
					num_args--;
					args[num_args] = tmp_arg;
				} else
					(void)XukcChangeObjectOnList(
							*constraint_list,
							j + 1, (XtPointer)TRUE);
				break;
			}
	}
	return (num_args);
}


static void
next_get_values_transfer(client_num, window)
Cardinal client_num;
Window window;
{
	ObjectPtr next_object, head = GetHeadObject();

	if (current_get_values_object == NULL) {
		XukcFreeRemoteHierarchy(hierarchy, FALSE);
		hierarchy = NULL;
		(*finished_call)(client_num, window, FALSE, (XtPointer)NULL,
							    (XtPointer)NULL);
	}

	next_object = current_get_values_object;

	do {
		/* find a node not visited before */
		if (next_object->children == NULL ||
		    next_object->children->remote_done) {
			if (next_object->prev == NULL) {
				next_object = next_object->parent;
			} else {
				next_object = next_object->prev;
			}
		} else do {
			next_object = next_object->children;
			while (next_object->next != NULL)
				next_object = next_object->next;
		} while (next_object->children != NULL);
	} while (next_object != NULL && next_object != head
		 && next_object->remote_done);

	current_get_values_object = next_object;

	if (next_object != NULL && next_object != head)
		remote_get_all_values(next_object);
	else {
		XukcFreeRemoteHierarchy(hierarchy, FALSE);
		hierarchy = NULL;
		(*finished_call)(client_num, window, TRUE, (XtPointer)NULL,
							    (XtPointer)NULL);
		GiveMessage("icrmIO", "finishedTransfer",
				"Finished remote data transfer OK", 0);
		AllowEditing();
	}
}


static XukcIcrmProc
finished_get_values(client_num, window, status, arg1, arg2)
Cardinal client_num;
Window window;
Boolean status;
XtPointer arg1, arg2;
{
	ArgList args = (ArgList)arg1;
	Cardinal num_args = (Cardinal)arg2;
	Cardinal i, constraint_list;
	Widget instance;

	if (current_get_values_object == NULL)
		XtError("Internal error in SuckWindow(finished_get_values)");

	if (!status) { /* GetValues failed for some reason */
		XukcFreeRemoteHierarchy(hierarchy, FALSE);
		hierarchy = NULL;
		(*finished_call)(client_num, window, FALSE, (XtPointer)NULL,
							    (XtPointer)NULL);
		GiveMessage("icrmIO", "transferFailed",
				"Remote Data transfer failure", 0);
		AllowEditing();
		return;
	}

	constraint_list = 0;
	num_args = strip_out_default_values(current_get_values_object,
						&constraint_list,
						res, cres, num_res, num_cres,
						args, num_args);
	instance = current_get_values_object->instance;
	current_get_values_object->instance = NULL;

	for (i = 0; i < num_args; i++) {
		/* pull out special interest resources .. x, y, width etc */
		if (strcmp(args[i].name, "x") == 0 ||
		    strcmp(args[i].name, "y") == 0 ||
		    strcmp(args[i].name, "width") == 0 ||
		    strcmp(args[i].name, "height") == 0 ||
		    strcmp(args[i].name, "borderWidth") == 0) {
			LogResourceSetting(current_get_values_object,
						args[i].name,
						(String)args[i].value, FALSE);
			args[i].value = NULL;
		}
	}

	current_get_values_object->instance = instance;

	for (i = 0; i < num_args; i++)
		if (args[i].value != NULL)
			LogResourceSetting(current_get_values_object,
				args[i].name, (String)args[i].value,
				(Boolean)XukcGetObjectOnList(constraint_list,
								i + 1));

	/* NB: the memory allocated for the args[?].name's will be freed
	 *     by the ICRM mechanism after this function call returns.
	 */
	XtFree((char *)args);
	current_get_values_object->remote_done = TRUE;
	next_get_values_transfer(client_num, window);
	XukcDestroyList(&constraint_list, FALSE);
}

/* Build up a list of the possible resources in the remote application's
 * widget instance.  Then call the ICRM function to retrieve the current
 * values and only save those that differ from the default values.
 * The standard Dimensions; width, height, borderWidth and the standard
 * Positions; x and y are saved as preset values.  All other values
 * are saved as normal resource changes.  The whole hierarchy can then
 * be saved and treated as any other Dirt! specification.
 */
static void
remote_get_all_values(object)
ObjectPtr object;
{
	ArgList args;
	Cardinal i, j;

	XtInitializeWidgetClass(GetClass(object->class));

	if (res != NULL)
		XtFree((char *)res);
	if (cres != NULL)
		XtFree((char *)cres);

	num_res = num_cres = 0;
	cres = res = NULL;

	XtGetResourceList(GetClass(object->class), &res, &num_res);
	if (!NO_PARENT(object))
		XtGetConstraintResourceList(GetClass(object->parent->class),
						&cres, &num_cres);

	args = (ArgList)XtMalloc(sizeof(Arg) * (num_res + num_cres));
	for (i = j = 0; i < num_res; i++)
		if (ConversionExists(res[i].resource_type)) {
			XtSetArg(args[j], res[i].resource_name, NULL); j++;
		}
	for (i = 0; i < num_cres; i++)
		if (ConversionExists(cres[i].resource_type)) {
			XtSetArg(args[j], cres[i].resource_name, NULL); j++;
		}

	if (!XukcRemoteGetValues(XukcDirtEditor, comms_window,
	    object->instance, args, j, (XukcIcrmProc)finished_get_values)) {
		/* return with an error */
		XukcFreeRemoteHierarchy(hierarchy, FALSE);
		hierarchy = NULL;
		(*finished_call)(XukcDirtEditor, comms_window, FALSE,
					(XtPointer)NULL, (XtPointer)NULL);
		GiveMessage("icrmIO", "transferFailed",
				"Remote Data transfer failure", 0);
		AllowEditing();
		return;
	}

	current_get_values_object = object;
}


start_get_values_sequence(new_start)
ObjectPtr new_start;
{
	current_get_values_object = new_start;
	remote_get_all_values(new_start);
}


/* Build up a creation list from the widget hierarchy that was returned
 * from the remote application.  We shall use the remote application's
 * instances to edit and all editing functions are done on the remote
 * application.  The "is_remote" flag in the Object structure is set
 * to indicate this.
 */
static Cardinal
create_from_hierarchy(current, parent)
RemoteHierarchy current;
ObjectPtr parent;
{
	Cardinal result = CAUGHT_EOT;

	if (current == NULL)
		return (result);

	new_object = CreateObjectRecord();
	new_object->name = current->remote_name;
	new_object->instance = current->remote_widget;
	new_object->parent = parent;
	new_object->is_remote = TRUE;
	new_object->remote_done = FALSE; /* set to true when GetValues called */
	new_object->in_app = TRUE;
	new_object->managed = current->managed;
	new_object->class = GetClassFromClassName(current->widget_class);

	if (parent == GetHeadObject()) /* head of list */
		new_start = new_object;
	else
		AddCreatedObject(new_object);

	XtFree(current->widget_class);

	result = create_from_hierarchy(current->children, new_object);
	if (result == CAUGHT_ERR)
		return (result);
	return (create_from_hierarchy(current->next, parent));
}


static Window
search_all_children(dpy, children, num_children)
Display *dpy;
Window *children;
unsigned int num_children;
{
	Window root, parent, ret_win;
	Window *grand_children;
	unsigned int num_grand;
	int i;

	/* check the children */
	for (i = 0; i < num_children; i++)
		if (XukcIsClientWindow(XukcDirtEditor, children[i]))
			return (children[i]);

	/* now check the grand children downwards */
	for (i = 0; i < num_children; i++) {
		if (XQueryTree(dpy, children[i], &root, &parent,
		    &grand_children, &num_grand)) {
			if (ret_win = search_all_children(dpy, grand_children,
			    num_grand)) {
				XFree((char *)grand_children);
				return (ret_win);
			}
		} else
			return (NULL);
		XFree((char *)grand_children);
	}
	return (NULL);
}


static Window
find_comms_window(dpy, window)
Display *dpy;
Window window;
{
	Window root, parent, *children;
	unsigned int num_children;

	while(window != NULL && !XukcIsClientWindow(XukcDirtEditor, window) &&
	      XQueryTree(dpy, window, &root, &parent, &children, &num_children)) {
		if (XukcIsClientWindow(XukcDirtEditor, parent))
			window = parent;
		else if (parent != root)
			window = parent; /* go up the tree to just below root */
		else
			window = search_all_children(dpy, children,
							num_children);
		XFree((char *)children);
	}
	/* ||| if window still == NULL then do a client for client check
	 *     via XukcRemoteWindowToWidget() and grab the first one ||| */
	return (window);
}


static XukcIcrmProc
process_hierarchy(client_num, window, status, arg1, arg2)
Cardinal client_num;
Window window;
Boolean status;
XtPointer arg1, arg2;
{
	extern char app_class[];
	Cardinal err;

	if (client_num != XukcDirtEditor)
		return;

	err = create_from_hierarchy(hierarchy, GetHeadObject());

	if (err == CAUGHT_EOT) {
		*new_head = new_start;
		(void)strcpy(app_class, new_start->name); /* ||| */
		GiveMessage("icrmIO", "stillInTransfer",
			"Finished first part of transfer .. please wait", 0);
		start_get_values_sequence(new_start);
	} else {
		/* ||| free all the memory we created ||| */
		if (finished_call == NULL)
			XtWarning("finished_call == NULL in process_hierarchy"); 
		comms_window = NULL;
		AllowEditing();
	}
}

/****** Module Exported Routines ******/

Boolean
SuckWindow(w, head, called_when_done)
Widget w;
ObjectPtr *head;
XukcIcrmProc called_when_done;
{
	Cardinal client_list;
	Window window;

	/* transfer still going on?? */
	if (hierarchy != NULL &&
	    XukcStillDoingTransfers(XukcDirtEditor, comms_window)) {
		GiveMessage("icrmIO", "stillDoingTransfer",
				"A transfer is still in progress",
				0);
		return FALSE;
	}

	AllowEditing();

	if ((client_list = XukcClientWindows(XukcDirtEditor)) == 0) {
		GiveWarning(WI_dirt, "icrmIO", "noAvailableClients",
				"No running clients suitable for access",
				0);
		return FALSE;
	}

	if ((window = XukcSelectWindow(w)) == NULL)
		return FALSE;

	comms_window = find_comms_window(XtDisplay(w), window);

	if (!XukcGetClientWidgetTree(XukcDirtEditor, comms_window, &hierarchy,
	    (XukcIcrmProc)process_hierarchy))
		return FALSE;
	
	new_object = new_start = NULL;
	finished_call = called_when_done;
	new_head = head;
	DisallowEditing();
	return TRUE;
}


Window
CurrentCommsWindow()
{
	return (comms_window);
}
