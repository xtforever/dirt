#ifndef lint
static char rcsid[] =
   "$Id: gspreview_create.c,v 1.2 91/12/17 12:00:15 rlh2 Rel $";
#endif /* !lint */

/*
 * This file was generated by the DIRT interface editor
 *	Author: Richard Hesketh / rlh2@ukc.ac.uk
 *	Version: 1.3 (Wc) of July 1990
 *
 * Generated for rlh2 on Mon Dec 16 16:29:54 1991
 */

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/Wc/WcCreate.h>

/* fixes for Wcl constructor registration */
#define XtCreateApplicationShell WcCreateApplicationShell
extern Widget WcCreateApplicationShell();
#define XtCreateOverrideShell WcCreateOverrideShell
extern Widget WcCreateOverrideShell();
#define XtCreateShell WcCreateShell
extern Widget WcCreateShell();
#define XtCreateTopLevelShell WcCreateTopLevelShell
extern Widget WcCreateTopLevelShell();
#define XtCreateTransientShell WcCreateTransientShell
extern Widget WcCreateTransientShell();
#define XtCreateWMShell WcCreateWMShell
extern Widget WcCreateWMShell();
#define XtCreateVendorShell WcCreateVendorShell
extern Widget WcCreateVendorShell();

#include "gspreview_layout.h"

/* ARGSUSED */
static void
unbound_callback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	String params[2];
	Cardinal np = 2;
	params[0] = XtName(w);
	params[1] = client_data;

	XtAppWarningMsg(XtWidgetToApplicationContext(w),
		"unboundAppFunction", "unboundCallback", "DirtWarning",
		"Unbound Callback called by %s object (client_data = 0x%x)",
		params, &np);
}


/* ARGSUSED */
static void
unbound_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	String *new_params;
	Cardinal np = 4;

	new_params = (String *)XtMalloc(4 * sizeof(String));
	new_params[0] = XtName(w);
	new_params[1] = " ";
	new_params[2] = " ";
	new_params[3] = " ";

	if (params != NULL) {
		new_params[1] = *num_params > 0 ? params[0] : " ";
		new_params[2] = *num_params > 1 ? params[1] : " ";
		new_params[3] = *num_params > 2 ? params[2] : " ";
	}

	XtAppWarningMsg(XtWidgetToApplicationContext(w),
		"unboundAppFunction", "unboundAction", "DirtWarning",
		"Unbound Action called by %s object\n\tParameters = (%s,%s,%s, ...)",
		new_params, &np);
	XtFree((char *)new_params);
}


static void
register_names(app_context, names, num_names, actions)
XtAppContext app_context;
String *names;
Cardinal num_names;
Boolean actions;
{
	Cardinal i;

	for (i = 0; i < num_names; i++)
		if (actions) {
			static XtActionsRec action_rec[] = {
				{ (String)NULL, unbound_action }
			};
			action_rec[0].string = names[i];
			XtAppAddActions(app_context, action_rec, 1);
		} else
			WcRegisterCallback(app_context, names[i],
				unbound_callback, (XtPointer)NULL);
}



Display *
XukcToolkitInitialize(dpy_str, app_name, app_class, argc, argv,
				options, num_options, app_defaults)
String dpy_str;
String app_name;
String app_class;
Cardinal *argc;
String argv[];
XrmOptionDescRec options[];
Cardinal num_options;
String *app_defaults;
{
	static Boolean inited = FALSE;
	XtAppContext new_context;
	Display *dpy;
	Cardinal i = 1;

	if (!inited) {
		XtToolkitInitialize();
		inited = TRUE;
	}

	new_context = XtCreateApplicationContext();
	XtAppSetFallbackResources(new_context, app_defaults);
	dpy = XtOpenDisplay(new_context, dpy_str, app_name,
				app_class, options, num_options,
				argc, argv);
	if (dpy == NULL) {
		XtErrorMsg("invalidDisplay", "xukcToolkitInitialize",
			"XukcToolkitError",
			(dpy_str == NULL ? "Can't Open Display" :
			 "Can't Open Display %s"), &dpy_str, &i);
	}

	WcRegisterIntrinsic(new_context);

	for (i = 0; i < XtNumber(WI_gspreview_classes); i++)
		WcRegisterClassPtr(new_context,
			WI_gspreview_classes[i].class_name,
			*(WI_gspreview_classes[i].class));

	WcAllowDuplicateConstructorReg(TRUE);
	for (i = 0; i < XtNumber(WI_gspreview_constructors); i++)
		WcRegisterConstructor(new_context,
			WI_gspreview_constructors[i].constructor_name,
			WI_gspreview_constructors[i].constructor);

	WcAllowDuplicateCallbackReg(TRUE);
	register_names(new_context, WI_gspreview_callback_names,
			XtNumber(WI_gspreview_callback_names), FALSE);
	register_names(new_context, WI_gspreview_action_names,
			XtNumber(WI_gspreview_action_names), TRUE);
	return (dpy);
}


XukcDirtRegisterApplicationDefaults(screen, defs, n_defs)
Screen *screen;
String defs[];
Cardinal n_defs;
{
	XrmDatabase rdb = NULL;
	Cardinal i;

	for (i = 0; i < n_defs; i++)
		XrmPutLineResource(&rdb, defs[i]);

	if (rdb != NULL) {
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
		XrmDatabase sdb = XtScreenDatabase(screen);

		XrmCombineDatabase(rdb, &sdb, False);
#else
		XrmMergeDatabases(DisplayOfScreen(screen)->db, &rdb);
		DisplayOfScreen(screen)->db = rdb;
		/* original dpy->db has been destroyed */
#endif
	}
}


Widget
XukcDirtGO(dpy_str, argc, argv, options, num_options, app_defaults)
String dpy_str;
Cardinal *argc;
String argv[];
XrmOptionDescRec options[];
Cardinal num_options;
String app_defaults[];
{
	Widget new_shell;
	Display *dpy;

	dpy = XukcToolkitInitialize(dpy_str, "gspreview", "GSPreview",
					argc, argv, options, num_options,
					app_defaults);
	new_shell = XtAppCreateShell("gspreview", "GSPreview",
					applicationShellWidgetClass, dpy,
					(ArgList)NULL, 0);
	return (new_shell);
}


void
XukcDirtAddLayoutConnections()
{
}

void
XukcCreateUserInterface(toplevel_shell)
Widget toplevel_shell;
{
	Cardinal i;

	WI_gspreview = toplevel_shell;

	/* create all the widgets in the user interface */
	WcWidgetCreation(toplevel_shell);

	/* bind the widget ids. to variables */
	for (i = 0; i < XtNumber(WI_gspreview_bind_list); i++) {
		*(WI_gspreview_bind_list[i].w) = WcFullNameToWidget(toplevel_shell,
			WI_gspreview_bind_list[i].str);
		if (*(WI_gspreview_bind_list[i].w) == NULL) {
			printf("WI_gspreview: failed to bind user interface.  Missing defaults file?\n");
			exit(1);
		}
	}

}
