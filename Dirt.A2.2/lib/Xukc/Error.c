#ifndef lint
static char sccsid[] = "@(#)Error.c	1.2 (UKC) 5/10/92";
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

#define BUG_IN_XRM 1

#include <stdio.h>
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xukc/memory.h>
#include "ErrorP.h"

static void Initialize();
static Boolean SetValues();
static void Destroy();

/* ||| yukky, horrible, no context way of finding out which error widget
 * ||| to send the message to ... both the Xt error and Xlib error handling
 * ||| routines don't allow any special data to be passed to be used by the
 * ||| registered error/warning handler!
 * ||| */
static Cardinal xlib_error_handler_stack = 0;
static Cardinal xt_error_handler_stack = 0;

/****************************************************************
 *
 * UKCError Resources
 *
 ****************************************************************/

#define offset(field) XtOffset(UKCErrorWidget, error.field)
static XtResource resources[] = {
	{ XtNerrorTitle, XtCErrorTitle, XtRString, sizeof(String),
	  offset(title), XtRString, (XtPointer)NULL },
	{ XtNerrorCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(callbacks), XtRCallback, (XtPointer)NULL },
	{ XtNhandleXtErrors, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  offset(handle_xt), XtRImmediate, (XtPointer)TRUE },
	{ XtNhandleXlibErrors, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  offset(handle_xlib), XtRImmediate, (XtPointer)TRUE },
	{ XtNhistoryList, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  offset(history_list), XtRImmediate, (XtPointer)TRUE },
	{ XtNpauseOnError, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  offset(pause), XtRImmediate, (XtPointer)FALSE },
	{ XtNbeepOnError, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  offset(beep), XtRImmediate, (XtPointer)TRUE },
	{ XtNbeepVolume, XtCVolume, XtRInt, sizeof(int),
	  offset(volume), XtRImmediate, (XtPointer)-50 },
	{ XtNcurrentHandler, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  offset(is_current), XtRImmediate, (XtPointer)FALSE },
};
#undef offset

static String defaults = "\
	*UKCError*Edge: chainleft\n\
	*UKCError.errorLabel.label: Warnings:\n\
	*UKCError.errorLabel.borderWidth: 0\n\
	*UKCError.clearButton.label: Clear Messages\n\
	*UKCError.clearButton.fromHoriz: errorLabel\n\
	*UKCError.hideButton.label: Hide Messages\n\
	*UKCError.hideButton.fromHoriz: clearButton\n\
	*UKCError.errorText.fromVert: clearButton\n\
	*UKCError.errorText.scrollVertical: always\n\
	*UKCError.errorText.scrollHorizontal: whenNeeded\n\
	*UKCError.errorText.displayCaret: false\n\
	*UKCError.errorText.resizable: true\n\
	*UKCError.errorText.top: chaintop\n\
	*UKCError.errorText.right: chainright\n\
	*UKCError.errorText.bottom: chainbottom\n\
	*UKCError.errorText.width: 400\n\
	*UKCError.errorText.height: 110\n\
	*UKCError.errorText.translations:\
		<FocusIn>:   display-caret(on)\\n\
		<FocusOut>:  display-caret(off)\\n\
		<Btn1Down>:  select-start()\\n\
		Button1<MotionNotify>: extend-adjust()\\n\
		<Btn1Up>:    extend-end(PRIMARY)\\n\
		<Btn3Down>:  extend-start()\\n\
		Button3<MotionNotify>: extend-adjust()\\n\
		<Btn3Up>:    extend-end(PRIMARY)\n\
";

#ifdef BUG_IN_XRM
static String errorTextTranslations = " \
		<FocusIn>:   display-caret(on)\n\
		<FocusOut>:  display-caret(off)\n\
		<Btn1Down>:  select-start()\n\
		Button1<MotionNotify>: extend-adjust()\n\
		<Btn1Up>:    extend-end(PRIMARY)\n\
		<Btn3Down>:  extend-start()\n\
		Button3<MotionNotify>: extend-adjust()\n\
		<Btn3Up>:    extend-end(PRIMARY)";

static XtTranslations parsed_translations = NULL;
#endif /* BUG_IN_XRM */


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

UKCErrorClassRec ukcErrorClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &formClassRec,
    /* class_name         */    "UKCError",
    /* widget_size        */    sizeof(UKCErrorRec),
    /* class_initialize   */    NULL,
    /* class_part_init    */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions	  */	0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* version            */	XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL,
  },{
/* composite_class fields */
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension          */	NULL,
  },{
/* constraint_class fields */
    /* subresourses       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(FormConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL,
  }, {
/* Athena Form class fields */
    /* layout		  */	XtInheritLayout,
  }, {
/* UKCError class fields */
    /* empty		  */	0,
  }
};

WidgetClass ukcErrorWidgetClass = (WidgetClass)&ukcErrorClassRec;

/****************************************************************
 *
 * Private Routines
 *
 ****************************************************************/

static Widget
current_error_widget(xt_handler)
Boolean xt_handler;
{
	XtPointer *stack;
	Cardinal on_list;
	Cardinal list_num;

	list_num = xt_handler ? xt_error_handler_stack
			      : xlib_error_handler_stack;

	if (list_num == 0)
		return NULL;

	stack = XukcGetListPointer(list_num, &on_list);

	if (on_list == 0)
		return NULL;

	return (Widget)stack[on_list-1];
}


static void
push_handler_on_to_stack(xt_handler, ew)
Boolean xt_handler;
Widget ew;
{
	Cardinal on_list;
	Cardinal *list_num;

	list_num = xt_handler ? &xt_error_handler_stack
			      : &xlib_error_handler_stack;

	if (*list_num > 0)
		on_list = XukcFindObjectOnList(*list_num, (XtPointer)ew);
	else
		on_list = 0;

	if (on_list > 0)
		XukcRemoveEntryOnList(list_num, on_list);

	(void)XukcAddObjectToList(list_num, (XtPointer)ew, FALSE);
}


static void
remove_handler_from_stack(xt_handler, ew)
Boolean xt_handler;
{
	Cardinal on_list;
	Cardinal *list_num;

	list_num = xt_handler ? &xt_error_handler_stack
			      : &xlib_error_handler_stack;

	if (*list_num == 0)
		return;

	on_list = XukcFindObjectOnList(*list_num, (XtPointer)ew);

	if (on_list > 0)
		XukcRemoveEntryOnList(list_num, on_list);
}
	
	
static void
show_error_widget(ew)
UKCErrorWidget ew;
{
	Widget w = (Widget)ew;

	XtManageChild(w);

	if (XtIsSubclass(XtParent(w), shellWidgetClass)) {
		Arg args[2];
		Widget toplevel = XtParent(w);
		Position x, y;

		/* recursive up widget tree to find toplevel app shell */
		while (XtParent(toplevel) != NULL)
			toplevel = XtParent(toplevel);

		if (XtParent(w)->core.x == 0 && XtParent(w)->core.y == 0) {
			XtTranslateCoords(toplevel, 20, 20, &x, &y);
			XtSetArg(args[0], XtNx, x);
			XtSetArg(args[1], XtNy, y);
			XtSetValues(XtParent(w), (ArgList)args, 2);
		}

		XtPopup(XtParent(w), ew->error.pause ? XtGrabExclusive :
						       XtGrabNone);
	}
}


static void
remove_error_widget(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	w = XtParent(w);

	if (XtIsSubclass(XtParent(w), shellWidgetClass))
		XtPopdown(XtParent(w));

	XtUnmanageChild(w);
}


static void
update_error_text(ew, error_string)
UKCErrorWidget ew;
String error_string;
{
#define BIG_END (XawTextPosition)9999999
	XawTextBlock text;

	text.format = FMT8BIT;
	text.firstPos = 0;
	text.length = strlen(error_string);
	text.ptr = error_string;

	if (XawTextReplace(ew->error.error_text, ew->error.history_list ?
	    BIG_END : 0L, BIG_END, &text) != XawEditDone) {
		fprintf(stderr, "XukcErrorWidget: Very fatal error .. cannot output error message\n");
		exit(-1);
	}

	/* make the text widget scroll to the new error */
	XawTextSetInsertionPoint(ew->error.error_text, BIG_END);

	/* popup or re-manage (and therefore display) the error widget */
	show_error_widget(ew);

	if (ew->error.beep)
		XBell(XtDisplay((Widget)ew), ew->error.volume);

	XtCallCallbacks((Widget)ew, XtNerrorCallback, (XtPointer)error_string);
}


static void
clear_messages(ew)
UKCErrorWidget ew;
{
	Boolean history_list = ew->error.history_list;
	Boolean beep = ew->error.beep;

	ew->error.history_list = FALSE;
	ew->error.beep = FALSE;
	update_error_text(ew, "");
	ew->error.history_list = history_list;
	ew->error.beep = beep;
}


static void
clear_error(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	UKCErrorWidget ew = (UKCErrorWidget)XtParent(w);
	clear_messages(ew);
}


static void
change_history_list(ew)
UKCErrorWidget ew;
{
	if (ew->error.history_list) {
#ifndef BUG_IN_XRM
		ew->error.hide_button = XtCreateManagedWidget("hideButton",
						ukcCommandWidgetClass,
						(Widget)ew, (ArgList)NULL, 0);
#else
		ew->error.hide_button = XtVaCreateManagedWidget("hideButton",
						ukcCommandWidgetClass,
						(Widget)ew,
						XtNlabel, "Hide Messages",
						XtNfromHoriz, 
						ew->error.clear_button,
						NULL);
#endif /* BUG_IN_XRM */
		XtAddCallback(ew->error.hide_button, XtNcallback,
					remove_error_widget, (XtPointer)NULL);
	} else {
		if (ew->error.hide_button != NULL) {
			XtDestroyWidget(ew->error.hide_button);
			ew->error.hide_button = NULL;
		}
	}
}


static int
cvt_xlib_error_to_xt_warning(dpy, error_event)
Display *dpy;
XErrorEvent *error_event;
{
#define ERRSIZE 1000
	char tmp[ERRSIZE], tmp1[30], tmp2[ERRSIZE], tmp3[ERRSIZE];
	Widget xlib_error_widget;

	if ((xlib_error_widget = current_error_widget(FALSE)) == NULL)
		return NULL; /* should never happen 8-{ */

	XGetErrorText(XtDisplay(xlib_error_widget), error_event->error_code,
			tmp, ERRSIZE);

	(void)sprintf(tmp1, "XRequest.%d", error_event->request_code);
	XGetErrorDatabaseText(XtDisplay(xlib_error_widget),
				"XukcErrorWidget", tmp1, "a", tmp2,
				ERRSIZE);
	(void)sprintf(tmp3, "%s in %s protocol request\n", tmp, tmp2);
	update_error_text((UKCErrorWidget)xlib_error_widget, tmp3);
	return NULL;
}


static void
xt_warning_handler(message)
String message;
{
	Widget xt_error_widget = current_error_widget(TRUE);

	if (xt_error_widget != NULL) {
		char tmp[10000];

		(void)sprintf(tmp, "Toolkit Warning: %s\n", message);
		update_error_text((UKCErrorWidget)xt_error_widget, tmp);
	}
}


static void
xt_warning_msg_handler(name, type, class, defaultp, params, num_params)
String name, type, class, defaultp, *params;
Cardinal *num_params;
{
	char big_buffer[10000], tmp[10000];
	Widget xt_error_widget = current_error_widget(TRUE);

	if (xt_error_widget == NULL)
		return;

	XtAppGetErrorDatabaseText(XtWidgetToApplicationContext(xt_error_widget),					name, type, class, defaultp,
					big_buffer, 10000, (XrmDatabase)NULL);
	if (num_params != NULL && *num_params > 0) {
		(void)sprintf(tmp, big_buffer, params[0], params[1],
				params[2], params[3], params[4], params[5],
				params[6]);
		update_error_text((UKCErrorWidget)xt_error_widget,
					strcat(tmp, "\n"));
	} else
		update_error_text((UKCErrorWidget)xt_error_widget,
					strcat(big_buffer, "\n"));
}


#ifndef BUG_IN_XRM
/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCErrorWidget ew = (UKCErrorWidget)new;
	Arg args[2];
	XrmDatabase rdb = NULL, orig_db = NULL;
	Cardinal i;

	/* register the defaults for the sub-parts */
	rdb = XrmGetStringDatabase(defaults);

	if (rdb != NULL) {
		orig_db = XtDatabase(XtDisplay(ew));
		XrmMergeDatabases(orig_db, &rdb);
		/* original dpy->db has been destroyed */
		XtDisplay(ew)->db = rdb;
	}

	if (new->core.width == 0) /* get our width from our parent */
		new->core.width = XtParent(new)->core.width;
	if (new->core.height == 0) /* and out height */
		new->core.height = XtParent(new)->core.height;

	i = 0;
	if (ew->error.title != NULL) {
		XtSetArg(args[0], XtNlabel, ew->error.title); i++;
	}
	(void)XtCreateManagedWidget("errorLabel", ukcLabelWidgetClass,
							new, (ArgList)args, i);
	i = 0;
	ew->error.clear_button = XtCreateManagedWidget("clearButton",
							ukcCommandWidgetClass,
							new, (ArgList)args, i);
	XtAddCallback(ew->error.clear_button, XtNcallback,
					clear_error, (XtPointer)NULL);
	XtAddCallback(ew->error.clear_button, XtNcallback,
					remove_error_widget, (XtPointer)NULL);
	i = 0;
	XtSetArg(args[i], XtNeditType, XawtextEdit); i++;
	ew->error.error_text = XtCreateManagedWidget("errorText",
				asciiTextWidgetClass, new, (ArgList)args, i);

	ew->error.hide_button = NULL;
	change_history_list(ew);

	/* intercept Xlib protocol errors */
	if (ew->error.handle_xlib) {
		XSetErrorHandler(cvt_xlib_error_to_xt_warning);
		push_handler_on_to_stack(FALSE, new);
	}

	/* intercept Xt warnings */
	if (ew->error.handle_xt) {
		XtSetWarningMsgHandler(xt_warning_msg_handler);
		XtSetWarningHandler(xt_warning_handler);
		push_handler_on_to_stack(TRUE, new);
	}
}

#else

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	Widget el;
	UKCErrorWidget ew = (UKCErrorWidget)new;

	if (new->core.width == 0) /* get our width from our parent */
		new->core.width = XtParent(new)->core.width;
	if (new->core.height == 0) /* and out height */
		new->core.height = XtParent(new)->core.height;

	el = XtVaCreateManagedWidget("errorLabel", ukcLabelWidgetClass, new,
		XtNlabel,
		ew->error.title != NULL ? ew->error.title : "Warnings:",
		XtNborderWidth, 0, NULL);

	ew->error.clear_button = XtVaCreateManagedWidget("clearButton",
		ukcCommandWidgetClass, new, XtNlabel, "Clear Messages",
		XtNfromHoriz, el, NULL);
	XtAddCallback(ew->error.clear_button, XtNcallback,
					clear_error, (XtPointer)NULL);
	XtAddCallback(ew->error.clear_button, XtNcallback,
					remove_error_widget, (XtPointer)NULL);

	if (parsed_translations == NULL)
		parsed_translations =
			XtParseTranslationTable(errorTextTranslations);

	ew->error.error_text = XtVaCreateManagedWidget("errorText",
				asciiTextWidgetClass, new,
				XtNeditType, XawtextEdit,
				XtNfromVert, ew->error.clear_button,
				XtNscrollVertical, XawtextScrollAlways,
				XtNscrollHorizontal, XawtextScrollWhenNeeded,
				XtNdisplayCaret, FALSE,
				XtNresizable, TRUE,
				XtNwidth, 400,
				XtNheight, 110,
				XtNtranslations, parsed_translations,
				NULL);

	ew->error.hide_button = NULL;
	change_history_list(ew);

	/* intercept Xlib protocol errors */
	if (ew->error.handle_xlib) {
		XSetErrorHandler(cvt_xlib_error_to_xt_warning);
		push_handler_on_to_stack(FALSE, new);
	}

	/* intercept Xt warnings */
	if (ew->error.handle_xt) {
		XtSetWarningMsgHandler(xt_warning_msg_handler);
		XtSetWarningHandler(xt_warning_handler);
		push_handler_on_to_stack(TRUE, new);
	}
}
#endif /* BUG_IN_XRM */


/* ARGSUSED */
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
	UKCErrorWidget curew = (UKCErrorWidget)current;
	UKCErrorWidget newew = (UKCErrorWidget)new;

	if (curew->error.history_list != newew->error.history_list)
		change_history_list(newew);

	if (curew->error.handle_xlib != newew->error.handle_xlib) {
		if (newew->error.handle_xlib) {
			XSetErrorHandler(cvt_xlib_error_to_xt_warning);
			push_handler_on_to_stack(FALSE, new);
		} else {
			XSetErrorHandler(NULL);
			remove_handler_from_stack(FALSE, new);
		}
	}

	if (curew->error.handle_xt != newew->error.handle_xt) {
		if (newew->error.handle_xt) {
			XtAppSetWarningHandler(
					XtWidgetToApplicationContext(new),
							xt_warning_handler);
			XtAppSetWarningMsgHandler(
					XtWidgetToApplicationContext(new),
							xt_warning_msg_handler);
			push_handler_on_to_stack(TRUE, new);
		} else {
			XtAppSetWarningMsgHandler(
					XtWidgetToApplicationContext(new),
								NULL);
			remove_handler_from_stack(TRUE, new);
		}
	}

	if (curew->error.is_current != newew->error.is_current) {
		if (newew->error.is_current) {
		}
	}

	return False;
}


static void
Destroy(w)
Widget w;
{
	remove_handler_from_stack(TRUE, w);
	remove_handler_from_stack(FALSE, w);
}

/*********** Public Functions **************/

void
XukcGiveErrorMessage(ew, message, append)
UKCErrorWidget ew;
String message;
Boolean append;
{
	if (!append)
		clear_messages(ew);
	update_error_text(ew, message);
}
