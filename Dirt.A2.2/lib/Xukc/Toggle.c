#ifndef lint
/* From: "$Xukc: Toggle.c,v 1.9 91/12/17 10:02:01 rlh2 Rel $"; */
static char sccsid[] = "@(#)Toggle.c	1.2 (UKC) 5/10/92";
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

/**********************************************************************
 *        UKCToggle Widget - subclass of ukcCommandWidgetClass        *
 **********************************************************************/

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/CharSet.h>
#include "ToggleP.h"

#define RESET_STATE 0
#define IGNORE_TOGGLE 1

static void Do_Toggle();
static void okay2toggle();

static void Redisplay();
static void Destroy();
static void Initialize();
static Boolean SetValues();
static XukcUpdateBitmapProc UpdateBitmap();

static String stop_calls[] = { "no_call" };

/* toggle() can have 2 of 4 parameters ("set" | "unset") & "no_call" */

static char defaultTranslations[] =
       "<Btn1Up>: toggle() \n\
	<EnterWindow>: okay2toggle() highlight() \n\
	Button1<LeaveWindow>: okay2toggle(no) unhighlight()\n\
	<LeaveWindow>: unhighlight()\n\
	<Btn2Down>: label_edit(on)\n\
	<Btn2Motion>: position_bar()\n\
	:<Key>Delete: delete_char()\n\
	:<Key>BackSpace: delete_char()\n\
	:<Key>Return: label_update() label_edit(off)\n\
	:<Key>Linefeed: label_update() label_edit(off)\n\
	:<Key>Escape: undo_edit()\n\
	:Ctrl<Key>h: position_bar(-1)\n\
	:Ctrl<Key>j: position_bar(beginning)\n\
	:Ctrl<Key>k: position_bar(end)\n\
	:Ctrl<Key>l: position_bar(+1)\n\
	:Ctrl<Key>u: delete_char(all)\n\
	<Key>: insert_char()\n\
";

static XtActionsRec actionsList[] = {
	{ "toggle", Do_Toggle },
	{ "okay2toggle", okay2toggle }
};

static XtResource resources[] = {
#define offset(field) XtOffset(UKCToggleWidget, toggle.field)
  { XtNtoggleAddress, XtCToggleAddress, XtRToggleAddress, sizeof(Boolean *),
    offset(toggle_address), XtRToggleAddress, NULL},
  { XtNstate, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(state), XtRImmediate, (XtPointer)FALSE },
  { XtNradioToggle, XtCWidget, XtRWidget, sizeof(Widget),
    offset(next_toggle), XtRImmediate, (XtPointer)NULL},
  { XtNradioValue, XtCRadioValue, XtRInt, sizeof(int),
    offset(radio_value), XtRImmediate, (XtPointer)0 },
  { XtNinformWhenOff, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(inform_when_off), XtRImmediate, (XtPointer)FALSE },
  { XtNonLeft, XtCOnLeft, XtRBoolean, sizeof(Boolean),
    offset(on_left), XtRImmediate, (XtPointer)TRUE },
  { XtNonBitmap, XtCOnBitmap, XtRBitmap, sizeof(Pixmap),
    offset(on_pixmap), XtRImmediate, (XtPointer)None },
  { XtNoffBitmap, XtCOffBitmap, XtRBitmap, sizeof(Pixmap),
    offset(off_pixmap), XtRImmediate, (XtPointer)None },
  { XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
    XtOffset(UKCToggleWidget, simple.cursor), XtRString, "hand1" },
#undef offset
};

/* Full Class Record Constant */

#define SuperClass ((UKCCommandWidgetClass)&ukcCommandClassRec)

UKCToggleClassRec ukcToggleClassRec = {
  {
  /* core_class fields  */	
    /* superclass	  */	(WidgetClass) SuperClass,
    /* class_name	  */	"UKCToggle",
    /* widget_size	  */	sizeof(UKCToggleRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	XtInheritRealize,
    /* actions		  */	actionsList,
    /* num_actions	  */	XtNumber(actionsList),
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	XtInheritResize,
    /* expose		  */	Redisplay,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	XtInheritAcceptFocus,
    /* version		  */	XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table		  */	defaultTranslations,
    /* query_geometry	  */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL
  },{ /* simple class */
    /* no class fields */	XtInheritChangeSensitive
  },{ /* UKCLabel class */
    /* update_bitmap */		(XukcUpdateBitmapProc)UpdateBitmap
  },{ /* UKCCommand class */
    /* no class fields */	0
  },{ /* UKCToggle class */
    /* no class fields */	0
  }
};

WidgetClass ukcToggleWidgetClass = (WidgetClass)&ukcToggleClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void Redisplay(w, event, region)
Widget w;
XEvent *event;              /* unused */
Region region;              /* unused */
{
	UKCToggleWidget tw = (UKCToggleWidget)w;

	if (tw->toggle.toggle_address != NULL)
		tw->toggle.state = tw->command.set
				 = (*tw->toggle.toggle_address);

	tw->label.normal_GC = tw->command.normal_GC;
	if (tw->toggle.state) {
		if (tw->toggle.on_pixmap == None)
			tw->label.normal_GC = tw->command.inverse_GC;
		else
			tw->command.set = FALSE;
	}

	(*SuperClass->core_class.expose)(w, event, region);
}


/* Toggle Actions */

static void
add_radio_button(tw)
UKCToggleWidget tw;
{
   UKCToggleWidget tmp;
	
	/* we are being connected to another toggle .. so we become a
	 * radio button.
	 */
   if (tw->toggle.next_toggle != NULL) {
      if (!XtIsSubclass(tw->toggle.next_toggle, ukcToggleWidgetClass)) {
		XtWarning("Non ukcToggleWidget given as XtNradioToggle resource");
	return;
      }	

      tmp = (UKCToggleWidget)(tw->toggle.next_toggle);
      if (tmp->toggle.prev_toggle != NULL)
	  ((UKCToggleWidget)(tmp->toggle.prev_toggle))->toggle.next_toggle
		= (Widget)tw;
      tw->toggle.prev_toggle = tmp->toggle.prev_toggle;
      tmp->toggle.prev_toggle = (Widget)tw;
   }
}


static void
remove_radio_button(tw)
UKCToggleWidget tw;
{
	if (tw->toggle.next_toggle != NULL)
	      ((UKCToggleWidget)(tw->toggle.next_toggle))
			->toggle.prev_toggle = tw->toggle.prev_toggle;
	if (tw->toggle.prev_toggle != NULL)
	      ((UKCToggleWidget)(tw->toggle.prev_toggle))
			->toggle.next_toggle = tw->toggle.next_toggle;
}


static XukcUpdateBitmapProc
UpdateBitmap(w, new_bitmap, is_left)
Widget w;
Pixmap new_bitmap;
Boolean is_left;
{
	UKCToggleWidget tw = (UKCToggleWidget)w;

	tw->command.set = FALSE;
	tw->label.left_bitmap = tw->label.right_bitmap = None;
	(*SuperClass->label_class.update_bitmap)(w, new_bitmap, is_left);
}


static Boolean
set_bitmap(tw)
UKCToggleWidget tw;
{
#define MyClass ((UKCToggleWidgetClass)ukcToggleWidgetClass)

	if (tw->toggle.state) {
		if (tw->toggle.on_pixmap != None) {
		    (*MyClass->label_class.update_bitmap)
			 ((Widget)tw, tw->toggle.on_pixmap, tw->toggle.on_left);
		} else
			return FALSE;
	} else {
		if (tw->toggle.off_pixmap != None) {
		    (*MyClass->label_class.update_bitmap)
			((Widget)tw, tw->toggle.off_pixmap, tw->toggle.on_left);
		} else
			return FALSE;
	}
	return TRUE;
}


static void
toggle(tw, original_state, call_callbacks, event)
UKCToggleWidget tw;
Boolean original_state, call_callbacks;
XEvent *event;
{
	if (original_state != tw->toggle.state) {
		if (tw->toggle.toggle_address != NULL)
			(*tw->toggle.toggle_address) = tw->toggle.state;
		tw->command.set = tw->toggle.state;
		if (!set_bitmap(tw) && XtIsRealized((Widget)tw))
			Redisplay((Widget)tw, event, tw->command.set_region);

		/* call the callbacks on the toggle widget's callback list
		 * if we have been instructed to do so.
		 * If the toggle widget is a radio button then pass
		 * the associated radioValue (as "call_data") if the button
		 * has been turned on.  If the radio button is being turned
		 * off and it has set the informWhenOff resource 
		 * then pass 0 as the "call_data".
		 */
		if (call_callbacks)
			XtCallCallbacks((Widget)tw, XtNcallback,
				tw->toggle.next_toggle != NULL ||
				tw->toggle.prev_toggle != NULL ?
					(tw->toggle.state ?
					    (XtPointer)tw->toggle.radio_value
					    : (XtPointer)0)
					: (XtPointer)tw->toggle.state);
	} else {
		if (tw->toggle.state &&
		    tw->toggle.on_pixmap != None)
			/* unhighlight getting things wrong */
			tw->command.set = FALSE;

	}
}


static void
okay2toggle(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	/* if a parameter is given then it is not okay to do a toggle
	 * and so the next call to Do_Toggle() should be ignored.
	 *
	 * The "ignore_toggle" flag in w->toggle.internal_state is set if
	 * the next call to Do_Toggle() should be ignored otherwise it
	 * it is reset.
	 */
	UKCToggleWidget tw = (UKCToggleWidget)w;

	if (*num_params > 0)
		tw->toggle.internal_state |= IGNORE_TOGGLE;
	else
		tw->toggle.internal_state &= ~IGNORE_TOGGLE;
}


static void
Do_Toggle(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCToggleWidget tw = (UKCToggleWidget)w;
	Boolean set_used = FALSE;
	Boolean unset_used = FALSE;
	Boolean call_used = FALSE;
	Cardinal i;
	Boolean original_state = tw->toggle.state;

	if (tw->toggle.internal_state & IGNORE_TOGGLE) {
		tw->toggle.internal_state &= ~IGNORE_TOGGLE;
		return;
	}

	for (i = *num_params; i > 0; i--) {
		XmuCopyISOLatin1Lowered(params[i-1], params[i-1]);
		if (!set_used && strcmp(params[i-1], "set") == 0) {
			set_used = TRUE;
			continue;
		}

		if (!unset_used && strcmp(params[i-1], "unset") == 0) {
			unset_used = TRUE;
			continue;
		}

		if (!call_used && strcmp(params[i-1], "no_call") == 0)
			call_used = TRUE;
	}

	if (set_used && unset_used)
		XtWarning("Cannot use set and unset in the same toggle() call; using set");

	if (set_used)
		tw->toggle.state = TRUE;
	else {
		if (unset_used)
			tw->toggle.state = FALSE;
		else
			tw->toggle.state = !tw->toggle.state;
	}

	/* Turn off any other radio toggle buttons */
	if (tw->toggle.next_toggle != NULL
	    || tw->toggle.prev_toggle != NULL) {
		UKCToggleWidget temp;
		
		tw->toggle.state = TRUE;
		temp = (UKCToggleWidget)tw->toggle.next_toggle;
		for (; temp != NULL;) {
			if (temp->toggle.state) {
				temp->toggle.state = FALSE;
				toggle(temp, TRUE,
					temp->toggle.inform_when_off, event);
			}
			temp = (UKCToggleWidget)temp->toggle.next_toggle;
		}

		temp = (UKCToggleWidget)tw->toggle.prev_toggle;
		for (; temp != NULL;) {
			if (temp->toggle.state) {
				temp->toggle.state = FALSE;
				toggle(temp, TRUE, 
					temp->toggle.inform_when_off, event);
			}
			temp = (UKCToggleWidget)temp->toggle.prev_toggle;
		}
	}

	tw->command.set = tw->toggle.state;
	toggle(tw, original_state, !call_used, event);
}


static void
Initialize(request, new)
Widget request, new;
{
	UKCToggleWidget tw = (UKCToggleWidget)request;
	UKCToggleWidget nw = (UKCToggleWidget)new;

	nw->toggle.internal_state = RESET_STATE;
	nw->toggle.prev_toggle = NULL;
	if (tw->toggle.toggle_address != NULL)
		nw->toggle.state = nw->command.set
				 = (*tw->toggle.toggle_address);
	nw->toggle.toggle_address = tw->toggle.toggle_address;
	add_radio_button(nw);

	if (nw->toggle.state) {
	    if (nw->toggle.on_pixmap != None) {
		if (tw->core.width == 0)
			nw->core.width = 0;
		if (tw->core.height == 0)
			nw->core.height = 0;

		(void)set_bitmap(nw);

		/* force the setting of radio buttons */
		nw->toggle.state = nw->command.set = FALSE;
		XukcSetToggle(nw, TRUE);
	    }
	} else {
	    if (nw->toggle.off_pixmap != None) {
		if (tw->core.width == 0)
			nw->core.width = 0;
		if (tw->core.height == 0)
			nw->core.height = 0;

		(void)set_bitmap(nw);
	    }
	}
}


/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
	UKCToggleWidget twc = (UKCToggleWidget)current;
	UKCToggleWidget twn = (UKCToggleWidget)new;
	Boolean redisplay = FALSE;

	if (twn->toggle.next_toggle != twc->toggle.next_toggle) {
		remove_radio_button(twc);
		add_radio_button(twn);
	}
	if (twn->toggle.toggle_address != twc->toggle.toggle_address) {
		if (twn->toggle.toggle_address != NULL)
			twn->toggle.state = (*twn->toggle.toggle_address);
	} else {
		if (twn->toggle.toggle_address != NULL) {
			if (*twn->toggle.toggle_address
			    != *twc->toggle.toggle_address)
				twn->toggle.state =
					(*twn->toggle.toggle_address);
		}
	}
	if (twn->toggle.state != twc->toggle.state)
		redisplay = TRUE;

	if (redisplay || twn->toggle.on_pixmap != twc->toggle.on_pixmap
	    || twn->toggle.off_pixmap != twc->toggle.off_pixmap
	    || twn->toggle.on_left != twc->toggle.on_left) {
		(void)set_bitmap(twn);
		redisplay = TRUE;
	}

	if (redisplay) {
		if (twn->toggle.off_pixmap != None &&
		    twn->toggle.on_pixmap != None)
			twn->label.normal_GC = twn->command.normal_GC;
		else
			twn->label.normal_GC = twn->command.set ?
				twn->command.inverse_GC :
				twn->command.normal_GC;
	}
	return (redisplay);
}


static void
Destroy(w)
Widget w;
{
	UKCToggleWidget tw = (UKCToggleWidget)w;

	remove_radio_button(tw);
}

/*******************/
/* public routines */
/*******************/

#define CHECK_FOR_TOGGLE(w, routine) \
	if (!XtIsSubclass(w, ukcToggleWidgetClass)) { \
		XtWarning("Non UKCToggleWidget passed to routine()"); \
		return; \
	}

/* toggle the current state of the button ... also changes the contents of
 * the toggleAddress resource.
 */
void
XukcDoToggle(w, do_calls)
Widget w;
Boolean do_calls;
{
	Cardinal n_params = do_calls ? 0 : 1;

	CHECK_FOR_TOGGLE(w, XukcDoToggle);
	Do_Toggle(w, (XEvent *)NULL, (String *)(do_calls ? NULL : stop_calls),
		  &n_params);
}

/* Sets the state of the button .. unsets any other toggles linked as radio
 * buttons. */
void
XukcSetToggle(w, do_calls)
Widget w;
Boolean do_calls;
{
	UKCToggleWidget tw = (UKCToggleWidget)w;
	Cardinal n_params = do_calls ? 0 : 1;

	CHECK_FOR_TOGGLE(w, XukcSetToggle);
	if (!tw->toggle.state)
		Do_Toggle(w, (XEvent *)NULL,
			  (String *)(do_calls ? NULL : stop_calls), &n_params);
}

void
XukcUnsetToggle(w, do_calls)
Widget w;
Boolean do_calls;
{
	UKCToggleWidget tw = (UKCToggleWidget)w;
	Cardinal n_params = do_calls ? 0 : 1;

	CHECK_FOR_TOGGLE(w, XukcUnsetToggle);
	if (tw->toggle.state)
		Do_Toggle(w, (XEvent *)NULL,
			  (String *)(do_calls ? NULL : stop_calls), &n_params);
} 


void
XukcAddRadioToggle(w1, w2)
Widget w1, w2;
{
	Arg args[1];

	XtSetArg(args[0], XtNradioToggle, w2);
	XtSetValues(w1, (ArgList)args, 1);
}


void
XukcRemoveRadioToggle(w)
Widget w;
{
	Arg args[1];

	XtSetArg(args[0], XtNradioToggle, NULL);
	XtSetValues(w, (ArgList)args, 1);
}


/* returns id. to the radio toggle button currently set */
/* NULL is returned if none of them are set */
UKCToggleWidget
XukcGetRadioSet(w)
Widget w;
{
	UKCToggleWidget tw = (UKCToggleWidget)w;

	if (!XtIsSubclass(w, ukcToggleWidgetClass)) { 
		XtWarning("Non UKCToggleWidget passed to XukcGetRadioSet()");
		return (NULL); 
	}

	if (tw->toggle.state)
		return (tw);

	if (tw->toggle.next_toggle != NULL
	    || tw->toggle.prev_toggle != NULL) {
		UKCToggleWidget temp;
		
		temp = (UKCToggleWidget)tw->toggle.next_toggle;
		for (; temp != NULL;) {
			if (temp->toggle.state)
				return (temp);
			temp = (UKCToggleWidget)temp->toggle.next_toggle;
		}
		temp = (UKCToggleWidget)tw->toggle.prev_toggle;
		for (; temp != NULL;) {
			if (temp->toggle.state)
				return (temp);
			temp = (UKCToggleWidget)temp->toggle.prev_toggle;
		}
	}
	return (NULL);
}
