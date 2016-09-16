#ifndef lint
static char sccsid[] = "@(#)connect.c	1.2 (UKC) 5/10/92";
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
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/Layout.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"
#include "dirt_bindings.h"

extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern Boolean NoObjectsCreated();
extern ObjectPtr SearchRecords();
extern String CvtToVariableName();
extern String GetComplexName();
extern void DrainUIEventQueue();
extern WidgetClass GetClass();
extern Boolean IsShellClass();

/* linked lists that hold the connections */
static Cardinal parent_list = 0;
static Cardinal from_list = 0;
static Cardinal to_list = 0;
static Cardinal direction_list = 0;

static Cardinal app_parent_list = 0;
static Cardinal app_from_list = 0;
static Cardinal app_to_list = 0;
static Cardinal app_direction_list = 0;

static Cardinal template_parent_list = 0;
static Cardinal template_from_list = 0;
static Cardinal template_to_list = 0;
static Cardinal template_direction_list = 0;

static Boolean undos_registered = FALSE;
static ObjectPtr undo_parent = NULL;
static ObjectPtr undo_from;
static ObjectPtr undo_to;
static Positioning undo_dir; 

static Boolean connections_on = FALSE;

#define SET_LISTS(a) { \
	parent_list = a ? app_parent_list : template_parent_list; \
	from_list = a ? app_from_list : template_from_list; \
	to_list = a ? app_to_list : template_to_list; \
	direction_list = a ? app_direction_list : template_direction_list; \
}
#define SAVE_LISTS(a) { \
	if (a) { \
		app_parent_list = parent_list; \
		app_from_list = from_list; \
		app_to_list = to_list; \
		app_direction_list = direction_list; \
	} else { \
		template_parent_list = parent_list; \
		template_from_list = from_list; \
		template_to_list = to_list; \
		template_direction_list = direction_list; \
	} \
}


/* ARGSUSED */
static void
undo_connection(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (undo_parent == NULL)
		SayNothingToUndo("Undo Connection");
	else {
		XukcLayoutRemoveConnection(
				(UKCLayoutWidget)undo_parent->instance,
				undo_from->instance,
				undo_to->instance, undo_dir);
		undo_parent = NULL;
	}
}


/* ARGSUSED */
static void
undo_disconnect(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (undo_parent == NULL)
		SayNothingToUndo("Undo Disconnection");
	else {
		XukcLayoutAddConnection(
				((UKCLayoutWidget)undo_parent->instance),
				undo_from->instance,
				undo_to->instance, undo_dir);
		undo_parent = NULL;
	}
}


static void
show_existing_connections(obj, on)
ObjectPtr obj;
Boolean on;
{
	if (obj == NULL)
		return;

	show_existing_connections(obj->children, on);
	show_existing_connections(obj->next, on);

	if (XtIsSubclass(obj->instance, ukcLayoutWidgetClass))
		XukcDisplayLayoutConnections(obj->instance, on);
}


struct layout_weight {
	Widget picked;
	Widget child;
	Widget layout;
	Cardinal level;
};

	
static struct layout_weight *
new_weight(w)
Widget w;
{
	Widget sh = w;
	struct layout_weight *weight = XtNew(struct layout_weight);

	weight->picked = NULL;
	weight->child = NULL;
	weight->layout = NULL;
	weight->level = 0;

	while (sh != NULL && !XtIsShell(sh)) {
		sh = XtParent(sh);
		weight->level++;
	}
	if (sh == NULL)
		XtError("internal error: failed to find shell in connect.c:new_weight()");
	return (weight);
}


static void
find_layout_weighting(w, weight_list, shell)
Widget w;
Cardinal *weight_list;
Widget *shell;
{
	Widget sh;
	struct layout_weight *weight = NULL;

	*weight_list = 0;
	*shell = NULL;

	if (w == NULL)
		return;

	for (sh = w; sh != NULL && !XtIsShell(sh); )
		sh = XtParent(sh);
	*shell = sh;

	if (XtIsSubclass(w, ukcLayoutWidgetClass)) {
		weight = new_weight(w);
		weight->picked = w;
		weight->child = w;
		weight->layout = w;
		(void)XukcAddObjectToList(weight_list,
						(XtPointer)weight, FALSE);
	}

	for (sh = w; XtParent(sh) != NULL && !XtIsShell(sh); ) {
		if (XtIsSubclass(XtParent(sh), ukcLayoutWidgetClass)) {
			weight = new_weight(XtParent(sh));
			weight->picked = w;
			weight->child = sh;
			weight->layout = XtParent(sh);
			(void)XukcAddObjectToList(weight_list,
						(XtPointer)weight, FALSE);
		}
		sh = XtParent(sh);
	}
}


static Widget
find_layout_parent(from_list, to_list, from_child, to_child)
Cardinal from_list, to_list;
Widget *from_child, *to_child;
{
	struct layout_weight **from_weights;
	struct layout_weight **to_weights;
	Cardinal from_num, to_num, i, j, k;
	Boolean swapped_lists = FALSE;

	from_weights = (struct layout_weight **)XukcGetListPointer(
							from_list, &from_num);
	to_weights = (struct layout_weight **)XukcGetListPointer(
							to_list, &to_num);

	if (from_weights == 0 || to_weights == 0)
		return (NULL);

	if (from_weights[0]->level < to_weights[0]->level) {
		struct layout_weight **tmp_weights;

		tmp_weights = from_weights;
		i = from_num;
		from_weights = to_weights;
		from_num = to_num;
		to_weights = tmp_weights;
		to_num = i;
		swapped_lists = TRUE;
	}
	j = i = 0;
	while (j < to_num && from_weights[i]->level != to_weights[j]->level) {
		k = i;
		while (k < from_num
		       && from_weights[k]->level != to_weights[j]->level)
				k++;
		if (k == from_num)
			j++;
		else
			i = k;
	}

	if (i == from_num || j == to_num)
		return (NULL);
	
	do {
		if (to_weights[j]->layout == from_weights[i]->layout) {
			*from_child = swapped_lists ? to_weights[j]->child :
						      from_weights[i]->child;
			*to_child = swapped_lists ? from_weights[i]->child :
						      to_weights[j]->child;
			if (*from_child == *to_child)
				return (NULL);
			else
				return (to_weights[j]->layout);
		}
		j++;
		i++;
	} while (j < to_num && i < from_num);
	return (NULL);
}


static Boolean
prompt_user_for_connecting_line(vertically)
Boolean vertically;
{
	int x1, y1, x2, y2;
	Position pos_x1, pos_y1, pos_x2, pos_y2;
	Positioning direction;
	Widget from, to, layout = NULL;
	Window w1, w2;
	Widget w, from_shell, to_shell;
	Cardinal from_weight_list, to_weight_list;
	ObjectPtr start;
	
	start = GetStartOfList(TRUE);

	if (start->is_remote) {
		GiveWarning("illegalObject", "layoutConnections",
			"Cannot access Layout Objects in external application",
			0);
		return (FALSE);
	}

	w = start->instance;

	if (!undos_registered) {
		RegisterUndoProcedure(CONNECT_HORIZ, undo_connection);
		RegisterUndoProcedure(CONNECT_VERT, undo_connection);
		RegisterUndoProcedure(DISCONNECT_HORIZ, undo_disconnect);
		RegisterUndoProcedure(DISCONNECT_VERT, undo_disconnect);
		undos_registered = TRUE;
	}

	/* prompt the user to drag out a line */
	XukcDragLine(w, &w1, &x1, &y1, &w2, &x2, &y2, (void (*)())NULL);

	/* find the two widgets we are trying to connect */
	from = XtWindowToWidget(XtDisplay(w), w1);
	to = XtWindowToWidget(XtDisplay(w), w2);

	if (from == NULL || to == NULL)
		return (FALSE);

	XtTranslateCoords(from, (Position)x1, (Position)y1, &pos_x1, &pos_y1);
	XtTranslateCoords(to, (Position)x2, (Position)y2, &pos_x2, &pos_y2);

	/* work out the direction of the connection */
	if (vertically) {
		if (pos_y1 < pos_y2)
			direction = vertical_below;
		else
			direction = vertical_above;
	} else {
		if (pos_x1 < pos_x2)
			direction = horizontal_after;
		else
			direction = horizontal_before;
	}

	find_layout_weighting(from, &from_weight_list, &from_shell);
	find_layout_weighting(to, &to_weight_list, &to_shell);

	if (from_shell != to_shell) {
		XtWarning("Different parents: cannot make connection");
		return (FALSE);
	}
		
	layout = find_layout_parent(from_weight_list, to_weight_list,
					&from, &to);
	if (layout == NULL)
		return (FALSE);

	if (!XtIsSubclass(layout, ukcLayoutWidgetClass)) {
		XtWarning("Parent is not a UKCLayoutWidget: cannot make connection");
		return (FALSE);
	}

	undo_parent = SearchRecords(start, layout);
	undo_from = SearchRecords(start, from);
	undo_to = SearchRecords(start, to);
	undo_dir = direction;
	return (TRUE);
}


/**** Module Exported Routines ****/

#define OPPOSITE(d) (d == vertical_above ? vertical_below : d == vertical_below ? vertical_above : d == horizontal_before ? horizontal_after : horizontal_before)

/* ARGSUSED */
void
DisconnectWidgets(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Boolean ok = TRUE;

	if (NoObjectsCreated() || parent_list == 0) {
		GiveWarning(w, "noObjects", "disconnectWidgets",
				"No Objects To Disconnect", 0);
		return;
	}

	while (ok && prompt_user_for_connecting_line((Boolean)client_data)) {
		ObjectPtr p, f, t;
		Positioning dir;
		Cardinal i;
		Cardinal lists = 0;

		for (lists = 0; lists < 2; lists++) {
			i = 1;
			SET_LISTS(lists);

			if (parent_list == 0)
				continue;

			while ((p = (ObjectPtr)XukcGetObjectOnList(parent_list, i)) != NULL) {
				f = (ObjectPtr)XukcGetObjectOnList(from_list, i);
				t = (ObjectPtr)XukcGetObjectOnList(to_list, i);
				dir = (Positioning)XukcGetObjectOnList(direction_list, i);
				if (p == undo_parent &&
				   ((f == undo_from && t == undo_to && dir == undo_dir)
				   || (f == undo_to && t == undo_from &&
				       dir == OPPOSITE(undo_dir)))){
					XukcRemoveEntryOnList(&parent_list, i);
					XukcRemoveEntryOnList(&from_list, i);
					XukcRemoveEntryOnList(&to_list, i);
					XukcRemoveEntryOnList(&direction_list, i);
					SAVE_LISTS(lists);
					break;
				}
				i++;
			}
			if (p != NULL)
				break;
		}

		if (p == NULL) {
			XtWarning("Cannot disconnect widgets : no existing connection");
			return;
		}
		XukcLayoutRemoveConnection(
			(UKCLayoutWidget)undo_parent->instance,
			undo_from->instance, undo_to->instance, undo_dir);
		RegisterUndoAction((Boolean)client_data
					? DISCONNECT_VERT : DISCONNECT_HORIZ);
		XukcLayoutRefreshConnections(undo_parent->instance);
		XFlush(XtDisplay(undo_parent->instance));
		ok = !connections_on;
	}
}


void
StoreConnection(parent, from, to, direction)
ObjectPtr parent, from, to;
Positioning direction;
{
	Cardinal found;
	Cardinal list_size;

	SET_LISTS(parent->in_app);

	if (parent_list == 0)
		list_size = 0;
	else
		(void)XukcGetListPointer(parent_list, &list_size);

	/* look for existing connections and ignore them */
	for (found = 1; found <= list_size; found++) {
		if ((ObjectPtr)XukcGetObjectOnList(parent_list, found)
		    != parent)
			continue;

		if (((ObjectPtr)XukcGetObjectOnList(from_list, found) == from)
		    && ((ObjectPtr)XukcGetObjectOnList(to_list, found) == to)
		    && ((Positioning)XukcGetObjectOnList(direction_list, found)
			== direction))
				return; /* connection already exists */

		if (((ObjectPtr)XukcGetObjectOnList(from_list, found) == to)
		    && ((ObjectPtr)XukcGetObjectOnList(to_list, found) == from)
		    && ((Positioning)XukcGetObjectOnList(direction_list, found)
			== OPPOSITE(direction)))
				return; /* connection already exists */
	}

	XukcAddObjectToList(&parent_list, (XtPointer)parent, FALSE);
	XukcAddObjectToList(&from_list, (XtPointer)from, FALSE);
	XukcAddObjectToList(&to_list, (XtPointer)to, FALSE);
	XukcAddObjectToList(&direction_list, (XtPointer)direction, FALSE);
	XukcLayoutAddConnection((UKCLayoutWidget)parent->instance,
				from->instance, to->instance, direction);
	SAVE_LISTS(parent->in_app);
}


/* ARGSUSED */
void
ConnectWidgets(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Boolean ok = TRUE;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "connectWidgets",
				"No Objects To Connect", 0);
		return;
	}

	while (ok && prompt_user_for_connecting_line((Boolean)client_data)) {
		StoreConnection(undo_parent, undo_from, undo_to, undo_dir);
		RegisterUndoAction((Boolean)client_data
					? CONNECT_VERT : CONNECT_HORIZ);
		XukcLayoutRefreshConnections(undo_parent->instance);
		XFlush(XtDisplay(undo_parent->instance));
		ok = !connections_on;
	}
}


Boolean
ConnectionsExist(is_app)
Boolean is_app;
{
	SET_LISTS(is_app);

	if (parent_list == 0 || XukcGetObjectOnList(parent_list, 1) == NULL)
		return (FALSE);
	else
		return (TRUE);
}


/* when an object is destroyed we must remove any UKCLayout connections
 * that we made and update our internal lists.
 */
void
RemoveDyingWidgetConnections(obj)
ObjectPtr obj;
{
	Cardinal i;
	ObjectPtr from, to, parent;

	if (!ConnectionsExist(obj->in_app) || obj->is_remote)
		return;

	SET_LISTS(obj->in_app);

	if (parent_list == 0 ||
	    !((!NO_PARENT(obj)
	    && XtIsSubclass(obj->parent->instance, ukcLayoutWidgetClass))
	    || XtIsSubclass(obj->instance, ukcLayoutWidgetClass)))
		return;

	parent = *(ObjectPtr *)XukcGetListPointer(parent_list, &i);
	for (; i > 0; parent++, i--) {
		from = (ObjectPtr)XukcGetObjectOnList(from_list, i);
		to = (ObjectPtr)XukcGetObjectOnList(to_list, i);

		if (parent == obj || from == obj || to == obj) {
			XukcRemoveEntryOnList(&parent_list, i);
			XukcRemoveEntryOnList(&from_list, i);
			XukcRemoveEntryOnList(&to_list, i);
			XukcRemoveEntryOnList(&direction_list, i);
		}
	};
	SAVE_LISTS(obj->in_app);
}


#define POS(a) (a == horizontal_before ? "horizontal_before" : \
		a == horizontal_after ? "horizontal_after" : \
		a == vertical_above ? "vertical_above" : "vertical_below")

void
SaveConnectionList(fd, is_app)
FILE *fd;
Boolean is_app;
{
	Cardinal i;
	Positioning dir;
	ObjectPtr from, to, parent;
	String name;

	if (!ConnectionsExist(is_app))
		return;

	SET_LISTS(is_app);

	i = 1;
	while ((parent = (ObjectPtr)XukcGetObjectOnList(parent_list, i))
	      != NULL) {
		from = (ObjectPtr)XukcGetObjectOnList(from_list, i);
		to = (ObjectPtr)XukcGetObjectOnList(to_list, i);
		dir = (Positioning)XukcGetObjectOnList(direction_list, i);

		name = GetComplexName(parent);
		fprintf(fd, "%%L %s ", name);
		XtFree(name);

		name = GetComplexName(from);
		fprintf(fd, "; %s ", name);
		XtFree(name);

		name = GetComplexName(to);
		fprintf(fd, "; %s ", name);
		XtFree(name);

		fprintf(fd, "; %s\n", POS(dir));
		i++;
	};
}


void
PrintConnectionList(fd)
FILE *fd;
{
	Cardinal i;
	Positioning dir;
	ObjectPtr from, to, parent;
	String vn;

	if (!ConnectionsExist(TRUE))
		return;

	SET_LISTS(TRUE);
        fprintf(fd, "\n/* widget connection list for UKCLayout widgets */\n");
	vn = CvtToVariableName(GetStartOfList(TRUE)->name);
	fprintf(fd, "struct w_connections %s_widget_connections[] = {\n", vn);
	XtFree(vn);
	i = 1;
	while ((parent = (ObjectPtr)XukcGetObjectOnList(parent_list, i)) != NULL
	      && parent->in_app) {
		from = (ObjectPtr)XukcGetObjectOnList(from_list, i);
		to = (ObjectPtr)XukcGetObjectOnList(to_list, i);
		dir = (Positioning)XukcGetObjectOnList(direction_list, i);

		vn = CvtToVariableName(parent->name);
		fprintf(fd, "\t{ &%s, ", vn);
		XtFree(vn);
		vn = CvtToVariableName(from->name);
		fprintf(fd, "&%s, ", vn);
		XtFree(vn);
		vn = CvtToVariableName(to->name);
		fprintf(fd, "&%s, ", vn);
		XtFree(vn);
		fprintf(fd, "%s },\n", POS(dir));
		i++;
	};
	fprintf(fd, "};\n\n");
}
#undef POS


/* ARGSUSED */
void
ShowLayoutConnections(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	show_existing_connections(GetStartOfList(TRUE), (Boolean)call_data);
	show_existing_connections(GetStartOfList(FALSE), (Boolean)call_data);
	connections_on = (Boolean)call_data;
	DrainUIEventQueue();
}


/* ARGSUSED */
void
ConnectToParent(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	/* ||| NYI:  this will connect the vertical and horizontal
	 * ||| connections to the widgets layout parent, unless any
	 * ||| other connections exist.
	 */
}
