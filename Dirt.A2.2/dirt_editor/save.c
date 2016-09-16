#ifndef lint
static char sccsid[] = "@(#)save.c	1.2 (UKC) 5/10/92";
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

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include "dirt_structures.h"

extern struct app_resources editor_resources;

extern String GetClassVariableName();
extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern String GetAppClassName();
extern String XukcGrowString();
extern String UniqueObjectName();
extern void SaveConnectionList();
extern void SaveRegisteredNames();
extern ObjectPtr CurrentlyEditing();
extern void EditComments();
extern String XukcCreateMeasurementString();
extern XtPointer *XukcGetListPointer();
extern XtPointer XukcGetObjectOnList();
extern void XukcRemoveEntryOnList();

String CvtFromNewlines();
String GetComplexName();

#ifndef CREATION_DATE
#	define CREATION_DATE "July 1990"
#endif !CREATION_DATE
#ifndef VERSION
#	define VERSION "1.4"
#endif !VERSION
#ifndef UKC_INCLUDES
#	define UKC_INCLUDES "X11/Xukc"
#endif !UKC_INCLUDES


static void
strip_lines(object, line)
ObjectPtr object;
String line;
{
	String *rdb;
	Cardinal on_list;
	Boolean did_removal;

	if (object->sub_resources_list == 0)
		return;

	do {
		rdb = (String *)XukcGetListPointer(object->sub_resources_list,
							&on_list);
		did_removal = FALSE;
		for (; on_list > 0; on_list--)
			if (strcmp(rdb[on_list-1], line) == 0) {
				XukcRemoveEntryOnList(
					&object->sub_resources_list,
					on_list);
				did_removal = TRUE;
				break;
			}
	} while (did_removal && object->sub_resources_list != 0);
}


static void
strip_matching_lines(sub_object)
ObjectPtr sub_object;
{
	char big_str[10000]; /* ||| this should be dynamic ||| */

	if (sub_object == NULL)
		return;

	if (sub_object->is_resource) {
		if (sub_object->resource_name_list > 0) {
			Cardinal i = 1;
			String tmp1, tmp2, tmp3, tmp4;

			while ((tmp1 = (String)XukcGetObjectOnList(sub_object->resource_name_list, i)) != NULL) {
				tmp2 = UniqueObjectName(sub_object->parent);
				tmp3 = (String)XukcGetObjectOnList(sub_object->resource_value_list, i);
				sprintf(big_str, "*%s%s.%s: ", tmp2,
						sub_object->name, tmp1);
				tmp4 = CvtFromNewlines(tmp3, TRUE);
				strcat(big_str, tmp4);
				strip_lines(sub_object->parent, big_str);
				XtFree(tmp4);
				XtFree(tmp2);
				i++;
			}
		}
		if (sub_object->constraint_name_list > 0) {
			Cardinal i = 1;
			String tmp1, tmp2, tmp3, tmp4;

			while ((tmp1 = (String)XukcGetObjectOnList(sub_object->constraint_name_list, i)) != NULL) {
				tmp2 = UniqueObjectName(sub_object->parent);
				tmp3 = (String)XukcGetObjectOnList(sub_object->constraint_value_list, i);
				sprintf(big_str, "*%s%s.%s: ", tmp2,
						sub_object->name, tmp1);
				tmp4 = CvtFromNewlines(tmp3, TRUE);
				strcat(big_str, tmp4);
				strip_lines(sub_object->parent, big_str);
				XtFree(tmp4);
				XtFree(tmp2);
				i++;
			}
		}
	}
	strip_matching_lines(sub_object->next);
}


static void
print_sub_resources(fd, sub_object, indent)
FILE *fd;
ObjectPtr sub_object;
String indent;
{
	if (sub_object == NULL)
		return;
	if (sub_object->is_resource) {
		if (sub_object->resource_name_list > 0) {
			Cardinal i = 1;
			String tmp1, tmp2, tmp3, tmp4;

			while ((tmp1 = (String)XukcGetObjectOnList(sub_object->resource_name_list, i)) != NULL) {
				tmp2 = UniqueObjectName(sub_object->parent);
				tmp3 = (String)XukcGetObjectOnList(sub_object->resource_value_list, i);
				fprintf(fd, "%%d %s*%s%s.%s: ", indent, tmp2,
							sub_object->name, tmp1);
				tmp4 = CvtFromNewlines(tmp3, TRUE);
				fprintf(fd, "%s\n", tmp4);
				XtFree(tmp4);
				XtFree(tmp2);
				i++;
			}
		}
		if (sub_object->constraint_name_list > 0) {
			Cardinal i = 1;
			String tmp1, tmp2, tmp3, tmp4;

			while ((tmp1 = (String)XukcGetObjectOnList(sub_object->constraint_name_list, i)) != NULL) {
				tmp2 = UniqueObjectName(sub_object->parent);
				tmp3 = (String)XukcGetObjectOnList(sub_object->constraint_value_list, i);
				fprintf(fd, "%%d %s*%s%s.%s: ", indent, tmp2,
							sub_object->name, tmp1);
				tmp4 = CvtFromNewlines(tmp3, TRUE);
				fprintf(fd, "%s\n", tmp4);
				XtFree(tmp4);
				XtFree(tmp2);
				i++;
			}
		}
	}
	print_sub_resources(fd, sub_object->next, indent);
}


static void
print_help(fd, object, indent, prefix)
FILE *fd;
ObjectPtr object;
String indent;
char prefix;
{
	String tmp;

	if (object->help == NULL || *(object->help) == '\0')
		return;

	tmp = CvtFromNewlines(object->help, TRUE);
	fprintf(fd, "%%%c %s%s\n", prefix, indent, tmp);
	XtFree(tmp);
}


static void
print_comments(fd, object)
FILE *fd;
ObjectPtr object;
{
	String this_comment, next_comment;

	if (object->comments == NULL || *(object->comments) == '\0')
		return;

	this_comment = object->comments;
	next_comment = index(this_comment, '\n');
	while (next_comment != NULL) {
		*next_comment = '\0';
		fprintf(fd, "%%%% %s\n", this_comment);
		*next_comment = '\n';
		this_comment = next_comment + 1;
		next_comment = index(this_comment, '\n');
	}
	if (*this_comment != '\0')
		fprintf(fd, "%%%% %s\n", this_comment);
}


static void
print_database_lines(fd, object, indent)
FILE *fd;
ObjectPtr object;
String indent;
{
	String *rdb;
	Cardinal on_list;

	if (object->sub_resources_list == 0)
		return;

	rdb = (String *)XukcGetListPointer(object->sub_resources_list, &on_list);

	for (; on_list > 0; on_list--)
		fprintf(fd, "%%d %s%s\n", indent, rdb[on_list-1]);
}


static void
print_resources(fd, name_list, value_list, prefix, indent)
FILE *fd;
Cardinal name_list, value_list;
String prefix, indent;
{
	Cardinal i = 1;
	String tmp1, tmp2, tmp3;

	if (name_list == 0)
		return;

	while ((tmp1 = (String)XukcGetObjectOnList(name_list, i)) != NULL) {
		tmp2 = (String)XukcGetObjectOnList(value_list, i);
		fprintf(fd, "%s %s%s : ", prefix, indent, tmp1);
		tmp3 = CvtFromNewlines(tmp2, TRUE);
		fprintf(fd, "%s\n", tmp3);
		XtFree(tmp3);
		i++;
	}
}


static void
save_application_details(fd, app)
FILE *fd;
ObjectPtr app;
{
#define MEASUREMENT(dim) (editor_resources.pixels ? \
	((void)sprintf(tmp, "%d", dim), (String)tmp) : \
	XukcCreateMeasurementString(XtScreen(app->instance), \
	(int)dim, !editor_resources.inches))
	char tmp[30];

	fprintf(fd, "%%%%%%\n");
	print_comments(fd, app);
	fprintf(fd, "%%A %s\n", app->name);
	fprintf(fd, "%%C %s\n", GetAppClassName());
	print_help(fd, app, "", 'H');
	/* print out any locked geometry resources */
	if (app->resource_flags & X_RES_LOCKED)
		fprintf(fd, "%%R x : %s\n", MEASUREMENT(app->x));
	if (app->resource_flags & Y_RES_LOCKED)
		fprintf(fd, "%%R y : %s\n", MEASUREMENT(app->y));
	if (app->resource_flags & WIDTH_RES_LOCKED)
		fprintf(fd, "%%R width : %s\n", MEASUREMENT(app->width));
	if (app->resource_flags & HEIGHT_RES_LOCKED)
		fprintf(fd, "%%R height : %s\n", MEASUREMENT(app->height));
	print_resources(fd, app->preset_name_list,
			    app->preset_value_list, "%R", "");
	print_resources(fd, app->resource_name_list,
			    app->resource_value_list, "%R", "");
	print_resources(fd, app->constraint_name_list,
			    app->constraint_value_list, "%R", "");
	fprintf(fd, "%%F\n");
#undef MEASUREMENT
}


static void
save_template_details(fd, app)
FILE *fd;
ObjectPtr app;
{
	fprintf(fd, "%%%%%%\n");
	print_comments(fd, app);
	fprintf(fd, "%%T %s\n", app->name);
	print_help(fd, app, "", 'H');
	fprintf(fd, "%%F\n");
}


static void
save_widget_details(fd, object, level)
FILE *fd;
ObjectPtr object;
Cardinal level;
{
#define MEASUREMENT(dim) (editor_resources.pixels ? \
	((void)sprintf(tmp, "%d", dim), (String)tmp) : \
	XukcCreateMeasurementString(XtScreen(object->instance), \
	(int)dim, !editor_resources.inches))

	Cardinal i;
	char indent[100], tmp[30];
	String parent_name;

	if (object == NULL || object->is_resource)
		return;

	for (i = 0; i < level; i++)
		indent[i] = ' ';
	indent[i] = '\0';

	parent_name = GetComplexName(object->parent);
	fprintf(fd, "%%%%%%\n");
	print_comments(fd, object);
	fprintf(fd, "%%N %s%s\n", indent, object->name);
	fprintf(fd, "%%c %s%s\n", indent, GetClassVariableName(object->class));
	fprintf(fd, "%%p %s%s\n", indent, parent_name);
	XtFree(parent_name);
	fprintf(fd, "%%m %s%s\n", indent, object->managed ? "true" : "false");
	fprintf(fd, "%%o %s%s\n", indent,
				object->deferred_creation ? "true" : "false");
	print_help(fd, object, indent, 'h');
	print_sub_resources(fd, object->children, indent);
	print_database_lines(fd, object, indent);

	/* better safe then sorry */
	if (object->width == 0)
		object->width = object->parent->width;
	if (object->height == 0)
		object->height = object->parent->height;

	/* print out any locked geometry resources */
	if (object->resource_flags & X_RES_LOCKED)
		fprintf(fd, "%%s %sx : %s\n", indent, MEASUREMENT(object->x));
	if (object->resource_flags & Y_RES_LOCKED)
		fprintf(fd, "%%s %sy : %s\n", indent, MEASUREMENT(object->y));
	if (object->resource_flags & WIDTH_RES_LOCKED)
		fprintf(fd, "%%s %swidth : %s\n", indent,
						MEASUREMENT(object->width));
	if (object->resource_flags & HEIGHT_RES_LOCKED)
		fprintf(fd, "%%s %sheight : %s\n", indent,
						MEASUREMENT(object->height));

	print_resources(fd, object->preset_name_list,
			    object->preset_value_list, "%s", indent);
	print_resources(fd, object->constraint_name_list,
			    object->constraint_value_list, "%z", indent);
	print_resources(fd, object->resource_name_list,
			    object->resource_value_list, "%r", indent);
	fprintf(fd, "%%f\n");

	save_widget_details(fd, object->children, level+1);
	save_widget_details(fd, object->next, level);
#undef MEASUREMENT
}


static void
print_generated_message(fd, is_app)
FILE *fd;
Boolean is_app;
{
	struct passwd *pwe;
	long current_time;

	pwe = getpwuid(getuid());
	current_time = (long)time((long *)0);

	if (is_app) {
		fprintf(fd, "%%%%ID  DIRT! %s (c)1990 rlh2@ukc.ac.uk : UI Specification file\n", VERSION);
		fprintf(fd, "%%%%ID  Application = \"%s\"\n", GetStartOfList(TRUE)->name);
	} else {
		fprintf(fd, "%%%%ID  DIRT! %s (c)1990 rlh2@ukc.ac.uk : Template Specification file\n", VERSION);
		fprintf(fd, "%%%%ID  Template = \"%s\"\n", GetStartOfList(FALSE)->name);
	}
	if (pwe != NULL)
		fprintf(fd, "%%%%ID  Created by %s on %s",
			pwe->pw_name, ctime(&current_time));
}


/*** Module Exported Routines ***/

SaveUISpec(fd)
FILE *fd;
{
	ObjectPtr current;

	if (GetStartOfList(TRUE) == NULL) return;

	/* save any comments currently being edited */
	current = CurrentlyEditing();
	EditComments((ObjectPtr)NULL);
	EditComments(current);

	print_generated_message(fd, TRUE);
	SaveRegisteredNames(fd, "%Y", TRUE);
	SaveRegisteredNames(fd, "%Z", FALSE);
	save_application_details(fd, GetStartOfList(TRUE));
	save_widget_details(fd, GetStartOfList(TRUE)->children, 0);
	SaveConnectionList(fd, TRUE);
}


SaveTemplateSpec(fd)
FILE *fd;
{
	ObjectPtr current;

	if (GetStartOfList(FALSE) == NULL)
		return;

	/* save any comments currently being edited */
	current = CurrentlyEditing();
	EditComments((ObjectPtr)NULL);
	EditComments(current);

	print_generated_message(fd, FALSE);
	save_template_details(fd, GetStartOfList(FALSE));
	save_widget_details(fd, GetStartOfList(FALSE)->children, 0);
	SaveConnectionList(fd, FALSE);
}


String
CvtFromNewlines(str, just_escape_newlines)
String str;
Boolean just_escape_newlines;
{
#define CHUNK_SIZE 512
	String outstr = NULL;
	Cardinal i, size;

	size = strlen(str) + CHUNK_SIZE;
	outstr = XukcGrowString((String)NULL, size);

	for (i = 0; *str; str++) {
		if (i > size - 5) {
			size += CHUNK_SIZE;
			outstr = XukcGrowString(outstr, size);
		}
		if (*str == '\n') {
			if (!just_escape_newlines) {
				/* gives "\n\" */
				outstr[i++] = '\\';
				outstr[i++] = 'n';
			}
			outstr[i++] = '\\';
			outstr[i++] = '\n';
		} else
			outstr[i++] = *str;
	}
	outstr[i] = '\0';
	return (outstr);
}


/* Build a string containing the names of all the parents above this
 * object along with their associated child number.  This is used to uniquely
 * identify children with the same name.
 */
String
GetComplexName(obj)
ObjectPtr obj;
{
#define SZ 20
	Cardinal i, j, k, size;
	String str, ptr, name;
	Boolean first_time = TRUE;

	size = SZ;
	i = j = 0;
	str = XtMalloc(size + 1);

	while (obj != GetHeadObject()) {
		name = obj->name;
		j = strlen(name) + 1;

		if (j + i > size) {
			do {
				size += SZ;
			} while (j + i > size);
			str = XtRealloc(str, size + 1);
		}

		for (k = i; k > 0; k--)
			str[k + j - 1] = str[k - 1];
		ptr = str;

		while (*name) {
			*(ptr++) = *(name++);
			i++;
		}

		if (!first_time)
			*ptr = '.';
		else {
			first_time = FALSE;
			*ptr = '\0';
		}
		i++;
		obj = obj->parent;
	}
	str[i] = '\0';
	return (str);
#undef SZ
}


void
StripMatchingDatabaseLines(object)
ObjectPtr object;
{
	if (object == NULL)
		return;

	strip_matching_lines(object->children);
	StripMatchingDatabaseLines(object->children);
	StripMatchingDatabaseLines(object->next);
}
