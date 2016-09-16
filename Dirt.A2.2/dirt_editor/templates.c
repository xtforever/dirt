#ifndef lint
static char sccsid[] = "@(#)templates.c	1.2 (UKC) 5/10/92";
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
 * template.c : contains the functions for dealing specifically with
 *		"templates" of widgets.  This involves setting up the
 *		templates from within the ObjectPtr heirarchy.  Loading
 *		and saving of templates is done via the normal load/save
 *		mechanism with the template head being placed as a child
 *		of the UKCLayout in the ``Template Building Area''.
 */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/text.h>
#include "dirt_structures.h"

/* this is the pseudo-root of any template */
extern Widget WI_template_building_area;
extern Widget WI_template_building_shell;

extern Widget WI_templates_pressup;
extern Widget WI_templates_name_entry;
extern Widget WI_templates_available;
extern Widget WI_templates_notes;
extern Widget WI_templates_notes_title;

extern struct app_resources editor_resources;

/* externally defined routines */
extern void XukcChangeLabel();
/* in create.c */
extern ObjectPtr CreateObjectRecord();
extern ObjectPtr GetHeadObject();
extern ObjectPtr GetStartOfList();
extern void SetStartOfList();
/* in list.c */
extern Cardinal FindClass();
/* in file.c */
extern Cardinal SuffixedFilesInDirectories();
extern String GetUniqueFilename();
/* in dirt.c */
extern void PopupShell();
extern void InitializeTemplateShell();
/* in load.c */
extern Boolean ProcessFile();

/* in Widget Creation Library - Wc */
extern Widget WcFullNameToWidget();
extern void WcCreateNamedChildren();

static Cardinal template_files = 0;
static String current_template_file = NULL;
static String new_template_file = NULL;
static ObjectPtr templates = NULL;

/* for quick sort */
static int
compare_names(a, b)
String a, b;
{
	return (strcmp(*(String *)a, *(String *)b));
}


static void
update_template_list()
{
	Cardinal num;
	String *list_string;

	if (template_files == 0)
		return;

	list_string = (String *)XukcGetListPointer(template_files, &num);
	qsort(list_string, num, sizeof(String), compare_names);
	XukcScrListChange(WI_templates_available, list_string, num);
}


static void
change_current_name(new_name)
String new_name;
{
	static String title = "Notes for Template:";
	String name = NULL;

	name = XukcGrowString(name, (Cardinal)(strlen(title) +
						strlen(new_name) + 2));
	(void)sprintf(name, "%s %s", title, new_name);
	XukcChangeLabel(WI_templates_notes_title, name, FALSE);
	XtFree(name);
}


static void
load_a_template(file)
String file;
{
	extern char *reason();
	FILE *fd;
	String new_file = NULL;
	ObjectPtr new_start;

	new_file = XukcGrowString(new_file, (Cardinal)(strlen(file) +
						strlen(TEMPLATE_SUFFIX) + 1));
	(void)sprintf(new_file, "%s%s", file, TEMPLATE_SUFFIX);
	if ((fd = fopen(new_file, "r")) == NULL) {
		GiveWarning(WI_templates_available, "cannotReadTemplate",
			"loadATemplate",
			"Unable to read template file from %s : (%s)",
			2, new_file, reason());
		XtFree(new_file);
		return;
	}

	if (ProcessFile(fd, &new_start)) {
		XtFree(current_template_file);
		current_template_file = XtNewString(file);
		change_current_name(new_start->name);
		XukcNewTextWidgetString(WI_templates_notes, new_start->comments);
		SetStartOfList(new_start, FALSE);
	}

	if (fclose(fd) == EOF) {
		GiveWarning(WI_templates_available, "cannotCloseTemplate",
				"loadATemplate",
				"Unable to close %s template file after loading : (%s)",
				2, new_file, reason());
	}
	XFlush(XtDisplay(WI_template_building_area));
	XtFree(new_file);
}


/******************** Module Exported Routines **********************/

ObjectPtr
CreateTemplateHead(name)
String name;
{
	ObjectPtr new_template;

	new_template = CreateObjectRecord();
	new_template->name = XtNewString(name);
	new_template->in_app = FALSE;
	new_template->class = FindClass(XtClass(WI_template_building_area));
	new_template->instance = WI_template_building_area;

	return (new_template);
}


void
InitializeTemplates()
{
	static Boolean initialized = FALSE;
	extern Widget display_top;

	if (initialized)
		return;

	template_files = SuffixedFilesInDirectories(editor_resources.templates,
							TEMPLATE_SUFFIX);
	initialized = TRUE;
	update_template_list();

	/* create the deferred template building area because we might be
	 * putting this on a different display.
	 */
	InitializeTemplateShell();
	WcCreateNamedChildren(display_top, "templateBuildingShell");
	WI_template_building_area = WcFullNameToWidget(
					display_top, "*templateBuildingArea");

#if 0
	WI_template_building_layout = WcFullNameToWidget(GetHeadObject()->instance,
						"*templateBuildingLayout");
	WI_template_building_hide = WcFullNameToWidget(GetHeadObject()->instance,
						"*templateBuildingHide");
	WI_template_building_title = WcFullNameToWidget(GetHeadObject()->instance,
						"*templateBuildingTitle");
	XukcDirtAddLayoutConnections();
#endif

	if (WI_template_building_area == NULL)
		GiveError(WI_templates_available, "cannotBindWidget",
			"initializeTemplates",
			"Internal Error: cannot bind the templateBuildingArea",
			0);
	XFlush(XtDisplay(WI_template_building_area));
	XtAddCallback(WI_templates_pressup, XtNcallback, PopupShell,
				(XtPointer)"*templateBuildingShell");
}


/* ARGSUSED */
void
SaveCurrentTemplate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (current_template_file != NULL) {
		/* open the template file and save the current template
		 * in it .. this maybe called when choosing a new template,
		 * creating a new template, check-pointing the current
		 * template or when quiting the application.
		 */
		extern char *reason();
		FILE *fd;
		String new_file = NULL;

		new_file = XukcGrowString(new_file, (Cardinal)
						(strlen(current_template_file) +
						strlen(TEMPLATE_SUFFIX) + 1));
		(void)sprintf(new_file, "%s%s", current_template_file,
						TEMPLATE_SUFFIX);
		if ((fd = fopen(new_file, "w")) != NULL) {
			String new_notes;

			templates = GetStartOfList(FALSE);
			if (templates->comments != NULL)
				XtFree(templates->comments);

			new_notes = XukcGetTextWidgetString(WI_templates_notes);
			templates->comments = XtNewString(new_notes);
			XawAsciiSourceFreeString(
				XawTextGetSource(WI_templates_notes));
			SaveTemplateSpec(fd);

			if (fclose(fd) == EOF) {
				GiveWarning(WI_templates_available,
						"cannotCloseTemplate",
						"saveATemplate",
						"Unable to close %s template file after saving : (%s)",
						2, new_file, reason());
			}
		} else {
			GiveWarning(WI_templates_available, "cannotSaveTemplate",
				"saveATemplate",
				"Unable to save template file in %s : (%s)",
				2, new_file, reason());
		}
		XtFree(new_file);
	}
}


/* ARGSUSED */
void
PickATemplate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XawListReturnStruct *picked_string = (XawListReturnStruct *)call_data;

	SaveCurrentTemplate(w, client_data, call_data);

	if (current_template_file != NULL &&
	    strcmp(current_template_file, picked_string->string) == 0)
		return; /* not a new template .. just a checkpoint */

	load_a_template(picked_string->string);
}


/* ARGSUSED */
void
CreateTemplate(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	String new_template;

	new_template = XukcGetTextWidgetString(WI_templates_name_entry);

	if (new_template == NULL || *new_template == '\0') {
		GiveWarning(WI_templates_available, "noTemplateName",
			"createATemplate",
			"Please specify a name for the new template (a relative filename)",
			0);
		return; /* no template to create */
	}
	SaveCurrentTemplate(w, client_data, call_data);
	XtFree(current_template_file);
	current_template_file = GetUniqueFilename(&new_template,
							TEMPLATE_SUFFIX);
	change_current_name(new_template);
	(void)XukcAddObjectToList(&template_files,
				(XtPointer)XtNewString(new_template), TRUE);
	update_template_list();
	templates = CreateTemplateHead(new_template);
	XukcClearTextWidget(WI_templates_notes);
	SetStartOfList(templates, FALSE);
}


/* ARGSUSED */
void
DoCreateTemplate(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	CreateTemplate(w, (XtPointer)NULL, (XtPointer)NULL);
}
