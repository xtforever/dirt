#ifndef lint
static char sccsid[] = "@(#)widget_examples.c	1.1 (UKC) 5/10/92";
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
#define TRANSLATIONS_KLUDGE 0

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/memory.h>
#include <X11/Xukc/Med3Menu.h>
#include "dirt_structures.h"
#include "dirt_bindings.h"
#include "widget_examples.h"
#include "athena_examples.h"

extern void UpdatePrecreateChoices();
extern ObjectPtr GetHeadObject();
extern void XukcDirtAddLayoutConnections();
extern Widget WcFullNameToWidget();
extern void DrainUIEventQueue();
extern XtAppContext GetUIContext();
extern XtAppContext GetDirtContext();
extern void XukcChangeLabel();
extern String GetDisplayedClassName();
extern Cardinal FindClass();
extern void WcCreateNamedChildren();

extern Widget WI_template_building_shell;

#define CHOSEN (1 << 0)
#define DROPPED (1 << 1)
#define SHOW_FRAME (1 << 2)
#define START_DRAG (1 << 3)
#define DRAGGING (1 << 4)

static short chosen_flag = CHOSEN;

#define ATHENA_SET "Athena Widget Set"
#define ATHENA_INDEX 0
#define UKC_SET "UKC Widget Set"
#define UKC_INDEX 1
#define HP_SET "HP Widget Set"
#define HP_INDEX 2
#define MOTIF_SET "Motif Widget Set"
#define MOTIF_INDEX 3

struct _example_sets {
	XukcReplaceMenuStruct menu_entry;
	struct _examples *class_list;
	Cardinal num_examples;
	Widget *examples_popup;
	Widget *current_class;
	Widget *examples_hide;
};

#define TARGET_SIZE 5

static GC invert_gc = NULL;
static GC dashed_gc = NULL;
static XSegment lines[20];
static Dimension current_width, current_height, current_border_width;
static Position current_x, current_y;
static Widget current_widget = NULL;
static Widget menu_widget = NULL;
static Boolean started_drag_move = FALSE;

static struct _example_sets sets[] = {
 { { ATHENA_SET, ATHENA_INDEX }, athena_ex, XtNumber(athena_ex),
   &WI_athena_examples, &WI_athena_current_class, &WI_athena_examples_hide },
#if 0
 { { UKC_SET, UKC_INDEX }, ukc_ex, XtNumber(ukc_ex),
   &WI_ukc_examples, &WI_ukc_current_class, &WI_ukc_examples_hide },
 { { HP_SET, HP_INDEX }, hp_ex, XtNumber(hp_ex),
   &WI_hp_examples, &WI_hp_current_class, &WI_hp_examples_hide},
 { { MOTIF_SET, MOTIF_INDEX }, motif_ex, XtNumber(motif_ex),
   &WI_motif_examples, &WI_motif_current_class, &WI_motif_examples_hide },
#endif
};

static XukcReplaceMenuStruct *replace_menu[] = {
	&sets[ATHENA_INDEX].menu_entry,
#if 0
	&sets[UKC_INDEX].menu_entry,
	&sets[HP_INDEX].menu_entry,
	&sets[MOTIF_INDEX].menu_entry
#endif
};

static void class_select_highlight();
static void class_select_unhighlight();
static void class_select_chosen();
static void class_select_drag();
static void class_select_drop();

static XtActionsRec class_select_actions[] = {
	{ "dirtACTClassSelectChosen", class_select_chosen },
	{ "dirtACTClassSelectDrag", class_select_drag },
	{ "dirtACTClassSelectDrop", class_select_drop },
};


static void
make_gcs(w)
Widget w;
{
	if (invert_gc == NULL) { /* create the GC that inverts the colours */
		XGCValues gcvalues;

		gcvalues.function = GXinvert;
		gcvalues.line_width = 0;
		gcvalues.subwindow_mode = IncludeInferiors;
		invert_gc = XtGetGC(w, GCFunction | GCLineWidth |
					GCSubwindowMode, &gcvalues);

		gcvalues.line_style = LineOnOffDash;
		dashed_gc = XtGetGC(w, GCFunction | GCLineWidth | GCLineStyle |
					GCSubwindowMode, &gcvalues);
	}
}


static void
make_target(w, p, ax, ay, bx, by)
Widget w, p;
Position ax, ay, bx, by;
{
	Position x1, y1, x2, y2;

	if (p == NULL) {
		/* we are drawing into the root window
		 * so translate the coords */
		XtTranslateCoords(w, ax, ay, &x1, &y1);
		XtTranslateCoords(w, bx, by, &x2, &y2);
	} else {
		Dimension bw = 0;

		/* we are drawing into a particular parent */
		x1 = y1 = 0;
		while (w != p) {
			XtVaGetValues(w, XtNx, &x2, XtNy, &y2, NULL);
			x1 += x2 + bw;
			y1 += y2 + bw;
			XtVaGetValues(w, XtNborderWidth, &bw, NULL);
			w = XtParent(w);
		};
		x2 = bx + x1;
		y2 = by + y1;
		x1 += ax;
		y1 += ay;
	}

	/* x1, y1 to x2, y2 defines a box (0,0)-(a,b) */
	/* top left corner */
	lines[0].x1 = x1; lines[0].y1 = y1 + 1;
	lines[0].x2 = x1; lines[0].y2 = y1 + TARGET_SIZE;
	lines[1].x1 = x1; lines[1].y1 = y1;
	lines[1].x2 = x1 + TARGET_SIZE; lines[1].y2 = y1;
	lines[2].x1 = x1 - 1; lines[2].y1 = y1 - 1;
	lines[2].x2 = x1 + TARGET_SIZE + 1; lines[2].y2 = y1 - 1;
	lines[3].x1 = x1 - 1; lines[3].y1 = y1;
	lines[3].x2 = x1 - 1; lines[3].y2 = y1 + TARGET_SIZE;


	/* top right corner */
	lines[4].x1 = x2 - TARGET_SIZE; lines[4].y1 = y1;
	lines[4].x2 = x2 - 1; lines[4].y2 = y1;
	lines[5].x1 = x2; lines[5].y1 = y1 + TARGET_SIZE;
	lines[5].x2 = x2; lines[5].y2 = y1;
	lines[6].x1 = x2 - TARGET_SIZE; lines[6].y1 = y1 - 1;
	lines[6].x2 = x2 + 1; lines[6].y2 = y1 - 1;
	lines[7].x1 = x2 + 1; lines[7].y1 = y1 + TARGET_SIZE;
	lines[7].x2 = x2 + 1; lines[7].y2 = y1;

	/* bottom right corner */
	lines[8].x1 = x2 - TARGET_SIZE; lines[8].y1 = y2;
	lines[8].x2 = x2 - 1; lines[8].y2 = y2;
	lines[9].x1 = x2; lines[9].y1 = y2 - TARGET_SIZE;
	lines[9].x2 = x2; lines[9].y2 = y2;
	lines[10].x1 = x2 - TARGET_SIZE; lines[10].y1 = y2 + 1;
	lines[10].x2 = x2; lines[10].y2 = y2 + 1;
	lines[11].x1 = x2 + 1; lines[11].y1 = y2 - TARGET_SIZE;
	lines[11].x2 = x2 + 1; lines[11].y2 = y2 + 1;

	/* bottom left corner */
	lines[12].x1 = x1; lines[12].y1 = y2;
	lines[12].x2 = x1; lines[12].y2 = y2 - TARGET_SIZE;
	lines[13].x1 = x1 + 1; lines[13].y1 = y2;
	lines[13].x2 = x1 + TARGET_SIZE; lines[13].y2 = y2;
	lines[14].x1 = x1 - 1; lines[14].y1 = y2 + 1;
	lines[14].x2 = x1 - 1; lines[14].y2 = y2 - TARGET_SIZE;
	lines[15].x1 = x1; lines[15].y1 = y2 + 1;
	lines[15].x2 = x1 + TARGET_SIZE; lines[15].y2 = y2 + 1;

	/* "chosen" feedback lines */
	lines[16].x1 = x1 + TARGET_SIZE + 1; lines[16].y1 = y1;
	lines[16].x2 = x2 - TARGET_SIZE - 1; lines[16].y2 = y1;
	lines[17].x1 = x2; lines[17].y1 = y1 + TARGET_SIZE + 1;
	lines[17].x2 = x2; lines[17].y2 = y2 - TARGET_SIZE - 1;
	lines[18].x1 = x1 + TARGET_SIZE + 1; lines[18].y1 = y2;
	lines[18].x2 = x2 - TARGET_SIZE - 1; lines[18].y2 = y2;
	lines[19].x1 = x1; lines[19].y1 = y1 + TARGET_SIZE + 1;
	lines[19].x2 = x1; lines[19].y2 = y2 - TARGET_SIZE - 1;
}


static void
draw_chosen(w, x, y, width, height, border_width)
Widget w;
Position x, y;
Dimension width, height, border_width;
{
	make_gcs(w);
	make_target(w, (Widget)NULL, x, y,
			(Position)(x + width + border_width),
			(Position)(y + height + border_width));
	XDrawSegments(XtDisplayOfObject(w), XtScreenOfObject(w)->root,
			invert_gc, lines, 16);
	XDrawSegments(XtDisplayOfObject(w), XtScreenOfObject(w)->root,
			dashed_gc, lines + 16, 4);
	XFlush(XtDisplayOfObject(w));
}


static void
draw_target(w)
Widget w;
{
	Dimension width, height, border_width;
	Widget p;

	make_gcs(w);

	XtVaGetValues(w, XtNwidth, &width,
		 	 XtNheight, &height,
			 XtNborderWidth, &border_width, NULL);
	p = w;
	while (XtParent(XtParent(p)) != NULL)
		p = XtParent(p);

	make_target(w, p, (Position)(-border_width), (Position)(-border_width),
			(Position)width, (Position)height);
	XDrawSegments(XtDisplayOfObject(p), XtWindow(p),
			invert_gc, lines, 16);
	XFlush(XtDisplayOfObject(p));
}


static void
update_chosen_class_label(set, w)
struct _example_sets *set;
Widget w;
{
	char tmp[100];

	if (w != NULL) {
		(void)sprintf(tmp, ": %s",
				GetDisplayedClassName(FindClass(XtClass(w))));
		XukcChangeLabel(*(set->current_class), tmp, TRUE);
	} else
		XukcChangeLabel(*(set->current_class), "", TRUE);
}


/* ARGSUSED */
static void
left_examples(w, client_data, event, continue_to_dispatch)
Widget w;
XtPointer client_data;
XEvent *event;
Boolean *continue_to_dispatch;
{
	struct _example_sets *set = (struct _example_sets *)client_data;

	update_chosen_class_label(set, (Widget)NULL);

	if (current_widget != NULL)
		draw_target(current_widget);

	current_widget = NULL;
}


/* ARGSUSED */
static void
entered_widget(w, client_data, event, continue_to_dispatch)
Widget w;
XtPointer client_data;
XEvent *event;
Boolean *continue_to_dispatch;
{
	struct _example_sets *set = (struct _example_sets *)client_data;

	if (current_widget != NULL)
		draw_target(current_widget);

	current_widget = w;
	update_chosen_class_label(set, w);
	draw_target(w);
}


static void
class_select_chosen(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	if (chosen_flag != SHOW_FRAME)
		return;

	XUngrabServer(XtDisplayOfObject(w));
	XSync(XtDisplayOfObject(w), FALSE);
	DrainUIEventQueue();
	UpdatePrecreateChoices(XtClass(w), -9999, -9999, -9999, -9999);

#ifndef TRANSLATIONS_KLUDGE
	draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
#endif /* !TRANSLATIONS_KLUDGE */
	XFlush(XtDisplayOfObject(w));
	chosen_flag = CHOSEN;
}


static void
class_select_drag(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	if (*num_params == 1) { /* get ready to drag window */
		XGrabServer(XtDisplayOfObject(w));
		XSync(XtDisplayOfObject(w), FALSE);
		XtVaGetValues(w, XtNwidth, &current_width,
				 XtNheight, &current_height,
				 XtNborderWidth, &current_border_width,
				 NULL);
		current_x = -current_border_width;
		current_y = -current_border_width;
		draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
		chosen_flag = SHOW_FRAME;
	} else if (*num_params == 2) { /* start dragging */
		draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
		current_x = event->xcrossing.x - current_width/2;
		current_y = event->xcrossing.y - current_height/2;
		draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
		chosen_flag = START_DRAG;
	} else {
		draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
		current_x = event->xmotion.x - current_width/2;
		current_y = event->xmotion.y - current_height/2;
		draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
		chosen_flag = DRAGGING;
	}
}


static void
class_select_drop(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	Position x1, y1, x2, y2;

	if (chosen_flag & ~(SHOW_FRAME | START_DRAG | DRAGGING))
		return;

	draw_chosen(w, current_x, current_y,
			current_width, current_height, current_border_width);
	XUngrabServer(XtDisplayOfObject(w));
	XSync(XtDisplayOfObject(w), FALSE);

	if (chosen_flag & SHOW_FRAME)
		return;

	XtTranslateCoords(w, current_x, current_y, &x1, &y1);
	XtTranslateCoords(w, current_x + current_width,
			     current_y + current_height, &x2, &y2);
	UpdatePrecreateChoices(XtClass(w), x1, y1, x2, y2);
	chosen_flag = DROPPED;
}


#ifdef TRANSLATIONS_KLUDGE /* ||| See Kludge: below ||| */
static void
kludge_release(w, client_data, event, continue_to_dispatch)
Widget w;
XtPointer client_data;
XEvent *event;
Boolean *continue_to_dispatch;
{
	class_select_drop(w, event, (String *)NULL, (Cardinal *)NULL);
}
#endif /* ||| */


static void
create_popup_menu(menu)
Widget menu;
{
	Cardinal i;

	if (!XukcReplaceMenu((UKCMed3MenuWidget)menu, EXAMPLES, replace_menu,
	    XtNumber(replace_menu), TRUE))
		XtWarning("Internal Error: Couldn't create examples menu");

	for (i = 0; i < XtNumber(sets); i++) {
		XtPopdownID pd_id = XtNew(XtPopdownIDRec);

		XukcBindButton((UKCMed3MenuWidget)menu,
					sets[i].menu_entry.button_value,
			XtCallbackNone, (XtPointer)*sets[i].examples_popup);
		pd_id->shell_widget = *sets[i].examples_popup;
		pd_id->enable_widget = XukcFindButton(menu,
					sets[i].menu_entry.button_value);
		XtAddCallback(*sets[i].examples_hide, XtNcallback,
				XtCallbackPopdown, (XtPointer)pd_id);
	}
}
	

static void
initialize_class_selection(set)
Cardinal set;
{
	static String select_translations =
		"Shift<Btn1Down>(2): dirtACTClassSelectChosen() \n\
		 Shift<Btn1Up>: dirtACTClassSelectDrop() \n\
		 Shift<Btn1Down>: dirtACTClassSelectDrag(showframe) \n\
		 Shift Button1<LeaveWindow>: dirtACTClassSelectDrag(start dragging) \n\
		 Shift<Btn1Motion>: dirtACTClassSelectDrag()";
	static XtTranslations select_trans = NULL;
	struct _example_sets *this_set;
	Widget w;
	Cardinal i;

	this_set = &sets[set];

	if (select_trans == NULL) /* "I shall zay zhis onlee wunce" */
		select_trans = XtParseTranslationTable(select_translations);

	/* for each different widget class in this set which has an
	 * example add event handlers which allow the class to be chosen
	 * by the user simply clicking on the displayed widget .. or
	 * dragging a copy of it to his application.
	 */
	for (i = 0; i < this_set->num_examples; i++) {
		w = *(this_set->class_list[i].choice);
		XtOverrideTranslations(w, select_trans);
#ifdef TRANSLATIONS_KLUDGE
      /* ||| Kludge: I cannot get the above translations right
       *	     dirtACTClassSelectDrop() only gets called after a
       *	     dirtACTClassSelectChosen() or dirtACTClassSelectDrag()
       *	     is called.
       * |||
       */
		XtAddEventHandler(w, ButtonReleaseMask, 0, kludge_release,
					(XtPointer)NULL);
#endif /* TRANSLATIONS_KLUDGE */
		XtAddEventHandler(w, EnterWindowMask, 0, entered_widget,
					(XtPointer)this_set);
	}
	XtAddEventHandler(*(this_set->examples_popup), FocusChangeMask, 0,
				left_examples, (XtPointer)this_set);
	XtAddEventHandler(XtParent(*(this_set->examples_hide)),
				EnterWindowMask, 0,
				left_examples, (XtPointer)this_set);
}

/***** Module Exported Routines *****/

void
InitializeExamples(menu)
Widget menu;
{
#define BIND_OBJECT(v, n) v = WcFullNameToWidget(display_top, n)
	extern Widget display_top;
	static Boolean initialized = FALSE;

	if (initialized)
		return;
	else
		initialized = TRUE;

	menu_widget = menu;
	XtAppAddActions(XtWidgetToApplicationContext(display_top),
				class_select_actions,
				XtNumber(class_select_actions));
	/* create the Athena examples sheet */
	WcCreateNamedChildren(display_top, "athenaExamples");

	BIND_OBJECT(WI_athena_examples, "*athenaExamples");
	BIND_OBJECT(WI_athena_examples_layout, "*athenaExamplesLayout");
	BIND_OBJECT(WI_athena_examples_title, "*athenaExamplesTitle");
	BIND_OBJECT(WI_athena_examples_box_label, "*athenaExamplesBoxLabel");
	BIND_OBJECT(WI_athena_form_label, "*athenaFormLabel");
	BIND_OBJECT(WI_athena_toggle_label, "*athenaToggleLabel");
	BIND_OBJECT(WI_athena_paned_label, "*athenaPanedLabel");
	BIND_OBJECT(WI_athena_grip_label, "*athenaGripLabel");
	BIND_OBJECT(WI_athena_popup_a_menu, "*athenaPopupAMenu");
	BIND_OBJECT(WI_athena_stripchart_label, "*athenaStripchartLabel");
	BIND_OBJECT(WI_athena_mailbox_label, "*athenaMailboxLabel");
	BIND_OBJECT(WI_athena_viewport_label, "*athenaViewportLabel");
	BIND_OBJECT(WI_athena_current_class, "*athenaCurrentClass");
	BIND_OBJECT(WI_athena_examples_hide, "*athenaExamplesHide");
	BIND_OBJECT(WI_athena_box, "*athenaBox");
	BIND_OBJECT(WI_athena_clock, "*athenaClock");
	BIND_OBJECT(WI_athena_command, "*athenaCommand");
	BIND_OBJECT(WI_athena_dialog_box, "*athenaDialogBox");
	BIND_OBJECT(WI_athena_editable_text, "*athenaEditableText");
	BIND_OBJECT(WI_athena_form, "*athenaForm");
	BIND_OBJECT(WI_athena_label, "*athenaLabel");
	BIND_OBJECT(WI_athena_grip, "*athenaGrip");
	BIND_OBJECT(WI_athena_mailbox, "*athenaMailbox");
	BIND_OBJECT(WI_athena_paned, "*athenaPaned");
	BIND_OBJECT(WI_athena_toggle, "*athenaToggle");
	BIND_OBJECT(WI_athena_logo, "*athenaLogo");
	BIND_OBJECT(WI_athena_scroll_bar, "*athenaScrollBar");
	BIND_OBJECT(WI_athena_strip_chart, "*athenaStripChart");
	BIND_OBJECT(WI_athena_viewport, "*athenaViewport");

	XukcDirtAddLayoutConnections();

	/* initialize each sheet so that classes can be chosen directly
	 * from the examples on the screen. */
	initialize_class_selection(ATHENA_INDEX);

	create_popup_menu(menu);
}
