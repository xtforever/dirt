#ifndef lint
static char sccsid[] = "@(#)list.c	1.3 (UKC) 5/17/92";
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
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/CompositeP.h>
#include <X11/ShellP.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

extern void CreateResourceTable();
extern void BindCallback();
extern void DeleteResourceTable();
extern String XukcClassName();
extern void XukcChangeLabel();

extern WidgetDefsRec widget_classes[];
extern String widget_class_variables[];
extern Cardinal num_on_widget_class_list;
extern Widget WI_class_list;
extern Widget WI_new_class_entry;

static Cardinal class_chosen = 0;
static Cardinal wcl = 0;


static int
compare_names(a, b)
String a, b;
{
	return (strcmp(*(String *)a, *(String *)b));
}


/**** Module Exported Routines ****/


void
CreateClassList()
{
	String *list_string;
	Cardinal i, k;

	if (WI_class_list != NULL)
		XukcDestroyList(&wcl, TRUE);

	for (i = 0, k = 0; i < num_on_widget_class_list; i++)
		if (strlen(CLASS_DISPLAYED(i)) > k)
			k = strlen(CLASS_DISPLAYED(i));
		
	for (i = 0; i < num_on_widget_class_list; i++) {
		char big_buf[1000];
		int l, j;

		/* show as "Class Name [padding] : description" */
		(void)strcpy(big_buf, CLASS_DISPLAYED(i));
		j = strlen(CLASS_DISPLAYED(i));
		for (l = k-j; l >= 0; l--)
			big_buf[l+j] = ' ';
		big_buf[k+1] = ':';
		big_buf[k+2] = ' ';
		big_buf[k+3] = '\0';
		(void)strcat(big_buf, CLASS_DESC(i));

		XukcAddObjectToList(&wcl, XtNewString(big_buf), TRUE);
	}

	list_string = (String *)XukcGetListPointer(wcl, &k);

	/* sort the class names */
	qsort(list_string, k, sizeof(String), compare_names);

	/* update a list widget to hold the available names for
	   the user to choose from. */
	XukcScrListChange(WI_class_list, list_string, i);
}


String
GetClassName(i)
Cardinal i;
{
	if (i < 1 || i > num_on_widget_class_list)
		return (NULL);	/* bad widget class number */

	return (CLASS_ENTRY(i-1)->core_class.class_name);
}


String
GetDisplayedClassName(i)
Cardinal i;
{
	if (i < 1 || i > num_on_widget_class_list)
		return (NULL);	/* bad widget class number */

	return (CLASS_DISPLAYED(i-1));
}


WidgetClass
GetClass(i)
Cardinal i;
{
	if (i < 1 || i > num_on_widget_class_list)
		return (NULL);	/* bad widget class number */

	return (CLASS_ENTRY(i-1));
}


Cardinal
FindClass(class)
WidgetClass class;
{
	Cardinal i;

	for (i = 0; i < num_on_widget_class_list; i++)
		if (CLASS_ENTRY(i) == class)
			return (i+1);
	return (0);
}


String
GetClassVariableName(i)
Cardinal i;
{
	if (i < 1 || i > num_on_widget_class_list)
		return (NULL);	/* bad widget class number */

	return (CLASS_NAME(i-1));
}


void
ResetClassChosen()
{
	class_chosen = 0;
	XukcScrListUnselectItems(WI_class_list);
	DeleteResourceTable(NEW_WIDGET_RESOURCES);
}


Cardinal
GetClassChosen()
{
	return (class_chosen);
}


void
SetClassChosen(i)
Cardinal i;
{
	String *list_string;
	Cardinal k, len;
	Cardinal j;

	if (wcl == 0)
		return;
	
	len = strlen(CLASS_DISPLAYED(i));
	list_string = (String *)XukcGetListPointer(wcl, &k);
	for (j = 0; j < k; j++)
		if (strncmp(list_string[j], CLASS_DISPLAYED(i), len) == 0)
			break;
	if (j == k)
		return;
	class_chosen = i;
	(void)XukcScrListSelectItem(WI_class_list, j, FALSE, TRUE);
}


Cardinal
GetClassFromName(name)
String name;
{
	Cardinal i;

	for (i = 0; i < num_on_widget_class_list; i++)
		if (strcmp(CLASS_NAME(i), name) == 0)
			return (i+1);
	return (0);
}


Cardinal
GetClassFromClassName(name)
String name;
{
	Cardinal i;

	for (i = 0; i < num_on_widget_class_list; i++)
		if (strcmp(XukcClassName(*(widget_classes[i].widget_class)),
		    (char *)name) == 0)
			return (i + 1);
	return (0);
}


Cardinal
GetSuperClass(class)
Cardinal class;
{
	WidgetClass wc;

	if (class == 0)
		return 0;

	wc = GetClass(class);
	/* ||| KLUDGE: to skip past the Xt UnNamedObjClass .. which we
	 *     haven't got access.
	 * |||
	 */
	if (wc->core_class.superclass != NULL &&
	    wc->core_class.superclass->core_class.superclass == rectObjClass)
		wc = wc->core_class.superclass;
	return (FindClass(wc->core_class.superclass));
}

	
Boolean
IsSubclass(class, superclass)
WidgetClass class, superclass;
{
	WidgetClass wc = class;

	while (wc != NULL & wc != superclass)
		wc = wc->core_class.superclass;
	return (wc != NULL);
}


Boolean
IsShellClass(class)
WidgetClass class;
{
	return (IsSubclass(class, shellWidgetClass));
}


Boolean
IsCompositeClass(class)
WidgetClass class;
{
	return (IsSubclass(class, compositeWidgetClass));
}


Boolean
AcceptsObjects(class, object_class)
WidgetClass class;
ObjectClass object_class;
{
	CompositeClassExtensionRec *class_rec;
	CompositeWidgetClass wc = (CompositeWidgetClass)class;
	Boolean acceptable;


	if (!IsCompositeClass(class))
		return (FALSE);
	class_rec =
		(CompositeClassExtensionRec *)wc->composite_class.extension;

	acceptable = class_rec != NULL ? class_rec->accepts_objects : FALSE;
	if (acceptable && object_class != NULL) {
		Cardinal cl, i;
		/* check that this class accepts certain object classes */
		cl = FindClass(class);
		if (CLASS_NUM_ACCEPTABLE(cl-1) != 0) {
			acceptable = FALSE;

			for (i = 0; !acceptable &&
			     i < CLASS_NUM_ACCEPTABLE(cl-1); i++)
				if (CLASS_ACCEPTABLE_OBJECTS(cl-1)[i]
				    == object_class)
					acceptable = FALSE;
		}
	}
	return (acceptable);
}


Boolean
IgnoreClassResource(class, resource_name, resource_class, resource_type, flags)
Cardinal class;
String resource_name, resource_class, resource_type;
Cardinal flags;
{
	Cardinal i;
	ResDefsPtr ignore_list;

	if (class == 0)
		return FALSE;

	i = CLASS_NUM_NONEDIT(class-1);
	ignore_list = CLASS_NONEDIT_RES(class-1);

	for (; i > 0; i--)
		if ((ignore_list[i-1].flags & flags)
		    && strcmp(resource_name, ignore_list[i-1].name) == 0
		    && strcmp(resource_class, ignore_list[i-1].class) == 0
		    && strcmp(resource_type, ignore_list[i-1].type) == 0)
			return TRUE;
	return (IgnoreClassResource(GetSuperClass(class), resource_name,
		resource_class, resource_type, flags));
}


Boolean
IsReadOnly(object, entry)
ObjectPtr object;
XtResourceList entry;
{
	Cardinal i;
	ResDefsPtr ignore_list;
	Cardinal class;

	if (entry->resource_class == NULL)
		return FALSE;

	if (strcmp(entry->resource_class, XtCReadOnly) == 0)
		return TRUE;

	class = object->class;

	do {
		i = CLASS_NUM_NONEDIT(class-1);
		ignore_list = CLASS_NONEDIT_RES(class-1);

		for (; i > 0; i--)
		  if ((ignore_list[i-1].flags & READ_ONLY)
		    && strcmp(entry->resource_name, ignore_list[i-1].name) == 0
		    && strcmp(entry->resource_class, ignore_list[i-1].class) == 0
		    && strcmp(entry->resource_type, ignore_list[i-1].type) == 0)
			return TRUE;
	} while ((class = GetSuperClass(class)) != 0);
	return FALSE;
}


void
InitializeWidgetClasses()
{
	Cardinal i;

	for (i = 0; i < num_on_widget_class_list; i++)
		XtInitializeWidgetClass(CLASS_ENTRY(i));
}


/* ARGSUSED */
void
PickAClass(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XawListReturnStruct *picked_string = (XawListReturnStruct *)call_data;
	char *tmp, *colon_pos;
	Cardinal i;

	colon_pos = tmp = index(picked_string->string, ':');
	if (tmp != NULL) {
		/* strip off trailing spaces */
		while (*(tmp-1) == ' ') tmp--;
		*tmp = '\0';
	}

	for (i = 0; i < num_on_widget_class_list; i++)
		if (strcmp(picked_string->string, CLASS_DISPLAYED(i)) == 0)
			break;
	if (i < num_on_widget_class_list) {
		class_chosen = i+1;
		CreateResourceTable(NEW_WIDGET_RESOURCES,
						(ObjectPtr)NULL, class_chosen,
						FALSE);
		XukcChangeLabel(WI_new_class_entry, picked_string->string,
						FALSE);
	}

	if (tmp != NULL)
		*tmp = tmp != colon_pos ? ' ' : ':';
}
