#ifndef lint
static char sccsid[] = "@(#)dirt.c	1.3 (UKC) 5/11/92";
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
#include <X11/Xos.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xukc/ICRM.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Med3Menu.h>
#include <X11/Xukc/Error.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Wc/WcCreate.h>
#include <X11/Shell.h>
#include "dirt_bindings.h"
#include "dirt_structures.h"
#include "Dirt.h" /* defines the app defaults */

static XtAppContext dirt_context = NULL;
static XtAppContext app_context = NULL;
static XtInputId app_id = NULL;

/* externally defined routines */
/* in dirt_create.c */
extern void XukcCreateUserInterface();
/* in comment.c */
extern void EditComments();
/* in convert.c */
extern void PickAnObject();
/* in connect.c */
extern void ConnectWidgets();
extern void DisconnectWidgets();
extern void ShowLayoutConnections();
/* in undo.c */
extern void UndoAction();
/* in edit.c */
extern void UpdateString();
extern void InitializeEdit();
/* in create.c */
extern void SetStartOfList();
extern ObjectPtr GetStartOfList();
extern ObjectPtr GetHeadObject();
extern String GetAppClassName();
extern Boolean NoObjectsCreated();
extern void InitializeCreation();
extern void DeleteButton();
extern void DeleteObjectDialog();
extern void NewApplication();
extern void DeleteApplication();
extern void NewObject();
extern void ResetNewObject();
/* in copy.c */
extern void CopyObject();
extern void MoveObject();
/* in geometry.c */
extern void AddAppEventHandlers();
/* in list.c */
extern void CreateClassList();
extern void PickAClass();
extern Cardinal FindClass();
extern void InitializeWidgetClasses();
/* in load.c */
extern Boolean ProcessFile();
/* in selected.c */
extern void PickedSelection();
/* in resources.c */
extern void InitializeResourceLists();
extern void UpdateTextResource();
extern void UpdateCallbackResource();
extern void CreateResourceTable();
/* in save.c */
extern void StripMatchingDatabaseLines();
/* in Xukc/utilities/Converters.c */
extern void XukcAddConverters();
extern void XukcOverrideConverters();
/* in translations.c */
extern void InitializeTranslationsEditor();
extern void ApplyTranslations();
/* in vampire.c */
extern Boolean SuckWindow();
/* in wedit.c */
extern void UpdateWidgetResource();
/* in gravity.c */
extern void InitializeGravityLines();
/* in widget_examples.c */
extern void InitializeExamples();
/* in instance_list.c */
extern ObjectPtr CurrentlyEditing();
extern void PickAnInstance();
/* in wc_extra.c */
extern void AddDirtConverters();
extern void AddDirtCallbacks();
extern void AddDirtActions();
/* in stubs.c */
extern void RegisterAction();
extern void RegisterCallback();
extern void DoRegisterCallback();
extern void DoRegisterAction();
extern void ClearRegisteredNames();
extern void DisplayRegisteredNames();
/* in templates.c */
extern void InitializeTemplates();
extern void PickATemplate();
extern void CreateTemplate();
extern void DoCreateTemplate();
extern void SaveCurrentTemplate();
/* in Widget Creation Library - Wc */
extern Widget WcFullNameToWidget();

static void set_popup_pos();

void ChangeBorderColor();
void AfterApplicationShellCreated();
void BeforeApplicationShellCreated();
void BindCallback();
void BindAction();

/* module exported variables (globals to you bud) */
Widget display_top = NULL;

extern MENU menu_struct, pick_menu, examples_struct;

extern Widget WI_error_trap;
extern Widget WI_dirt, WI_x_pos_value, WI_y_pos_value, WI_width_value;
extern Widget WI_height_value, WI_border_value, WI_new_shell;
extern Widget WI_create_name_entry;
extern Widget WI_create_class_entry, WI_main_menu, WI_pick_parent;
extern Widget WI_class_examples;
extern Widget WI_file_dialog_shell;
extern Widget WI_delete_dialog_yes, WI_delete_dialog_no, WI_create_pressup;
extern Widget WI_create_shell, WI_new_pressup, WI_instance_pressup;
extern Widget WI_instance_shell;
extern Widget WI_callback_stubs_pressup, WI_callback_stubs_shell;
extern Widget WI_callback_stubs_register;
extern Widget WI_action_stubs_pressup, WI_action_stubs_shell;
extern Widget WI_action_stubs_register;
extern Widget WI_templates_shell, WI_templates_pressup, WI_templates_hide;
extern Widget WI_template_building_shell, WI_template_building_hide;
extern Widget WI_create_shell;
extern Widget WI_file_dialog_spec_prefix, WI_file_dialog_code_prefix;
extern Widget WI_file_dialog_save_spec, WI_file_dialog_save_code;
extern Widget WI_file_dialog_save_resources, WI_file_dialog_save_both;
extern Widget WI_file_dialog_save_cancel;
extern Widget WI_load_dialog_shell, WI_load_dialog_load, WI_load_dialog_cancel;
extern Widget WI_load_dialog_spec_prefix;
extern Widget WI_app_new, WI_app_delete;
extern Widget WI_new_create;
extern Widget WI_new_reset;
extern Widget WI_app_resources_viewport, WI_new_resources_viewport;
extern Widget WI_instance_resources_viewport, WI_new_name_entry;
extern Widget WI_new_parent_entry, WI_instance_name_entry;
extern Widget WI_class_examples, WI_pick_parent, WI_instance_managed;
extern Widget WI_instance_list, WI_comment_shell;
extern Widget WI_comment_hide, WI_app_comments, WI_edit_comments;

static ObjectPtr new_start;

static XtActionsRec actionTable[] = {
	{ "dirtACTUpdateTextResource", UpdateTextResource },
	{ "dirtACTUpdateCallbackResource", UpdateCallbackResource },
	{ "dirtACTChangeBorderColor", ChangeBorderColor },
	{ "dirtACTUpdateWidgetResource", UpdateWidgetResource },
};

/* editor resources */

struct app_resources editor_resources;

#define offset(field) XtOffset(struct app_resources *, field)
static XtResource dirt_resources[] = {
	{"appDisplay", "Display", XtRString, sizeof(String),
	 offset(display), XtRString, (XtPointer)NULL}, 
	{"outputFile", XtCFile, XtRString, sizeof(String),
	 offset(generated_file), XtRString, "./dirt_test"},
	{"specFile", XtCFile, XtRString, sizeof(String),
	 offset(spec_file), XtRString, "./dirt_test"},
	{"gravityColour", XtCForeground, XtRPixel, sizeof(Pixel),
	 offset(grav_colour), XtRString, XtExtdefaultforeground},
	{XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	 offset(background), XtRString, XtExtdefaultbackground},
	{"inches", "Inches", XtRBoolean, sizeof(Boolean),
	 offset(inches), XtRImmediate, (XtPointer)FALSE},
	{"pixels", "Pixels", XtRBoolean, sizeof(Boolean),
	 offset(pixels), XtRImmediate, (XtPointer)TRUE},
	{"templates", "Templates", XtRString, sizeof(String),
	 offset(templates), XtRImmediate, (XtPointer)""},
	{"xGridSize", "GridSize", XtRPosition, sizeof(Position),
	 offset(x_grid), XtRImmediate, (XtPointer)X_GRID_DEFAULT},
	{"yGridSize", "GridSize", XtRPosition, sizeof(Position),
	 offset(y_grid), XtRImmediate, (XtPointer)Y_GRID_DEFAULT},
	{"noGrid", "NoGrid", XtRBoolean, sizeof(Boolean),
	 offset(grid_off), XtRImmediate, (XtPointer)FALSE},
	{"noSnapping", "NoSnapping", XtRBoolean, sizeof(Boolean),
	 offset(no_snapping), XtRImmediate, (XtPointer)FALSE},
	{"gridColour", XtCForeground, XtRString, sizeof(String),
	 offset(grid_colour), XtRString, XtExtdefaultforeground},
	{"sortType", "SortType", XtRBoolean, sizeof(Boolean),
	 offset(sort_by_type), XtRImmediate, (XtPointer)FALSE}
};
#undef offset

/* command line options only for the resource editor */
static XrmOptionDescRec dirt_options[] = {
	{"-appDisplay", ".appDisplay", XrmoptionSepArg, (XtPointer)NULL},
	{"-outputFile", ".outputFile", XrmoptionSepArg, (XtPointer)NULL},
	{"-specFile", ".specFile", XrmoptionSepArg, (XtPointer)NULL},
	{"-gravityColour", ".gravityColour", XrmoptionSepArg, (XtPointer)NULL},
	{"-files", ".File", XrmoptionSepArg, (XtPointer)NULL},
	{"-inches", ".inches", XrmoptionNoArg, (XtPointer)"true"},
	{"-metric", ".inches", XrmoptionNoArg, (XtPointer)"false"},
	{"-pixels", ".pixels", XrmoptionNoArg, (XtPointer)"true"},
	{"-ruler", ".pixels", XrmoptionNoArg, (XtPointer)"false"},
	{"-templates", ".templates", XrmoptionSepArg, (XtPointer)NULL},
	{"-xgrid", ".xGridSize", XrmoptionSepArg, (XtPointer)NULL},
	{"-ygrid", ".yGridSize", XrmoptionSepArg, (XtPointer)NULL},
	{"-grid", ".GridSize", XrmoptionSepArg, (XtPointer)NULL},
	{"+showGrid", ".noGrid", XrmoptionNoArg, (XtPointer)"false"},
	{"-showGrid", ".noGrid", XrmoptionNoArg, (XtPointer)"true"},
	{"+snapToGrid", ".noSnapping", XrmoptionNoArg, (XtPointer)"false"},
	{"-snapToGrid", ".noSnapping", XrmoptionNoArg, (XtPointer)"true"},
	{"-gridColour", ".gridColour", XrmoptionSepArg, (XtPointer)NULL},
	{"-sortByType", ".sortType", XrmoptionNoArg, (XtPointer)"true"},
	{"-sortByName", ".sortType", XrmoptionNoArg, (XtPointer)"false"}
};


/* The application we are prototyping is held in a separate application
 * context and its events are processed as separate input stream on
 * the main application context.
 */
/* ARGSUSED */
static XtInputCallbackProc
process_app_events(client_data, source, id)
XtPointer client_data;
int *source;
XtInputId *id;
{
	XtAppContext ac = (XtAppContext)client_data;
	XEvent event;

	while (XtAppPending(ac)) {
		XtAppNextEvent(ac, &event);
		(void)XtDispatchEvent(&event);
	}
}


/*  return sys_errlist[errno] if in range
 */
char *
reason()
{
	extern char *sys_errlist[];
	extern int errno, sys_nerr;

	return((errno > 0 && errno < sys_nerr) ? sys_errlist[errno]
							: "unknown reason");
}


/* ARGSUSED */
static XukcIcrmProc
finished_hierarchy_transfer(client_num, window, arg1, arg2)
Cardinal client_num;
Window window;
XtPointer arg1, arg2;
{
#define SET_VALUE(w, v) { int p; p = (int)(v); XtVaSetValues(w, XtNvalue, p, NULL); }
	/* ||| */
	if (new_start == NULL)
		XtWarning("Suck failed");
	else {
		Position x, y;
		Dimension width, height, border_width;
		Arg args[5];

		XtSetArg(args[0], XtNx, &x);
		XtSetArg(args[1], XtNy, &y);
		XtSetArg(args[2], XtNwidth, &width);
		XtSetArg(args[3], XtNheight, &height);
		XtSetArg(args[4], XtNborderWidth, &border_width);
		ObjectGetValues(new_start, (ArgList)args, 5, FALSE, FALSE);
		SET_VALUE(WI_x_pos_value, x);
		SET_VALUE(WI_y_pos_value, y);
		SET_VALUE(WI_width_value, width);
		SET_VALUE(WI_height_value, height);
		SET_VALUE(WI_border_value, border_width);

		SetStartOfList(new_start, TRUE);
		AfterApplicationShellCreated(new_start);
		XtPopdown(WI_new_shell);
		CreateResourceTable(APPLICATION_RESOURCES, new_start, 0, FALSE);
		AddAppEventHandlers(new_start);
		XukcNewTextWidgetString(WI_create_name_entry, new_start->name);
		XukcNewTextWidgetString(WI_create_class_entry, new_start->name);
	}
}


/* ARGSUSED */
static void
suck_layout(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (SuckWindow(WI_dirt, &new_start,
	    (XukcIcrmProc)finished_hierarchy_transfer))
		XtWarning("Starting suck transfer please be patient 8-)");
	else
		XtWarning("Failed to start sucking");
}


#define CANCEL "cancel"
#define JUST_SPEC "spec"
#define JUST_CODE "code"
#define JUST_RESOURCES "resources"
#define SAVE_BOTH "spec&code"
#define PARAMETER_EQUALS(p) (strcmp((char *)client_data, p) == 0) 

/* ARGSUSED */
static void
save_layout(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	FILE *layout_fd, *create_fd;
	char headers[1000], code[1000];

	(void)XukcChangeButtonStatus((UKCMed3MenuWidget)WI_main_menu, SAVE,
					TRUE);
	if (PARAMETER_EQUALS(CANCEL))
		return;

	if (NoObjectsCreated()) {
		GiveWarning(w, "noObjects", "saveObjects",
				"No Objects To Save", 0);
		return;
	}

	StripMatchingDatabaseLines(GetStartOfList(TRUE));

	if (PARAMETER_EQUALS(JUST_RESOURCES) || PARAMETER_EQUALS(SAVE_BOTH)) {
		char *tmp;

		editor_resources.generated_file =
			XukcGetTextWidgetString(WI_file_dialog_code_prefix);

		/* find the directory to stick the Resources file in */
		(void)sprintf(headers, "%s", editor_resources.generated_file);
		if ((tmp = rindex(headers, '/')) == NULL)
			(void)strcpy(headers, GetAppClassName());
		else {
			*(tmp+1) = '\0';
			(void)strcat(headers, GetAppClassName());
		}

		if ((layout_fd = fopen(headers, "w")) != NULL) {
			GenerateDefaultsFile(layout_fd);
			if (fclose(layout_fd) == EOF) {
				GiveWarning(w, "cannotCloseResourcesFile",
					"saveLayout",
					"Unable to close Dirt! resources file %s : (%s)",
					2, headers, reason());
				return;
			}
		} else {
			GiveWarning(w, "badResourcesFilename", "saveLayout",
				"Unable to write Dirt! resources file %s : (%s)",
				2, headers, reason());
			return;
		}
	}

	if (PARAMETER_EQUALS(JUST_CODE) || PARAMETER_EQUALS(SAVE_BOTH)) {
		editor_resources.generated_file =
			XukcGetTextWidgetString(WI_file_dialog_code_prefix);

		(void)sprintf(headers, "%s%s", editor_resources.generated_file,
						LAYOUT_SUFFIX);
		if ((layout_fd = fopen(headers, "w")) != NULL) {
			GenerateWidgetCode(layout_fd);
			if (fclose(layout_fd) == EOF) {
				GiveWarning(w, "cannotCloseHeader", "saveLayout",
					"Unable to close Dirt! header file %s : (%s)",
					2, headers, reason());
				return;
			}
		} else {
			GiveWarning(w, "badHeaderFilename", "saveLayout",
				"Unable to write Dirt! header file %s : (%s)",
				2, headers, reason());
			return;
		}

		(void)sprintf(code, "%s%s", editor_resources.generated_file,
						CREATE_SUFFIX);
		if ((create_fd = fopen(code, "w")) != NULL) {
			char *tmp = rindex(headers, '/');

			GenerateCreationRoutines(create_fd,
						 tmp == NULL ? headers : tmp + 1);
			if (fclose(create_fd) == EOF) {
				GiveWarning(w, "fileIO", "cannotCloseCreation",
					"Unable to close Dirt! creation file %s : (%s)",
					2, code, reason());
				return;
			}
		} else {
			GiveWarning(w, "badFilename", "badSpecFilename",
				"Unable to write Dirt! creation file %s : (%s)",
				2, code, reason());
			return;
		}
	}

	if (PARAMETER_EQUALS(JUST_SPEC) || PARAMETER_EQUALS(SAVE_BOTH)) {
		editor_resources.spec_file =
			XukcGetTextWidgetString(WI_file_dialog_spec_prefix);

		(void)sprintf(code, "%s%s", editor_resources.spec_file,
						SPEC_SUFFIX);
		if ((layout_fd = fopen(code, "w")) != NULL) {
			SaveUISpec(layout_fd);

			if (fclose(layout_fd) == EOF)
				GiveWarning(w, "fileIO", "cannotCloseSpec",
				   "Unable to close Dirt! specification file %s : (%s)",
				   2, code, reason());
		} else
			GiveWarning(w, "badFilename", "badSpecFilename", 
				"Unable to write Dirt! specification to %s : (%s)",
				2, code, reason());
	}
}


/* ARGSUSED */
static void
saved_callback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	if (client_data != NULL)
		*(Boolean *)client_data = FALSE;
}


/* ARGSUSED */
static void
load_layout(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	FILE *layout_fd;
	ObjectPtr new_start;
	char spec[1000];
	static Boolean cont;

	if (strcmp((char *)client_data, CANCEL) == 0)
		return;

	if (GetStartOfList(TRUE) != NULL) {
		XtAddCallback(WI_file_dialog_save_spec, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtAddCallback(WI_file_dialog_save_code, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtAddCallback(WI_file_dialog_save_resources, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtAddCallback(WI_file_dialog_save_both, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtAddCallback(WI_file_dialog_save_cancel, XtNcallback,
				saved_callback, (XtPointer)&cont);

		set_popup_pos(w, (XtPointer)"*fileDialogShell", call_data);
		XtPopup(WI_file_dialog_shell, XtGrabExclusive);
		cont = TRUE;
		XukcConditionalLoop(XukcDirtEditor, &cont);

		XtRemoveCallback(WI_file_dialog_save_spec, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtRemoveCallback(WI_file_dialog_save_code, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtRemoveCallback(WI_file_dialog_save_resources, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtRemoveCallback(WI_file_dialog_save_both, XtNcallback,
				saved_callback, (XtPointer)&cont);
		XtRemoveCallback(WI_file_dialog_save_cancel, XtNcallback,
				saved_callback, (XtPointer)&cont);
	}
		
	editor_resources.spec_file =
			XukcGetTextWidgetString(WI_load_dialog_spec_prefix);
	(void)sprintf(spec, "%s%s", editor_resources.spec_file, SPEC_SUFFIX);

	if ((layout_fd = fopen(spec, "r")) == NULL) {
		GiveWarning(w, "badSpecFilename", "loadLayout", 
			"Unable to read Dirt! specification from %s : (%s)",
			2, spec, reason());
		return;
	}

	/* clear any registered names */
	ClearRegisteredNames();

	if (ProcessFile(layout_fd, &new_start)) {
		if (!new_start->in_app) {
			GiveWarning(w, "badSpecFile", "loadLayout", 
			"%s is a Dirt! template file not a Dirt! specification file",
					1, spec);
			ClearRegisteredNames();
		}
		SetStartOfList(new_start, TRUE);
		AfterApplicationShellCreated(new_start);
		XFlush(XtDisplay(new_start->instance));
		XukcNewTextWidgetString(WI_file_dialog_spec_prefix,
						editor_resources.spec_file);
		XukcNewTextWidgetString(WI_file_dialog_code_prefix,
						editor_resources.spec_file);
		DisplayRegisteredNames();
	} else
		ClearRegisteredNames();
	if (fclose(layout_fd) == EOF)
		GiveWarning(w, "fileIO", "cannotCloseSpec",
			"Unable to close Dirt! specification file %s : (%s)",
			2, spec, reason());
}


/* ARGSUSED */
static void
do_load_layout(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	load_layout(w, (XtPointer)"", (XtPointer)NULL);
}


/* position the shell passed as client_data at the same position
 * as the calling widget */
/* ARGSUSED */
static void
set_popup_pos(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Widget shell = WcFullNameToWidget(WI_dirt, (String)client_data);
	Position x, y;

	if (shell == NULL)
		shell = WcFullNameToWidget(GetHeadObject()->instance,
					(String)client_data);
	if (shell == NULL)
		return;

	XtVaGetValues(shell, XtNx, &x, NULL);

	if (x != -9999 && x != 9999)
		/* popup has been repositioned by the user */
		return;

	if (x == 9999 && XtParent(shell) != NULL)
		w = XtParent(shell);

	XtTranslateCoords(w, (Position)0, (Position)0, &x, &y);
	/* ||| KLUDGE: to take border into account ||| */
	x -= 2;  y -= 2;
	XtVaSetValues(shell, XtNx, x, XtNy, y, NULL);
}


/* gracefully exit the application */
/* ARGSUSED */
static void
quit(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Widget tlw = (Widget)client_data;

	XtUnmapWidget((Widget)tlw);
	XukcFinishParty(XukcDirtEditor);
	XukcDestroyApplicationContext(XtWidgetToApplicationContext(tlw));
	(void)exit(0);
}


/* ARGSUSED */
void
popup_exclusive_shell(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Widget shell = (Widget)client_data;

	if (shell == NULL)
		return;

	XtPopup(shell, XtGrabExclusive);
	XFlush(XtDisplay(shell));
}


/* ARGSUSED */
void
PopupShell(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Widget shell = WcFullNameToWidget(WI_dirt, (String)client_data);

	if (shell == NULL && display_top != NULL)
		/* look for this shell on the other display?? */
		shell = WcFullNameToWidget(display_top, (String)client_data);
	if (shell == NULL)
		return;
	XtPopup(shell, XtGrabNone);
	XFlush(XtDisplay(shell));
}


/* register the callback and actions that we are going to call */
static void
register_application_callbacks()
{
	BindCallback("positionShell", set_popup_pos);
	BindCallback("popupShell", PopupShell);
	BindCallback("RegisterAction", RegisterAction);
	BindCallback("RegisterCallback", RegisterCallback);
	BindCallback("CreateObject", NewObject);
	BindCallback("DeleteObject", DeleteButton);
	BindCallback("LoadSpec", load_layout);
	BindCallback("SaveUI", save_layout);
	BindCallback("PickInstance", PickAnInstance);
	BindCallback("SelectClass", PickAClass);
	BindCallback("LoadTemplate", PickATemplate);
	BindCallback("ApplyTranslations", ApplyTranslations);
	BindCallback("CreateTemplate", CreateTemplate);
	BindCallback("SaveCurrentTemplate", SaveCurrentTemplate);
	BindCallback("CreateApplication", NewApplication);
	BindCallback("DeleteApplication", DeleteApplication);
	BindCallback("ResetCreateObject", ResetNewObject);

	BindAction("dirtACTPickObject", PickAnObject);
	BindAction("dirtACTUpdateInstanceName", UpdateString);
	BindAction("dirtACTUpdateApplicationName", UpdateString);
	BindAction("dirtACTUpdateApplicationClass", UpdateString);
	BindAction("dirtACTUpdateParent", UpdateString);
	BindAction("dirtACTUpdateNewName", UpdateString);
	BindAction("dirtACTRegisterAction", DoRegisterAction);
	BindAction("dirtACTRegisterCallback", DoRegisterCallback);
	BindAction("dirtACTLoadSpec", do_load_layout);
	BindAction("dirtACTCreateTemplate", DoCreateTemplate);
}


/* set up the callbacks between the user interface and the application */
static void
bind_user_interface_to_application()
{
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, PICK,
			PickedSelection, (XtPointer)NULL);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, QUIT,
			SaveCurrentTemplate, (XtPointer)NULL);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, QUIT_CONFIRM,
			quit, (XtPointer)WI_dirt);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, SAVE,
			set_popup_pos, (XtPointer)"*fileDialogShell");
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, SAVE,
			popup_exclusive_shell, (XtPointer)WI_file_dialog_shell);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, LOAD,
			set_popup_pos, (XtPointer)"*loadDialogShell");
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, LOAD,
			popup_exclusive_shell, (XtPointer)WI_load_dialog_shell);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, SUCK,
			suck_layout, (XtPointer)NULL);

	InitializeGravityLines((UKCMed3MenuWidget)WI_main_menu);

	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, CONNECT_VERT,
			ConnectWidgets, (XtPointer)TRUE);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, CONNECT_HORIZ,
			ConnectWidgets, (XtPointer)FALSE);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, DISCONNECT_VERT,
			DisconnectWidgets, (XtPointer)TRUE);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, DISCONNECT_HORIZ,
			DisconnectWidgets, (XtPointer)FALSE);
	XukcBindToggle((UKCMed3MenuWidget)WI_main_menu, DISPLAY_CONNECTIONS,
			"Connections Off", "Connections On", (Boolean *)NULL,
			ShowLayoutConnections, (XtPointer)NULL);
	XukcBindForceDown((UKCMed3MenuWidget)WI_main_menu, PUSH_LAYOUT_DOWN);

	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, DELETE,
			DeleteObjectDialog, (XtPointer)NULL);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, COPY_SINGLE,
			CopyObject, (XtPointer)FALSE);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, COPY_ALL,
			CopyObject, (XtPointer)TRUE);
	XukcBindButton((UKCMed3MenuWidget)WI_main_menu, MOVE,
			MoveObject, (XtPointer)NULL);

#ifdef ACCELERATORS
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, PICK,
			":Ctrl<KeyPress>p", "^p");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, QUIT_CONFIRM,
			":Ctrl<KeyPress>Q", "^Q");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, SAVE,
			":Ctrl<KeyPress>s", "^s");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, LOAD,
			":Ctrl<KeyPress>l", "^l");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu,
			DISPLAY_CONNECTIONS, ":Ctrl<KeyPress>C", "^C");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, DELETE,
			":Ctrl<KeyPress>d", "^d");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, COPY_SINGLE,
			":Ctrl<KeyPress>c", "^c");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, COPY_ALL,
			":Ctrl<KeyPress>a", "^a");
	XukcAddAcceleratorToMenu((UKCMed3MenuWidget)WI_main_menu, MOVE,
			":Ctrl<KeyPress>m", "^m");

	XukcInstallMenuAccelerators((UKCMed3MenuWidget)WI_main_menu, WI_dirt);
	XtSetKeyboardFocus(WI_dirt, WI_dirt);
#endif /* ACCELERATORS */
}


main(argc, argv)
Cardinal argc;
char **argv;
{
	Display *dpy;
	XrmDatabase sdb;
	extern Display *XukcToolkitInitialize();

	/* sets the slot increment for utilities/memory.c created lists.
	 * we use lots of lists with a small number of entries so this
	 * gives us better fragmentation with a slight lose in speed
	 * when growing the list.
	 */
	XukcSetSlotIncrement(5);

	/* allow installation of Dirt in a place other than the standard
	 * installation directories.
	 */
#ifdef FILE_SEARCH_PATH
	(void)my_putenv(FILE_SEARCH_PATH);
#endif /* FILE_SEARCH_PATH */

	dpy = XukcToolkitInitialize((String)NULL, "dirt", "Dirt", &argc, argv,
					dirt_options, XtNumber(dirt_options),
					(String *)NULL);
	InitializeWidgetClasses();
	app_context = XtCreateApplicationContext();
	XukcAddConverters();
	XukcOverrideConverters();
	XukcDirtRegisterApplicationDefaults(DefaultScreenOfDisplay(dpy),
					app_defaults, XtNumber(app_defaults));

#ifdef BITMAP_SEARCH_PATH
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
	sdb = XtScreenDatabase(DefaultScreenOfDisplay(dpy));
#else
     	sdb = dpy->db;
#endif
      	XukcAddToBitmapFilePath(&sdb, "dirt", "Dirt", BITMAP_SEARCH_PATH);
#endif /* BITMAP_SEARCH_PATH */

	WI_dirt = XtAppCreateShell("dirt", "Dirt", applicationShellWidgetClass,
					dpy, (ArgList)NULL, 0);
	dirt_context = XtWidgetToApplicationContext(WI_dirt);
	XtAppAddActions(dirt_context, actionTable, XtNumber(actionTable));
	register_application_callbacks();
	XukcCreateUserInterface(WI_dirt);

	XtVaSetValues(WI_main_menu, XtNmenuStruct, &menu_struct, NULL);
	XtVaSetValues(WI_pick_parent, XtNmenuStruct, &pick_menu, NULL);
	XtVaSetValues(WI_class_examples, XtNmenuStruct, &examples_struct, NULL);

	XtGetApplicationResources(WI_dirt, (XtPointer)&editor_resources,
		(XtResourceList)dirt_resources, XtNumber(dirt_resources),
		(ArgList)NULL, 0);
	if (editor_resources.display == NULL)
		editor_resources.display = DisplayString(dpy);

	bind_user_interface_to_application();
	InitializeResourceLists(WI_app_resources_viewport,
				WI_new_resources_viewport,
				WI_instance_resources_viewport);
	CreateClassList();
	/* these must be done before we install our converters */
	InitializeCreation();
	InitializeTemplates();
	InitializeEdit();
	InitializeTranslationsEditor();
	InitializeExamples(WI_class_examples);

	AddDirtConverters();
	AddDirtCallbacks();
	AddDirtActions();

	/* this must be done after we install our converters */
	CreateResourceTable(APPLICATION_RESOURCES, (ObjectPtr)NULL,
				FindClass(TOP_SHELL_CLASS),
				FALSE);
	BeforeApplicationShellCreated();

	/* initialize the filename prefix text widgets */
	XukcNewTextWidgetString(WI_file_dialog_spec_prefix,
					editor_resources.spec_file);
	XukcNewTextWidgetString(WI_load_dialog_spec_prefix,
					editor_resources.spec_file);
	XukcNewTextWidgetString(WI_file_dialog_code_prefix,
					editor_resources.generated_file);
	XtRealizeWidget(WI_dirt);
	XukcMailInvites(XukcDirtEditor, WI_dirt, editor_resources.display);

	XukcAppMainLoop(dirt_context, WI_dirt);
}


/************** Module defined routines *********************/

void
BeforeApplicationShellCreated()
{
        /* set default mode */
	XtSetSensitive(WI_app_new, TRUE); /* turn on "New Application" */
	XtSetSensitive(WI_app_delete, FALSE);

	XtSetSensitive(WI_instance_managed, TRUE);
	XukcTurnOnButton((UKCMed3MenuWidget)WI_main_menu, LAYOUT);
}


void
AfterApplicationShellCreated(obj)
ObjectPtr obj;
{
	/* set up the mode settings now that an application
	 * has been started.
	 */
	if (obj->is_remote) {
		XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, LAYOUT);
		XtSetSensitive(WI_instance_managed, FALSE);
	} else {
		/* turn off "New Application" */
		XtSetSensitive(WI_app_new, FALSE);
		/* turn on "Delete Appl" */
		XtSetSensitive(WI_app_delete, TRUE);
	}
	EditComments(obj);
}


XtAppContext
GetUIContext()
{
	return (app_context);
}


XtAppContext
GetDirtContext()
{
	return (dirt_context);
}


Widget
InitializeApplicationShell(toplevel_shell, wc, name, class, args, num_args)
Widget toplevel_shell;
WidgetClass wc;
String name, class;
ArgList args;
Cardinal num_args;
{
	Widget app_shell;
	Display *newDisplay;
	XrmDatabase sdb;
	Cardinal dummy = 0;

	if (toplevel_shell == NULL)
		XtError("toplevel_shell == NULL in InitializeApplicationShell()");
	newDisplay = XtOpenDisplay(app_context, editor_resources.display,
					name, class,
					(XrmOptionDescRec *)NULL, 0,
					&dummy, (char **)NULL);
	if (newDisplay == NULL)
		GiveError(NULL, "openAppDisplay",
				"cannotOpenAppDisplay",
				"Cannot open application display: %s",
				1, editor_resources.display);
	if (app_id != NULL)
		XtRemoveInput(app_id);
	app_id = XtAppAddInput(dirt_context, ConnectionNumber(newDisplay),
					(XtPointer)XtInputReadMask,
					(XtInputCallbackProc)process_app_events,
					(XtPointer)app_context);
	XukcDirtRegisterApplicationDefaults(DefaultScreenOfDisplay(newDisplay),
						gen_defaults,
						XtNumber(gen_defaults));

#ifdef BITMAP_SEARCH_PATH
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
	sdb = XtScreenDatabase(DefaultScreenOfDisplay(newDisplay));
#else
     	sdb = newDisplay->db;
#endif
      	XukcAddToBitmapFilePath(&sdb, "dirt", "Dirt", BITMAP_SEARCH_PATH);
#endif /* BITMAP_SEARCH_PATH */

	app_shell = XtAppCreateShell(name, class, wc, newDisplay,
							args, num_args);
	return (app_shell);
}


void
InitializeTemplateShell()
{
	Display *newDisplay;
	XrmDatabase sdb;
	Cardinal dummy = 0;

	newDisplay = XtOpenDisplay(dirt_context, editor_resources.display,
					"_dirtConnection", "Dirt",
					(XrmOptionDescRec *)NULL, 0,
					&dummy, (char **)NULL);
	if (newDisplay == NULL)
		GiveError(NULL, "openAppDisplay",
				"cannotOpenAppDisplay",
				"Cannot open application display: %s",
				1, editor_resources.display);
	XukcDirtRegisterApplicationDefaults(DefaultScreenOfDisplay(newDisplay),
						gen_defaults,
						XtNumber(gen_defaults));

#ifdef BITMAP_SEARCH_PATH
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
	sdb = XtScreenDatabase(DefaultScreenOfDisplay(newDisplay));
#else
     	sdb = newDisplay->db;
#endif
      	XukcAddToBitmapFilePath(&sdb, "dirt", "Dirt", BITMAP_SEARCH_PATH);
#endif /* BITMAP_SEARCH_PATH */

	display_top = XtVaAppCreateShell("_dirtConnection", "Dirt",
				overrideShellWidgetClass, newDisplay,
				XtNx, -10, XtNy, -10,
				XtNwidth, 1, XtNheight, 1,
				NULL);
	XtRealizeWidget(display_top);
	XtUnmapWidget(display_top);
}


/* ||| I don't like this but if I use XtAppPending() it can block in
 *     XtAppNextEvent() even though an event is supposed to be waiting.
 * |||
 */
void
DrainUIEventQueue()
{
	XEvent event;
	Widget w = GetHeadObject()->instance;

	/* sure-fire method of prodding the server to process all events??? */
	XNoOp(XtDisplayOfObject(w));
	XSync(XtDisplayOfObject(w), FALSE);

	while (XPending(XtDisplayOfObject(w))) {
		XtAppNextEvent(app_context, &event);
		(void)XtDispatchEvent(&event);
	}
}


/* ARGSUSED */
static void
ChangeBorderColor(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	Pixel fg, bg;

	XtVaGetValues(w, XtNforeground, &fg, XtNbackground, &bg, NULL);
	XtVaSetValues(w, XtNborderColor, *num_params ? fg : bg, NULL);
}


/* make the editing of resources impossible .. need to do this for
 * remote applications where transfers of data are asynchronous.
 */
void
DisallowEditing()
{
	XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, PICK);
	XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, DELETE);
	XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, COPY);
	XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, LOAD);
	XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, MOVE);
	XtSetSensitive(WI_create_shell, FALSE);
	XtSetSensitive(WI_instance_shell, FALSE);
	XtSetSensitive(WI_instance_list, FALSE);
}


void
AllowEditing()
{
	XukcTurnOnButton((UKCMed3MenuWidget)WI_main_menu, PICK);
	XukcTurnOnButton((UKCMed3MenuWidget)WI_main_menu, DELETE);
	XukcTurnOnButton((UKCMed3MenuWidget)WI_main_menu, COPY);
	XukcTurnOnButton((UKCMed3MenuWidget)WI_main_menu, LOAD);
	XukcTurnOffButton((UKCMed3MenuWidget)WI_main_menu, MOVE);
	XtSetSensitive(WI_create_shell, TRUE);
	XtSetSensitive(WI_instance_shell, TRUE);
	XtSetSensitive(WI_instance_list, TRUE);
	if (CurrentlyEditing())
		(void)XukcScrListSelectItem(WI_instance_list,
				CurrentlyEditing()->mark_num, FALSE, TRUE);
}


/* VARARGS */
void
GiveMessage(name, type, message, num_params, p1, p2, p3, p4, p5, p6, p7)
String name, type, message;
Cardinal num_params;
String p1, p2, p3, p4, p5, p6, p7;
{
	String params[7];

	params[0] = p1;
	params[1] = p2;
	params[2] = p3;
	params[3] = p4;
	params[4] = p5;
	params[5] = p6;
	params[6] = p7;

	XtAppWarningMsg(XtWidgetToApplicationContext(WI_dirt),
			name, type, "DirtError",
			message, params, &num_params);
}


/* VARARGS */
void
GiveWarning(w, name, type, defstr, num_params, p1, p2, p3, p4, p5)
Widget w;
String name, type, defstr;
Cardinal num_params;
String p1, p2, p3, p4, p5;
{
	String params[5];

	params[0] = p1;
	params[1] = p2;
	params[2] = p3;
	params[3] = p4;
	params[4] = p5;

	if (w == NULL)
		XtWarningMsg(name, type, "DirtWarning",
			defstr, params, &num_params);
	else
		XtAppWarningMsg(XtWidgetToApplicationContext(w),
			name, type, "DirtWarning",
			defstr, params, &num_params);
}


/* VARARGS */
void
GiveError(w, name, type, defstr, num_params, p1, p2, p3, p4, p5)
Widget w;
String name, type, defstr;
Cardinal num_params;
String p1, p2, p3, p4, p5;
{
	String params[5];

	params[0] = p1;
	params[1] = p2;
	params[2] = p3;
	params[3] = p4;
	params[4] = p5;

	if (w == NULL)
		XtErrorMsg(name, type, "DirtError",
			defstr, params, &num_params);
	else
		XtAppErrorMsg(XtWidgetToApplicationContext(w),
			name, type, "DirtError",
			defstr, params, &num_params);
}


/* output a message with no beep! */
void
Output(format, num_params, a, b, c, d, e, f)
char *format;
Cardinal num_params;
char *a, *b, *c, *d, *e, *f;
{
	Boolean beep;

	XtVaGetValues(WI_error_trap, XtNbeepOnError, &beep, NULL);
	XtVaSetValues(WI_error_trap, XtNbeepOnError, FALSE, NULL);

	GiveMessage("output", "outputMessage", format, num_params,
							a, b, c, d, e, f);
	XtVaSetValues(WI_error_trap, XtNbeepOnError, beep, NULL);
}


void
BindCallback(name, proc)
String name;
XtCallbackProc proc;
{
	WcRegisterCallback(dirt_context, name, proc, (XtPointer)NULL);
}


void
BindAction(name, proc)
String name;
XtActionProc proc;
{
	static XtActionsRec action_rec[] = {
		{ (String)NULL, (XtActionProc)NULL }
	};
	action_rec[0].string = name;
	action_rec[0].proc = proc;
	XtAppAddActions(dirt_context, action_rec, 1);
}
