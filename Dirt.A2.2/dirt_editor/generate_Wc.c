#ifndef lint
static char sccsid[] = "@(#)generate_Wc.c	1.2 (UKC) 5/10/92";
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
#include <pwd.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/Vendor.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

extern Boolean IsCompositeClass();
extern String GetClassVariableName();
extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern String GetAppClassName();
extern Boolean ConnectionsExist();
extern WidgetClass GetClass();
extern String CvtFromNewlines();
extern String CvtToVariableName();
extern String UniqueObjectName();
extern void PrintConnectionList();
extern void PrintRegisteredNames();
extern Boolean HasRegisteredNames();
extern Boolean IsShellClass();

#ifndef CREATION_DATE
#	define CREATION_DATE "July 1990"
#endif !CREATION_DATE
#ifndef VERSION
#	define VERSION "1.3 (Wc)"
#endif !VERSION
#ifndef UKC_INCLUDES
#	define UKC_INCLUDES "X11/Xukc"
#endif !UKC_INCLUDES


static void
create_header_list(list, list_num)
ObjectPtr list;
Cardinal *list_num;
{
	static char header_name[200];
	XtPointer *h_list;
	Cardinal k, l;

	if (list == NULL)
		return;

	if (list->is_resource) {
		create_header_list(list->next, list_num);
		return;
	}

	if (CLASS_DIR(list->class - 1) == NULL)
		(void)sprintf(header_name, "%s", CLASS_HEADER(list->class - 1));
	else
		(void)sprintf(header_name, "%s/%s", CLASS_DIR(list->class - 1),
						CLASS_HEADER(list->class - 1));

	if (*list_num == 0)
		k = 0; /* start of a new list */
	else
		h_list = XukcGetListPointer(*list_num, &k);

	for (l = 0; l < k; l++)
		if (strcmp((char *)h_list[l], header_name) == 0)
			break;

	if (l == k) /* not already in list so add the name */
		(void)XukcAddObjectToList(list_num,
					  (XtPointer)XtNewString(header_name),
					  FALSE);

	create_header_list(list->children, list_num);
	create_header_list(list->next, list_num);
}


static void
print_widget_headers(fd, list)
FILE *fd;
ObjectPtr list;
{
	Cardinal num_on_list, list_num = 0;
	XtPointer *header_list;

	create_header_list(list, &list_num);

	if (list_num == 0)
		return;

	header_list = XukcGetListPointer(list_num, &num_on_list);
	for (; num_on_list > 0; num_on_list--)
		fprintf(fd, "#include <X11/%s>\n",
					(char *)header_list[num_on_list-1]);
	XukcDestroyList(&list_num, TRUE);
}


static void
create_class_list(list, list_num)
ObjectPtr list;
Cardinal *list_num;
{
	if (list == NULL)
		return;

	if (list->is_resource) {
		create_class_list(list->next, list_num);
		return;
	}

	/* add widget class to list if it has not been entered before */
	(void)XukcAddObjectToList(list_num, (XtPointer)list->class, TRUE);

	create_class_list(list->children, list_num);
	create_class_list(list->next, list_num);
}


/* find out if any of the UI objects were created using a constructor method */
static int
has_widget_constructors(list, result)
ObjectPtr list;
char result;
{
	if (result || list == NULL)
		return (result);

	if (CLASS_CONSTRUCTOR_STRING(list->class - 1) != NO_CONSTRUCTOR_STRING)
		return 1;

	if ((result = has_widget_constructors(list->children, 0)) == 1)
		return 1;

	return (has_widget_constructors(list->next, result));
}


static void
print_widget_constructors(fd, list)
FILE *fd;
ObjectPtr list;
{
	Cardinal num_on_list, list_num = 0;
	XtPointer *class_list;

	create_class_list(list, &list_num);

	if (list_num == 0)
		return;

	class_list = XukcGetListPointer(list_num, &num_on_list);
	for (; num_on_list > 0; num_on_list--) {
		Cardinal i = (Cardinal)class_list[num_on_list-1] - 1;

		if (CLASS_CONSTRUCTOR_STRING(i) != NO_CONSTRUCTOR_STRING)
			fprintf(fd, "\t{ %s, \"%s\" },\n",
			     CLASS_CONSTRUCTOR_STRING(i),
			     CLASS_CONSTRUCTOR_STRING(i));
	}
	XukcDestroyList(&list_num, FALSE);
}

static void
print_widget_classes(fd, list)
FILE *fd;
ObjectPtr list;
{
	Cardinal num_on_list, list_num = 0;
	XtPointer *class_list;

	create_class_list(list, &list_num);

	if (list_num == 0)
		return;

	class_list = XukcGetListPointer(list_num, &num_on_list);
	for (; num_on_list > 0; num_on_list--) {
		WidgetClass wc;

		wc = GetClass((Cardinal)class_list[num_on_list-1]);

		/* shell classes etc, are registered by WcRegisterIntrinsic() */
		if (!(wc == applicationShellWidgetClass)
		    && !(wc == overrideShellWidgetClass)
		    && !(wc == shellWidgetClass)
		    && !(wc == topLevelShellWidgetClass)
		    && !(wc == transientShellWidgetClass)
		    && !(wc == vendorShellWidgetClass)
		    && !(wc == wmShellWidgetClass)
		    && !(wc == objectClass) && !(wc == rectObjClass)
		    && !(wc == coreWidgetClass) && !(wc == compositeWidgetClass)
		    && !(wc == constraintWidgetClass)
		   )
			fprintf(fd, "\t{ &%s, \"%s\" },\n",
			     CLASS_NAME((Cardinal)class_list[num_on_list-1]-1),
			     CLASS_NAME((Cardinal)class_list[num_on_list-1]-1));
	}
	XukcDestroyList(&list_num, FALSE);
}


static void
print_widget_definitions(fd, list, level, parent_is_deferred)
FILE *fd;
ObjectPtr list;
Cardinal level;
Boolean parent_is_deferred;
{
	if (list == NULL) return;

	do {
		if (!list->is_resource) {
			Cardinal i;
			String vn = CvtToVariableName(list->name);

			fprintf(fd, "Widget");
			for (i = 0; i < level; i++) fputc(' ', fd);
			if (parent_is_deferred || list->deferred_creation) {
				fprintf(fd, "%s = NULL;", vn);
				fprintf(fd, " /* deferred creation */\n");
			} else
				fprintf(fd, "%s;\n", vn);
			XtFree(vn);
			print_widget_definitions(fd, list->children, level+1,
				parent_is_deferred || list->deferred_creation);
		}
		list = list->next;
	} while (list != NULL);
}


static void
print_widget_bind_list(fd, list)
FILE *fd;
ObjectPtr list;
{
	if (list == NULL) return;

	if (!list->is_resource && !list->deferred_creation) {
		String id = UniqueObjectName(list);
		String vn = CvtToVariableName(list->name);
		/* strip trailing '.' */
		id[strlen(id)-1] = '\0';
		fprintf(fd, "\t{ &%s, \"*%s\" },\n", vn, id);
		XtFree(id);
		XtFree(vn);
	}

	if (!list->deferred_creation)
		print_widget_bind_list(fd, list->children);
	print_widget_bind_list(fd, list->next);
}


static void
print_comments(fd, comments)
FILE *fd;
String comments;
{
	String this_comment, next_comment;

	if (comments == NULL || *comments == '\0')
		/* no comments to output */
		return;

	this_comment = comments;
	next_comment = index(this_comment, '\n');
	while (next_comment != NULL) {
		*next_comment = '\0';
		fprintf(fd, "! %s\n", this_comment);
		*next_comment = '\n';
		this_comment = next_comment + 1;
		next_comment = index(this_comment, '\n');
	}
	fprintf(fd, "! %s\n", this_comment);
}


static void
print_help(fd, id, help)
FILE *fd;
String id;
String help;
{
	String tmp;

	if (help == NULL || *help == '\0')
		/* no help to output */
		return;

	tmp = CvtFromNewlines(help, FALSE);
	fprintf(fd, "*%sdirtContextHelp: %s\n", id, tmp);
	XtFree(tmp);
}


static void
print_defaults(fd, cl)
FILE *fd;
ObjectPtr cl;
{
	Cardinal i;
	String id, name, value;
	ObjectPtr start, children;

	if (cl == NULL)
		return;

	if (!cl->is_resource) {
		start = GetStartOfList(cl->in_app);
		id = UniqueObjectName(cl);

		name = CvtToVariableName(cl->name);
		fprintf(fd, "!ID\n!ID   %s (variable = %s), parent = %s\n!ID\n",
				cl->name, name, NO_PARENT(cl) ? "\"RootWindow\""
						   : cl->parent->name);
		XtFree(name);
		print_comments(fd, cl->comments);

		if (cl != start) {
			if (CLASS_CONSTRUCTOR_STRING(cl->class-1) != NULL)
				fprintf(fd, "*%swcConstructor: %s\n", id,
					CLASS_CONSTRUCTOR_STRING(cl->class-1));
			else
				fprintf(fd, "*%swcClass: %s\n", id,
					CLASS_NAME(cl->class-1));
		}

		if (cl->children != NULL) {
			Boolean printed_start = FALSE;
			children = cl->children;
			for (; children != NULL; children = children->next) {
				String cid;

				if (children->is_resource
				    || children->deferred_creation)
					continue;

				cid = UniqueObjectName(children);
				/* remove trailing '.' */
				cid[strlen(cid)-1] = '\0';
				if (!printed_start) {
					fprintf(fd, "*%swcChildren: %s", id,
							cid);
					printed_start = TRUE;
				} else
					fprintf(fd, ", %s", cid);
				XtFree(cid);
			}
			if (printed_start)
				fprintf(fd, "\n");
		}
		if (!cl->managed ||
		    !IsCompositeClass(GetClass(cl->parent->class)))
				fprintf(fd, "*%swcManaged: false\n", id);

		/* better safe than sorry */
		if (cl->width == 0)
			cl->width = cl->parent->width;
		if (cl->height == 0)
			cl->height = cl->parent->height;

		if (cl->resource_flags & X_RES_LOCKED)
			fprintf(fd, "*%sx: %d\n", id, cl->x);

		if (cl->resource_flags & Y_RES_LOCKED)
			fprintf(fd, "*%sy: %d\n", id, cl->y);

		if (cl->resource_flags & WIDTH_RES_LOCKED)
			fprintf(fd, "*%swidth: %d\n", id, cl->width);

		if (cl->resource_flags & HEIGHT_RES_LOCKED)
			fprintf(fd, "*%sheight: %d\n", id, cl->height);

		if (cl->preset_name_list > 0) {
		   i = 1;
		   while ((name = (String)XukcGetObjectOnList(cl->preset_name_list, i)) != NULL) {
			fprintf(fd, "*%s%s:", id, name);
			value = CvtFromNewlines((String)XukcGetObjectOnList(
					cl->preset_value_list, i), FALSE);
			fprintf(fd, " %s\n", value);
			XtFree(value);
			i++;
		   };
		}

		print_help(fd, id, cl->help);

		if (cl->resource_name_list > 0) {
		   i = 1;
		   while ((name = (String)XukcGetObjectOnList(cl->resource_name_list, i)) != NULL) {
			fprintf(fd, "*%s%s:", id, name);
			value = CvtFromNewlines((String)XukcGetObjectOnList(
					cl->resource_value_list, i), FALSE);
			fprintf(fd, " %s\n", value);
			XtFree(value);
			i++;
		   };
		}
		if (cl->constraint_name_list > 0) {
		   i = 1;
		   while ((name = (String)XukcGetObjectOnList(cl->constraint_name_list, i)) != NULL) {
			fprintf(fd, "*%s%s:", id, name);
			value = CvtFromNewlines((String)XukcGetObjectOnList(
					cl->constraint_value_list, i), FALSE);
			fprintf(fd, " %s\n", value);
			XtFree(value);
			i++;
		   };
		}
		if (cl->sub_resources_list > 0) {
		   i = 1;
		   while ((name = (String)XukcGetObjectOnList(cl->sub_resources_list, i)) != NULL) {
			fprintf(fd, "%s\n", name);
			i++;
		   };
		}
	} else {
		id = UniqueObjectName(cl->parent);

		if (cl->resource_name_list > 0) {
		   i = 1;
		   while ((name = (String)XukcGetObjectOnList(cl->resource_name_list, i)) != NULL) {
			fprintf(fd, "*%s%s.%s:", id, cl->name, name);
			value = CvtFromNewlines((String)XukcGetObjectOnList(
					cl->resource_value_list, i), FALSE);
			fprintf(fd, " %s\n", value);
			XtFree(value);
			i++;
		   };
		}
		if (cl->constraint_name_list > 0) {
		   i = 1;
		   while ((name = (String)XukcGetObjectOnList(cl->constraint_name_list, i)) != NULL) {
			fprintf(fd, "*%s%s.%s:", id, cl->name, name);
			value = CvtFromNewlines((String)XukcGetObjectOnList(
					cl->constraint_value_list, i), FALSE);
			fprintf(fd, " %s\n", value);
			XtFree(value);
			i++;
		   };
		}
	}
	XtFree(id);
	print_defaults(fd, cl->children);
	print_defaults(fd, cl->next);
}


static void
print_createconnections(fd)
FILE *fd;
{
	fprintf(fd, "\ncreate_connections(connections, j)\n");
	fprintf(fd, "struct w_connections connections[];\n");
	fprintf(fd, "Cardinal j;\n");
	fprintf(fd, "{\n");
	fprintf(fd, "\tCardinal i;\n\n");
	fprintf(fd, "\tfor (i=0; i < j; i++)\n");
	fprintf(fd, "\t\tif (connections[i].layout != NULL\n");
	fprintf(fd, "\t\t    && *connections[i].layout != NULL) {\n");
	fprintf(fd, "\t\t\tXukcLayoutAddConnection((UKCLayoutWidget)\n");
	fprintf(fd, "\t\t\t\t\t\t*(connections[i].layout),\n");
	fprintf(fd, "\t\t\t\t\t\t*(connections[i].w1),\n");
	fprintf(fd, "\t\t\t\t\t\t*(connections[i].w2),\n");
	fprintf(fd, "\t\t\t\t\t\tconnections[i].pos);\n");
	fprintf(fd, "\t\t\tconnections[i].layout = NULL;\n");
	fprintf(fd, "\t\t}\n");
	fprintf(fd, "}\n\n\n");
}


static void
print_name_registration(fd)
FILE *fd;
{
	if (!(HasRegisteredNames(TRUE) || HasRegisteredNames(FALSE)))
		return;

	fprintf(fd, "/* ARGSUSED */\nstatic void\n");
	fprintf(fd, "unbound_callback(w, client_data, call_data)\n");
	fprintf(fd, "Widget w;\nXtPointer client_data, call_data;\n");
	fprintf(fd, "{\n\tString params[2];\n\tCardinal np = 2;");
	fprintf(fd, "\n\tparams[0] = XtName(w);\n");
	fprintf(fd, "\tparams[1] = client_data;\n\n");
	fprintf(fd, "\tXtAppWarningMsg(XtWidgetToApplicationContext(w),\n");
	fprintf(fd, "\t\t\"unboundAppFunction\", \"unboundCallback\",");
	fprintf(fd, " \"DirtWarning\",\n");
	fprintf(fd, "\t\t\"Unbound Callback called by %%s object (client_data");
	fprintf(fd, " = 0x%%x)\",\n");
	fprintf(fd, "\t\tparams, &np);\n}\n\n\n");

	fprintf(fd, "/* ARGSUSED */\nstatic void\n");
	fprintf(fd, "unbound_action(w, event, params, num_params)\n");
	fprintf(fd, "Widget w;\nXEvent *event;\nString *params;\n");
	fprintf(fd, "Cardinal *num_params;\n");
	fprintf(fd, "{\n\tString *new_params;\n\tCardinal np = 4;\n\n");
	fprintf(fd, "\tnew_params = (String *)XtMalloc(4 * sizeof(String));\n");
	fprintf(fd, "\tnew_params[0] = XtName(w);\n");
	fprintf(fd, "\tnew_params[1] = \" \";\n");
	fprintf(fd, "\tnew_params[2] = \" \";\n");
	fprintf(fd, "\tnew_params[3] = \" \";\n\n");
	fprintf(fd, "\tif (params != NULL) {\n");
	fprintf(fd, "\t\tnew_params[1] = *num_params > 0 ? params[0] : \" \";\n");
	fprintf(fd, "\t\tnew_params[2] = *num_params > 1 ? params[1] : \" \";\n");
	fprintf(fd, "\t\tnew_params[3] = *num_params > 2 ? params[2] : \" \";\n");
	fprintf(fd, "\t}\n");
	fprintf(fd, "\n\tXtAppWarningMsg(XtWidgetToApplicationContext(w),\n");
	fprintf(fd, "\t\t\"unboundAppFunction\", \"unboundAction\",");
	fprintf(fd, " \"DirtWarning\",\n");
	fprintf(fd, "\t\t\"Unbound Action called by %%s object\\n");
	fprintf(fd, "\\tParameters = (%%s,%%s,%%s, ...)\",\n");
	fprintf(fd, "\t\tnew_params, &np);\n");
	fprintf(fd, "\tXtFree((char *)new_params);\n}\n\n\n");

	fprintf(fd, "static void\n");
	fprintf(fd, "register_names(app_context, names, num_names, actions)\n");
	fprintf(fd, "XtAppContext app_context;\n");
	fprintf(fd, "String *names;\n");
	fprintf(fd, "Cardinal num_names;\n");
	fprintf(fd, "Boolean actions;\n");
	fprintf(fd, "{\n\tCardinal i;\n\n");
	fprintf(fd, "\tfor (i = 0; i < num_names; i++)\n");
	fprintf(fd, "\t\tif (actions) {\n");
	fprintf(fd, "\t\t\tstatic XtActionsRec action_rec[] = {\n");
	fprintf(fd, "\t\t\t\t{ (String)NULL, unbound_action }\n");
	fprintf(fd, "\t\t\t};\n");
	fprintf(fd, "\t\t\taction_rec[0].string = names[i];\n");
	fprintf(fd, "\t\t\tXtAppAddActions(app_context, action_rec, 1);\n");
	fprintf(fd, "\t\t} else\n");
	fprintf(fd, "\t\t\tWcRegisterCallback(app_context, names[i],\n");
	fprintf(fd, "\t\t\t\tunbound_callback, (XtPointer)NULL);\n");
	fprintf(fd, "}\n\n\n");
}


static void
print_routines(fd)
FILE *fd;
{
	String vn = CvtToVariableName(GetStartOfList(TRUE)->name);

	fprintf(fd, "\nDisplay *\n");
	fprintf(fd, "XukcToolkitInitialize(dpy_str, app_name, app_class,");
	fprintf(fd, " argc, argv,\n\t\t\t\toptions, num_options,");
	fprintf(fd, " app_defaults)\n");
	fprintf(fd, "String dpy_str;\n");
	fprintf(fd, "String app_name;\nString app_class;\n");
	fprintf(fd, "Cardinal *argc;\nString argv[];\n");
	fprintf(fd, "XrmOptionDescRec options[];\nCardinal num_options;\n");
	fprintf(fd, "String *app_defaults;\n");
	fprintf(fd, "{\n\tstatic Boolean inited = FALSE;\n");
	fprintf(fd, "\tXtAppContext new_context;\n\tDisplay *dpy;\n");
	fprintf(fd, "\tCardinal i = 1;\n");
	fprintf(fd, "\n\tif (!inited) {\n");
	fprintf(fd, "\t\tXtToolkitInitialize();\n\t\tinited = TRUE;\n\t}\n\n");
	fprintf(fd, "\tnew_context = XtCreateApplicationContext();\n");
	fprintf(fd, "\tXtAppSetFallbackResources(new_context, app_defaults);\n");
	fprintf(fd, "\tdpy = XtOpenDisplay(new_context, dpy_str, app_name,\n");
	fprintf(fd, "\t\t\t\tapp_class, options, num_options,\n");
	fprintf(fd, "\t\t\t\targc, argv);\n");
	fprintf(fd, "\tif (dpy == NULL) {\n");
	fprintf(fd, "\t\tXtErrorMsg(\"invalidDisplay\",");
	fprintf(fd, " \"xukcToolkitInitialize\",\n");
	fprintf(fd, "\t\t\t\"XukcToolkitError\",\n");
	fprintf(fd, "\t\t\t(dpy_str == NULL ? \"Can't Open Display\" :\n");
	fprintf(fd, "\t\t\t \"Can't Open Display %%s\"), &dpy_str, &i);\n");
	fprintf(fd, "\t}\n\n");
	fprintf(fd, "\tWcRegisterIntrinsic(new_context);\n");
	fprintf(fd, "\n\tfor (i = 0; i < XtNumber(%s_classes); i++)\n", vn);
	fprintf(fd, "\t\tWcRegisterClassPtr(new_context,\n");
	fprintf(fd, "\t\t\t%s_classes[i].class_name,\n", vn);
	fprintf(fd, "\t\t\t*(%s_classes[i].class));\n\n", vn);
	if (has_widget_constructors(GetStartOfList(TRUE)->children, 0)) {
		fprintf(fd, "\tWcAllowDuplicateConstructorReg(TRUE);\n");
		fprintf(fd, "\tfor (i = 0; i < XtNumber(%s_constructors); i++)\n", vn);
		fprintf(fd, "\t\tWcRegisterConstructor(new_context,\n");
		fprintf(fd, "\t\t\t%s_constructors[i].constructor_name,\n", vn);
		fprintf(fd, "\t\t\t%s_constructors[i].constructor);\n\n", vn);
	}
	if (HasRegisteredNames(FALSE)) {
	    fprintf(fd, "\tWcAllowDuplicateCallbackReg(TRUE);\n");
	    fprintf(fd, "\tregister_names(new_context, %s_callback_names,\n",
			vn);
	    fprintf(fd, "\t\t\tXtNumber(%s_callback_names), FALSE);\n", vn);
	}
	if (HasRegisteredNames(TRUE)) {
	    fprintf(fd, "\tregister_names(new_context, %s_action_names,\n",
			vn);
	    fprintf(fd, "\t\t\tXtNumber(%s_action_names), TRUE);\n", vn);
	}
	fprintf(fd, "\treturn (dpy);\n}\n\n\n");
	fprintf(fd, "XukcDirtRegisterApplicationDefaults(screen, defs, n_defs)\n");
	fprintf(fd, "Screen *screen;\nString defs[];\nCardinal n_defs;\n{\n");
	fprintf(fd, "\tXrmDatabase rdb = NULL;\n\tCardinal i;\n\n");
	fprintf(fd, "\tfor (i = 0; i < n_defs; i++)\n");
	fprintf(fd, "\t\tXrmPutLineResource(&rdb, defs[i]);\n\n");
	fprintf(fd, "\tif (rdb != NULL) {\n");
	fprintf(fd, "#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4\n");
	fprintf(fd, "\t\tXrmDatabase sdb = XtScreenDatabase(screen);\n\n");
	fprintf(fd, "\t\tXrmCombineDatabase(rdb, &sdb, False);\n");
	fprintf(fd, "#else\n");
	fprintf(fd, "\t\tXrmMergeDatabases(DisplayOfScreen(screen)->db, &rdb);\n");
	fprintf(fd, "\t\tDisplayOfScreen(screen)->db = rdb;\n");
	fprintf(fd, "\t\t/* original dpy->db has been destroyed */\n");
	fprintf(fd, "#endif\n");
	fprintf(fd, "\t}\n");
	fprintf(fd, "}\n\n\n");
	XtFree(vn);
}


static void
print_go(fd, name, class)
FILE *fd;
String name, class;
{
	fprintf(fd, "Widget\n");
	fprintf(fd, "XukcDirtGO(dpy_str, argc, argv, options, num_options,");
	fprintf(fd, " app_defaults)\n");
	fprintf(fd, "String dpy_str;\nCardinal *argc;\nString argv[];\n");
	fprintf(fd, "XrmOptionDescRec options[];\nCardinal num_options;\n");
	fprintf(fd, "String app_defaults[];\n");
	fprintf(fd, "{\n");
	fprintf(fd, "\tWidget new_shell;\n\tDisplay *dpy;\n");
	fprintf(fd, "\n\tdpy = XukcToolkitInitialize(dpy_str, ");
	fprintf(fd, "\"%s\", \"%s\",\n", name, class);
	fprintf(fd, "\t\t\t\t\targc, argv, options, num_options,\n");
	fprintf(fd, "\t\t\t\t\tapp_defaults);\n");
	fprintf(fd, "\tnew_shell = XtAppCreateShell(\"%s\", \"%s\",\n",
								name, class);
	fprintf(fd, "\t\t\t\t\tapplicationShellWidgetClass, dpy,\n");
	fprintf(fd, "\t\t\t\t\t(ArgList)NULL, 0);\n");
	fprintf(fd, "\treturn (new_shell);\n}\n\n\n");
}


static void
print_createuserinterface(fd)
FILE *fd;
{
	String vn = CvtToVariableName(GetStartOfList(TRUE)->name);

	fprintf(fd, "void\nXukcDirtAddLayoutConnections()\n{\n");
	if (ConnectionsExist(TRUE)) {
		fprintf(fd, "\tcreate_connections(%s_widget_connections,\n",
									vn);
		fprintf(fd, "\t\t\tXtNumber(%s_widget_connections));\n", vn);
	}
	fprintf(fd, "}\n\n");
	fprintf(fd, "void\nXukcCreateUserInterface(toplevel_shell)\n");
	fprintf(fd, "Widget toplevel_shell;\n");
	fprintf(fd, "{\n\tCardinal i;\n\n");
	fprintf(fd, "\t%s = toplevel_shell;\n\n", vn);
	fprintf(fd, "\t/* create all the widgets in the user interface */\n");
	fprintf(fd, "\tWcWidgetCreation(toplevel_shell);\n");
	fprintf(fd, "\n\t/* bind the widget ids. to variables */\n");
	fprintf(fd, "\tfor (i = 0; i < XtNumber(%s_bind_list); i++) {\n", vn);
	fprintf(fd, "\t\t*(%s_bind_list[i].w) = WcFullNameToWidget(toplevel_shell,\n",
			vn);
	fprintf(fd, "\t\t\t%s_bind_list[i].str);\n", vn);
	fprintf(fd, "\t\tif (*(%s_bind_list[i].w) == NULL) {\n", vn);
	fprintf(fd, "\t\t\tprintf(\"%s: failed to bind user interface.  Missing defaults file?\\n\");\n",
			vn);
	fprintf(fd, "\t\t\texit(1);\n\t\t}\n\t}\n\n");

	if (ConnectionsExist(TRUE)) {
		fprintf(fd, "\tXukcDirtAddLayoutConnections();\n");
	}
	fprintf(fd, "}\n");
	XtFree(vn);
}


static void
print_generated_message(fd)
FILE *fd;
{
	struct passwd *pwe;
	long current_time;

	pwe = getpwuid(getuid());
	current_time = (long)time((long *)0);

	fprintf(fd, "\n/*\n * This file was generated by the DIRT interface editor\n");
	fprintf(fd, " *\tAuthor: Richard Hesketh / rlh2@ukc.ac.uk\n");
	fprintf(fd, " *\tVersion: %s of %s\n", VERSION, CREATION_DATE);
	if (pwe != NULL)
		fprintf(fd, " *\n * Generated for %s on %s",
			pwe->pw_name, ctime(&current_time));
	fprintf(fd, " */\n\n");
}


static void
print_resources_message(fd)
FILE *fd;
{
	struct passwd *pwe;
	long current_time;

	pwe = getpwuid(getuid());
	current_time = (long)time((long *)0);

	fprintf(fd, "!ID\n!ID\tThis file was automatically generated by the DIRT! interface editor\n");
	fprintf(fd, "!ID\t\tAuthor: Richard Hesketh / rlh2@ukc.ac.uk\n");
	fprintf(fd, "!ID\t\tVersion: %s of %s\n", VERSION, CREATION_DATE);
	if (pwe != NULL)
		fprintf(fd, "!ID\n!ID\tGenerated for %s on %s",
			pwe->pw_name, ctime(&current_time));
	fprintf(fd, "!ID\n");
}


/*** Module Exported Routines ***/

GenerateDefaultsFile(fd)
FILE *fd;
{
	if (GetStartOfList(TRUE) == NULL) return;

	fprintf(fd, "!ID\tX Toolkit resource file for \"%s\" application\n",
				GetStartOfList(TRUE)->name);
	print_resources_message(fd);
	print_defaults(fd, GetStartOfList(TRUE));
}


GenerateWidgetCode(fd)
FILE *fd;
{
	String vn;
	ObjectPtr sol = GetStartOfList(TRUE);

	if (sol == NULL) return;

	vn = CvtToVariableName(sol->name);
	fprintf(fd, "#ifndef %s_layout_h\n#define %s_layout_h\n", vn, vn);
	fprintf(fd, "/* $");
	fprintf(fd, "Id$\n *\n");
	fprintf(fd, " * X Toolkit program layout header file.\n");
	fprintf(fd, " * This file contains all the definitions of the static ");
	fprintf(fd, "parts\n * of the user interface.\n */\n");
	print_generated_message(fd);

	fprintf(fd, "#include <X11/Intrinsic.h>\n");
	fprintf(fd, "#include <X11/StringDefs.h>\n");
	print_widget_headers(fd, sol->children);

	if (ConnectionsExist(TRUE)) {
		fprintf(fd, "\nstruct w_connections {\n");
		fprintf(fd, "\tWidget *layout;\n");
		fprintf(fd, "\tWidget *w1;\n");
		fprintf(fd, "\tWidget *w2;\n");
		fprintf(fd, "\tPositioning pos;\n");
		fprintf(fd, "};\n");
	}

	fprintf(fd, "\n/* WIDGET/OBJECT class registration list */\n\n");
	fprintf(fd, "struct w_classes {\n");
	fprintf(fd, "\tWidgetClass *class;\n");
	fprintf(fd, "\tString class_name;\n};\n\n");

	fprintf(fd, "/* WIDGET/OBJECT constructor registration list */\n\n");
	fprintf(fd, "struct w_constructors {\n");
	fprintf(fd, "\tWidget (*constructor)();\n");
	fprintf(fd, "\tString constructor_name;\n};\n\n");

	fprintf(fd, "struct w_classes %s_classes[] = {\n", vn);
	print_widget_classes(fd, sol->children);
	fprintf(fd, "};\n\n");

	if (has_widget_constructors(sol->children, 0)) {
		fprintf(fd, "struct w_constructors %s_constructors[] = {\n", vn);
		print_widget_constructors(fd, sol->children);
		fprintf(fd, "};\n\n");
	}

	fprintf(fd, "/* WIDGET ID. VARIABLES */\n\n");
	print_widget_definitions(fd, sol, 1, FALSE);

	fprintf(fd, "\n\n/* RUNTIME BINDING STRUCTURE */\n\n");
	fprintf(fd, "struct w_bind_list {\n");
	fprintf(fd, "\tWidget *w;\n\tString str;\n");
	fprintf(fd, "};\n\nstruct w_bind_list %s_bind_list[] = {\n", vn);
	print_widget_bind_list(fd, sol->children);
	fprintf(fd, "};\n");

	if (HasRegisteredNames(FALSE)) {
		fprintf(fd, "\n\n/* CALLBACK REGISTRATION LIST */\n\n");
		fprintf(fd, "String %s_callback_names[] = {\n", vn);
		PrintRegisteredNames(fd, FALSE);
		fprintf(fd, "};\n");
	}
	if (HasRegisteredNames(TRUE)) {
		fprintf(fd, "\n/* ACTION REGISTRATION LIST */\n\n");
		fprintf(fd, "String %s_action_names[] = {\n", vn);
		PrintRegisteredNames(fd, TRUE);
		fprintf(fd, "};\n");
	}

	fprintf(fd, "\n\n/* LAYOUT CONNECTIONS */\n");
	PrintConnectionList(fd);
	fprintf(fd, "/* END */\n");
	fprintf(fd, "#endif /* %s_layout_h */\n", vn);
	XtFree(vn);
}


GenerateCreationRoutines(fd, header_file)
FILE *fd;
String header_file;
{
	fprintf(fd, "#ifndef lint\nstatic char rcsid[] =\n   \"$");
	fprintf(fd, "Id$\";\n");
	fprintf(fd, "#endif /* !lint */\n");
	print_generated_message(fd);
	fprintf(fd, "#include <X11/IntrinsicP.h>\n");
	fprintf(fd, "#include <X11/ShellP.h>\n");
	fprintf(fd, "#include <X11/Wc/WcCreate.h>\n");
	if (ConnectionsExist(TRUE))
		fprintf(fd, "#include <%s/Layout.h>\n", UKC_INCLUDES);
	fprintf(fd, "\n/* external declarations */\n");
	fprintf(fd, "extern void WcRegisterIntrinsic();\n");
	fprintf(fd, "/* fixes for Wcl constructor registration */\n");
	fprintf(fd, "#define XtCreateApplicationShell WcCreateApplicationShell\n");
	fprintf(fd, "extern Widget WcCreateApplicationShell();\n");
	fprintf(fd, "#define XtCreateOverrideShell WcCreateOverrideShell\n");
	fprintf(fd, "extern Widget WcCreateOverrideShell();\n");
	fprintf(fd, "#define XtCreateShell WcCreateShell\n");
	fprintf(fd, "extern Widget WcCreateShell();\n");
	fprintf(fd, "#define XtCreateTopLevelShell WcCreateTopLevelShell\n");
	fprintf(fd, "extern Widget WcCreateTopLevelShell();\n");
	fprintf(fd, "#define XtCreateTransientShell WcCreateTransientShell\n");
	fprintf(fd, "extern Widget WcCreateTransientShell();\n");
	fprintf(fd, "#define XtCreateWMShell WcCreateWMShell\n");
	fprintf(fd, "extern Widget WcCreateWMShell();\n");
	fprintf(fd, "#define XtCreateVendorShell WcCreateVendorShell\n");
	fprintf(fd, "extern Widget WcCreateVendorShell();\n");
	fprintf(fd, "\n");
	fprintf(fd, "#include \"%s\"\n\n", header_file);
	if (ConnectionsExist(TRUE))
		print_createconnections(fd);
	print_name_registration(fd);
	print_routines(fd);
	print_go(fd, GetStartOfList(TRUE)->name, GetAppClassName());
	print_createuserinterface(fd);
}
