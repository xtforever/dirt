#ifndef lint
static char sccsid[] = "@(#)rdb_parse.c	1.3 (UKC) 5/17/92";
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

#include <ctype.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/ObjectP.h>
#include <X11/Xukc/memory.h>
#if 0
#include "dirt_structures.h"
#endif

/* externally defined routines */
/* in create.c */
extern String GetAppClassName();

#define LOOSE_BINDING '*'
#define TIGHT_BINDING '.'

#define BIG_STR_SIZE 2048
#define FIND_NEXT_NAME(put_var, name) { \
	char *t1; \
	if ((t1 = put_var = index(name, TIGHT_BINDING)) == NULL) \
		put_var = index(name, LOOSE_BINDING); \
	else if ((put_var = index(name, LOOSE_BINDING)) == NULL || put_var > t1) \
		put_var = t1; \
}
#define FIND_PREVIOUS_NAME(put_var, name) { \
	char *t1; \
	if ((t1 = put_var = rindex(name, TIGHT_BINDING)) == NULL) \
		put_var = rindex(name, LOOSE_BINDING); \
	else if ((put_var = rindex(name, LOOSE_BINDING)) == NULL || put_var < t1) \
		put_var = t1; \
}
#define WIDGET_NAME_QUARK(w) (w->core.xrm_name)
#define WIDGET_CLASS_QUARK(w) (w->core.widget_class->core_class.xrm_class)

static Cardinal name_list, binding_list, class_list;
static Cardinal max_depth;


/* ARGSUSED */
static Boolean
check_resource(w, loose_binding, name, value)
Widget w;
Boolean loose_binding;
String name, value;
{
	XtResourceList resl, cresl;
	Cardinal num_resl, num_cresl;
	Boolean is_class = isupper(*name);

	/* we don't really need to worry about the loose_binding ?? */

#ifdef DEBUG
	printf("MATCHED: %s%c%s: %s\n", XtName(w), loose_binding ?
						'*' : '.', name, value);
#endif /* DEBUG */
	XtGetResourceList(XtClass(w), &resl, &num_resl);
	for (; num_resl > 0; num_resl--)
		if (strcmp(is_class ? resl[num_resl-1].resource_class :
				resl[num_resl-1].resource_name, name) == 0)
			break;
	XtFree((char *)resl);

	if (num_resl == 0 && XtParent(w) != NULL) {
		/* check any constraint resources */
		XtGetConstraintResourceList(XtClass(XtParent(w)), &cresl,
								&num_cresl);
		for (; num_cresl > 0; num_cresl--)
			if (strcmp(is_class ? cresl[num_cresl-1].resource_class
				: cresl[num_cresl-1].resource_name, name) == 0)
			break;
		XtFree((char *)cresl);

		if (num_cresl == 0)
			return (FALSE);
	}
	/* SetValues could be called here to apply the resource setting.
	 * if is_class == TRUE then we need to look through both the
	 * standard and contraint resource lists and do a SetValues on each
	 * resource whose class matches.
	 *
	 * For complete checking of the resource setting we should use
	 * the XtConvertAndStore() mechanism to convert the string to the
	 * appropriate type (again found in the resource lists) .. if this
	 * call returns FALSE then we output an error.
	 */
	return (TRUE);
}


static Boolean
check_parent(parent, has_loose_binding, depth)
Widget parent;
Boolean has_loose_binding;
Cardinal depth;
{
	Boolean name_is_class = FALSE;
	Boolean loose_binding = FALSE;
	Boolean this_level_matched = FALSE;
	XrmQuark name_quark;

	if (name_list == 0)
		return (TRUE);

	if ((name_quark = (XrmQuark)XukcGetObjectOnList(name_list, depth))
	    == NULL)
		return (TRUE);

	loose_binding = (Boolean)XukcGetObjectOnList(binding_list, depth);
	name_is_class = (Boolean)XukcGetObjectOnList(class_list, depth);

	this_level_matched = (name_is_class ? WIDGET_CLASS_QUARK(parent) :
			     WIDGET_NAME_QUARK(parent)) == name_quark;

	if (this_level_matched && depth == max_depth) {
		if (loose_binding)
			return (TRUE);
		else if (XtParent(parent) == NULL)
			return (TRUE);
	}

	if (this_level_matched || has_loose_binding) {
		/* check all parents .. if this level's binding is LOOSE
		 * then we must check ALL parents for possible matches
		 */
		if (loose_binding) {
			Boolean loose_result = FALSE;

			parent = XtParent(parent);
			while (parent != NULL && !loose_result) {
				loose_result = check_parent(parent,
						    TRUE,
						    depth + this_level_matched);
				parent = XtParent(parent);
			}
			return (loose_result);
		} else return (check_parent(XtParent(parent), FALSE,
					      depth + this_level_matched));
	}
	return (FALSE);
}


static Boolean
apply_resource_setting(start, resource_name, value, loose_resource_binding)
Widget start;
String resource_name, value;
Boolean loose_resource_binding;
{
	Cardinal i;
	Boolean found = FALSE;

	if (start == NULL)
		return (found);

	/* max_depth == 0 is the special case where the resource applies
	 * to all widget instances */
	if (max_depth == 0 ||
	    check_parent(start, loose_resource_binding, 1))
		found |= check_resource(start, loose_resource_binding,
				resource_name, value);

	if (XtIsComposite(start)) {
		/* look through all the children */
		CompositeWidget parent = (CompositeWidget)start;
		WidgetList widgets = parent->composite.children;

		for (i = 0; i < parent->composite.num_children; i++)
			found |= apply_resource_setting(widgets[i],
					resource_name, value,
					loose_resource_binding);
	}
	/* check all the popup shells */
	for (i = 0; i < start->core.num_popups; i++)
		found |= apply_resource_setting(start->core.popup_list[i],
					resource_name, value,
					loose_resource_binding);
	return (found);
}


/*** Module Exported Routines ***/


Boolean
LogResourceDatabaseSetting(start, full_resource_name, value)
Widget start;
String full_resource_name, value;
{
	String class_name, resource_name, full_name;
	Boolean loose_name_binding, loose_resource_binding;
	Boolean found;

	max_depth = name_list = binding_list = class_list = 0;
	full_name = XtNewString(full_resource_name);

	if (isupper(*full_name)) {
		FIND_NEXT_NAME(class_name, full_name);
		loose_name_binding = (*class_name == LOOSE_BINDING);
		*class_name = '\0';
		if (strcmp((char *)full_name, (char *)GetAppClassName()) != 0) {
			XtFree(full_name);
			/* wrong class of application .. error? */
			return (FALSE);
		}
		class_name[-1] = '\0';
		class_name[0] = loose_name_binding ? LOOSE_BINDING
						   : TIGHT_BINDING;
		full_name = class_name;
	}

	FIND_PREVIOUS_NAME(resource_name, full_name);
	if (resource_name != NULL) {
		loose_resource_binding = (*resource_name == LOOSE_BINDING);
		*resource_name = '\0';
		resource_name++;
	} else {
		loose_resource_binding = FALSE;
		resource_name = full_name;
	}

	/* build up a list of the name components, their types and their
	 * binding tightness's.
	 */
	while (*full_name) {
		String name;
		Boolean is_class;

		FIND_PREVIOUS_NAME(name, full_name);
		if (name != NULL) {
			loose_name_binding = (*name == LOOSE_BINDING);
			*name = '\0';
			name++;
		} else {
			loose_name_binding = FALSE;
			name = full_name;
			full_name += strlen(full_name);
		}
		is_class = isupper(*name);

		(void)XukcAddObjectToList(&name_list,
			(XtPointer)XrmStringToQuark(name), FALSE);
		(void)XukcAddObjectToList(&binding_list,
					(XtPointer)loose_name_binding, FALSE);
		(void)XukcAddObjectToList(&class_list, (XtPointer)is_class,
								FALSE);
	}

	if (name_list > 0)
		(void)XukcGetListPointer(name_list, &max_depth);

	found = apply_resource_setting(start, resource_name, value,
						loose_resource_binding);
	(void)XukcDestroyList(&name_list, FALSE);
	(void)XukcDestroyList(&binding_list, FALSE);
	(void)XukcDestroyList(&class_list, FALSE);
	XtFree(full_name);
	return (found);
}
