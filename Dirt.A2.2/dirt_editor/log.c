#ifndef lint
static char sccsid[] = "@(#)log.c	1.2 (UKC) 5/10/92";
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
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

#define BIG_STR_SIZE 2048
#define FIND_PREVIOUS_NAME(put_var, name) { \
	char *t1; \
	if ((t1 = put_var = rindex(name, '.')) == NULL) \
		put_var = rindex(name, '*'); \
	else if ((put_var = rindex(name, '*')) == NULL || put_var < t1) \
		put_var = t1; \
}
#define WIDGET_NAME_QUARK(w) (w->core.xrm_name)
#define WIDGET_CLASS_QUARK(w) (w->core.widget_class->core_class.xrm_class)

#if 0
static char out_line[1000];
static char widget_name[500];
static int name_len;
static char *ptr;
static XrmDatabase log_database = NULL;
static char *errm[1];
#endif

extern ObjectPtr GetStartOfList();
extern String XukcGrowString();
extern ObjectPtr GetHeadObject();
extern ObjectPtr SearchRecords();
extern Boolean IsShellClass();
extern WidgetClass GetClass();

static Boolean
find_unique(start, child)
ObjectPtr start, child;
{
	ObjectPtr current;
	Boolean is_unique = TRUE;
	String name = child->name;

	if (start == NULL)
		current = GetStartOfList(child->in_app)->next;
	else
		current = start->next;

	for (; current != NULL && is_unique; current = current->next) {
		if (current == child)
			continue;

		is_unique = strcmp(current->name, name) != 0;
	}
	return (is_unique);
}


static void
build_instance_name(child, start, current_name)
ObjectPtr child;
String *start, *current_name;
{
	Cardinal size;

	if (child == NULL)
		return;

	if (!NO_PARENT(child) && !find_unique((ObjectPtr)NULL, child))
		build_instance_name(child->parent, start, current_name);

	size = *current_name - *start;
	*start = XukcGrowString(*start, size + strlen(child->name) + 2);
	*current_name = *start + size;
	(void)sprintf(*current_name, "%s.", child->name);
	*current_name += strlen(child->name) + 1;
}


#if 0 /* ||| */
static void
apply_resource(child, name, is_class, value, apply_to_all)
ObjectPtr child;
String name;
Boolean is_class;
String value;
Boolean apply_to_all;
{
	XtResourceList res;
	int i;

printf("apply_resource(%s, %s, %d, %s, %d)\n", child, name, is_class, value,
apply_to_all);
#if 0
	res = get_a_resource(child->instance, name, is_class, &i);

	if (i >= 0) {
		if (is_class) {
			change_resource(child, &res[i], value, TRUE, FALSE);
			for (i--; i >= 0; i--) {
				if (strcmp(res[i].resource_class, name) == 0)
					change_resource(child, &res[i],
							value, TRUE, FALSE);
			}
		}
		else
			change_resource(child, &res[i], value, FALSE, FALSE);
	}
	XtFree(res);
#endif 0
}


static void
apply_default(child, wid_name, name, is_class, value, apply_to_all)
ObjectPtr child;
String wid_name, name;
Boolean is_class;
String value;
Boolean apply_to_all;
{
	String temp;
	Boolean apply_name_to_all = FALSE;
	Boolean apply_now = FALSE;
	Boolean do_apply;
	Boolean name_is_class;
	char temp_c;
	XrmQuark name_quark;
	char copy[200];

	FIND_PREVIOUS_NAME(temp,wid_name)
	if (temp != NULL) {
		apply_name_to_all = (*temp == '*');
		*temp = '\0';
		temp++;
		name_is_class = isupper(temp[0]);
		name_quark = name_is_class ? XrmStringToClass(temp)
					   : XrmStringToName(temp);
	}

	for (; child != NULL; child = child->next) {
		if (*temp == '\0')
			apply_resource(child, name, is_class, value,
								apply_to_all);
		else {
			(void)strcpy(copy, wid_name);

			if ((name_is_class ? WIDGET_CLASS_QUARK(child->instance) :
			    WIDGET_NAME_QUARK(child->instance)) == name_quark) {
				if (check_parent(child->parent, apply_name_to_all, copy))
					apply_resource(child, name, is_class,
							value, apply_to_all);
			}
		}
	}
}

save_log(name)
String name;
{
	if (log_database != NULL)
		XrmPutFileDatabase(log_database, name);
}


load_log(name)
String name;
{
	FILE *new_base;
	char temp[BIG_STR_SIZE];
	ObjectPtr child;
	Boolean apply_to_all;
	XtResourceList res_entry;
	String res_name;

	if ((new_base = fopen(name, "r")) == NULL) {
		errm[0] = name;
		XtWarningMsg("invalidFilename", "loadLog",
				"DirtWarning",
				"DIRT! : Cannot open \"%s\" database file",
				errm, 1);
	} else {
		while (fgets(temp, BIG_STR_SIZE, new_base) != NULL) {
			widget_name[0] = '\0';

			(void)sscanf(temp, "%[^:]*", widget_name);

			/* ignore ':' and intervening whitespace */
			res_name = temp + strlen(widget_name) + 1;
			while (res_name != NULL
				&& (*res_name == ' ' || *res_name == '\t'))
					res_name++;

			(void)sscanf(res_name, "%s", out_line);

			if (widget_name[0] == '\0') {
				errm[0] = name;
				XtErrorMsg("corruptDatabase", "loadLog",
				"DirtError",
				"DIRT! : \"%s\" database file is corrupt",
				errm, 1);
			}

			FIND_PREVIOUS_NAME(res_name,widget_name)

			if (res_name == NULL) {
				errm[0] = name;
				XtWarningMsg("corruptResource", "loadLog",
				"DirtError",
				"DIRT! : missing widget name in \"%s\" resource file",
				errm, 1);
			} else {
				apply_to_all = (*res_name == '*');
				*res_name = '\0';
				res_name++;
			}


			apply_default(widget_name, res_name, isupper(*res_name),
					out_line, apply_to_all);
			XrmPutLineResource(&log_database, temp);
		}
		(void)fclose(new_base);
	}
}
#endif /* ||| */


Boolean
log_resource(name_list, value_list, resource, value, add_if_not_found)
Cardinal *name_list, *value_list;
String resource;
String value;
Boolean add_if_not_found;
{
	String tmp;
	Boolean changed = FALSE;

	if (*name_list == 0) {
		if (add_if_not_found) {
			(void)XukcAddObjectToList(name_list,
					(XtPointer)XtNewString(resource), FALSE);
			(void)XukcAddObjectToList(value_list,
					(XtPointer)XtNewString(value), FALSE);
		}
	} else {
		Cardinal i = 1;
		Boolean found = FALSE;

		tmp = (String)XukcGetObjectOnList(*name_list, i);
		while (tmp != NULL && !found) {
			if (strcmp(tmp, resource) == 0)
				found = TRUE;
			else {
				i++;
				tmp = (String)XukcGetObjectOnList(*name_list, i);
			}
		}
		
		if (found) {
			XtFree((char *)XukcGetObjectOnList(*value_list, i));
			(void)XukcChangeObjectOnList(*value_list,
						i, (XtPointer)XtNewString(value));
			changed = TRUE;
		} else if (add_if_not_found) {
			(void)XukcAddObjectToList(name_list,
					(XtPointer)XtNewString(resource), FALSE);
			(void)XukcAddObjectToList(value_list,
					(XtPointer)XtNewString(value), FALSE);
		}
	}
	return (changed);
}


Boolean
remove_resource(name_list, value_list, resource)
Cardinal *name_list, *value_list;
String resource;
{
	String tmp;
	Boolean removed = FALSE;

	if (*name_list > 0) {
		Cardinal i = 1;
		Boolean found = FALSE;

		tmp = (String)XukcGetObjectOnList(*name_list, i);
		while (tmp != NULL && !found) {
			if (strcmp(tmp, resource) == 0)
				found = TRUE;
			else {
				i++;
				tmp = (String)XukcGetObjectOnList(*name_list, i);
			}
		}
		
		if (found) {
			XtFree(tmp);
			XtFree((char *)XukcGetObjectOnList(*value_list, i));
			XukcRemoveEntryOnList(value_list, i);
			XukcRemoveEntryOnList(name_list, i);
			removed = TRUE;
		}
	}
	return (removed);
}


/*** Module Exported Routines ***/


void
LogResourceSetting(cl, resource, value, is_constraint)
ObjectPtr cl;
String resource;
String value;
Boolean is_constraint;
{
	if (is_constraint)
		(void)log_resource(&(cl->constraint_name_list),
				    &(cl->constraint_value_list),
					    resource, value, TRUE);
	else { if (cl->instance != NULL) {
		if (!log_resource(&(cl->preset_name_list),
		     &(cl->preset_value_list), resource, value, FALSE))
			(void)log_resource(&(cl->resource_name_list),
					    &(cl->resource_value_list),
					    resource, value, TRUE);
	} else {
		(void)log_resource(&(cl->preset_name_list),
			    &(cl->preset_value_list), resource, value, TRUE);
	} }
}


void
RemoveResourceSetting(cl, resource, is_constraint)
ObjectPtr cl;
String resource;
Boolean is_constraint;
{
	if (is_constraint)
		(void)remove_resource(&(cl->constraint_name_list),
				    &(cl->constraint_value_list), resource);
	else { if (cl->instance != NULL) {
		if (!remove_resource(&(cl->preset_name_list),
		     &(cl->preset_value_list), resource))
			(void)remove_resource(&(cl->resource_name_list),
					    &(cl->resource_value_list),
					    resource);
	} else {
		(void)remove_resource(&(cl->preset_name_list),
			    &(cl->preset_value_list), resource);
	} }
}


/* builds a resource database specification name that uniquely identifies this
 * object.
 */
String
UniqueObjectName(object)
ObjectPtr object;
{
	String s, f;

	s = f = NULL;
	build_instance_name(object, &s, &f);
	return (s);
}


String
UniqueWidgetName(w)
Widget w;
{
	ObjectPtr obj;
	String str = NULL, tmp = NULL, current = NULL;
	Cardinal len = 0, new_len = 0, i = 0;

	obj = SearchRecords(GetHeadObject(), w);

	while (w != NULL && obj == NULL) {
		new_len = strlen(XtName(w)) + 1;
		str = XukcGrowString(str, len + new_len + 1);
		for (i = len; i > 0; i--)
			str[i + new_len - 1] = str[i - 1];
		memcpy(str + 1, XtName(w), new_len - 1);
		*str = '.';
		len += new_len;
		*(str + len) = '\0';
		w = XtParent(w);
		if (w != NULL)
			obj = SearchRecords(GetHeadObject(), w);
	}
	if (obj != NULL)
		build_instance_name(obj, &tmp, &current);
	if (tmp != NULL) {
		new_len = strlen(tmp) - 1;
		str = XukcGrowString(str, new_len + len + 2);
		for (i = len; i > 0; i--)
			str[i + new_len] = str[i - 1];
		memcpy(str + 1, tmp, new_len);
		XtFree(tmp);
		len += new_len;
		*(str + len + 1) = '\0';
	}
	*str = '*';
	return (str);
}
