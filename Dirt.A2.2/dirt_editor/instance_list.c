#ifndef lint
static char sccsid[] = "@(#)instance_list.c	1.2 (UKC) 5/10/92";
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

#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

extern Widget WI_instance_list;
extern Widget WI_templates_heirarchy;

extern ObjectPtr GetStartOfList();
extern ObjectPtr SearchRecordsForName();
extern ObjectPtr SearchRecordsForMarkNum();
extern ObjectPtr CurrentlyEditing();
extern void ChangeSelection();

void ChangeMark();

static Cardinal wil = 0;
static Cardinal template_list = 0;
static ObjectPtr picked_object = NULL;


static Cardinal
build_list_strings(object_list, list, indent, num_on_list)
ObjectPtr object_list;
Cardinal *list, indent, num_on_list;
{
	char big_buf[1000];
	Cardinal j;

	if (object_list == NULL)
		return (num_on_list);

	if (!object_list->is_resource) {
		big_buf[0] = '\0';
		for (j = 0; j < indent; j++) (void)strcat(big_buf, "    ");
		(void)strcat(big_buf, object_list->name);
		object_list->mark_num = XukcAddObjectToList(list,
						XtNewString(big_buf), FALSE);
		num_on_list++;

		num_on_list = build_list_strings(object_list->children,
						list, indent+1, num_on_list);
	}
	if (object_list->next != NULL
	    && object_list->next->in_app == object_list->in_app)
		return (build_list_strings(object_list->next, list,
			indent, num_on_list));
	else
		return (num_on_list);
}


/**** Module Exported Routines ****/


ObjectPtr
GetCurrentPicked()
{
	return (picked_object);
}


void
ClearInstanceList(in_app)
Boolean in_app;
{
	if (in_app) {
		XukcDestroyList(&wil, TRUE);
		XukcScrListChange(WI_instance_list, (String *)NULL, 0);
	} else {
		XukcDestroyList(&template_list, TRUE);
		XukcScrListChange(WI_templates_heirarchy, (String *)NULL, 0);
	}
}


void
CreateInstanceList(object_list)
ObjectPtr object_list;
{
	String *list_string = NULL;
	Cardinal i, num_widgets = 0;

	if (object_list == NULL)
		return;

	if (object_list->in_app) {
		XukcScrListChange(WI_instance_list, (String *)NULL, 0);
		XukcDestroyList(&wil, TRUE);

		i = build_list_strings(object_list, &wil, 0, 0);

		if (i > 0)
			list_string = (String *)XukcGetListPointer(wil,
								&num_widgets);

		/* update a list widget to hold the available names for
		   the user to choose from. */
		XukcScrListChange(WI_instance_list, list_string, num_widgets);
	} else {
		XukcScrListChange(WI_templates_heirarchy, (String *)NULL, 0);
		XukcDestroyList(&template_list, TRUE);

		i = build_list_strings(object_list->children, &template_list,
					0, 0);
		if (i > 0)
			list_string = (String *)XukcGetListPointer(
						template_list, &num_widgets);

		/* update a list widget to hold the available names for
		   the user to choose from. */
		XukcScrListChange(WI_templates_heirarchy,
						list_string, num_widgets);
	}

	if (i == 0 || CurrentlyEditing() == NULL) {
		XukcScrListUnselectItems(WI_instance_list);
		XukcScrListUnselectItems(WI_templates_heirarchy);
	} else {
		if (CurrentlyEditing()->in_app)
			XukcScrListSelectItem(WI_instance_list,
				CurrentlyEditing()->mark_num, FALSE, TRUE);
		else
			XukcScrListSelectItem(WI_templates_heirarchy,
				CurrentlyEditing()->mark_num, FALSE, TRUE);
	}	
}


void
ChangeMark(new_object)
ObjectPtr new_object;
{
	XukcScrListUnselectItems(WI_instance_list);
	XukcScrListUnselectItems(WI_templates_heirarchy);

	if (new_object != NULL)
		XukcScrListSelectItem(new_object->in_app ?
				WI_instance_list : WI_templates_heirarchy,
				new_object->mark_num, FALSE, TRUE);
	picked_object = new_object;
}


/* ARGSUSED */
void
PickAnInstance(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XawListReturnStruct *picked_string = (XawListReturnStruct *)call_data;
	ObjectPtr start;
	Boolean in_app = (w == WI_instance_list);

	picked_object = SearchRecordsForMarkNum(GetStartOfList(in_app),
				(Cardinal)picked_string->list_index + 1);
	if (picked_object == NULL)
		XtError("unable to find picked widget()");
	ChangeSelection(picked_object);
}
