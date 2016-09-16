#ifndef lint
/* From: Xukc: Mouse.c,v 1.6 91/12/17 10:01:40 rlh2 Rel */
static char sccsid[] = "@(#)Mouse.c	1.2 (UKC) 5/10/92";
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
 *		UKCMouse Hole Widget	rlh2@ukc.ac.uk		      *
 **********************************************************************/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>       /* for XA_CAP_HEIGHT */
#include "MouseP.h"

#define	XtStrlen(s)	((s) ? strlen(s) : 0)
#define NO_TIMER_SET 0
#define NO_CLIENT -1
#define SLOT_INCREMENT 10
#define DEFAULT_RATE 75     /* milliseconds mouse hole update rate */

static XtResource resources[] = {
#define Offset(field) XtOffset(UKCMouseWidget, mouse.field)
	{ XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	  Offset(font), XtRString, XtDefaultFont },
	{ XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	  Offset(foreground), XtRString, XtExtdefaultforeground},
	{ XtNbuttonDown, XtCButtonDown, XtRInt, sizeof(int),
	  Offset(buttons_down), XtRImmediate, (XtPointer)0 },
	{ XtNupdateRate, XtCUpdateRate, XtRInt, sizeof(int),
	  Offset(update_rate), XtRImmediate, (XtPointer)DEFAULT_RATE },
	{ XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
	  XtOffset(UKCMouseWidget, simple.cursor), XtRString, "mouse" },
#undef Offset
};

static void Initialize();
static void Destroy();
static void Resize();
static void Redisplay();
static Boolean SetValues();
static XtGeometryResult QueryGeometry();

static void timeoutDisplay();
static void UpdateMouse();
static void mwWindowEntry();
static void mwWindowLeave();
static void mwSetButton();
static void mwClearButton();
static void ClearMouseNameArea();
static void SetMouseName();
static void SetMouseButton();
static void SetButtons();
static void mouseClientDestroy();

void XukcMouseAddClient();
void XukcMouseRemoveClient();

/* Full Class Record Constant */

UKCMouseClassRec ukcMouseClassRec = {
  {
     /* core_class fields  */	
    /* superclass	  */	(WidgetClass) &simpleClassRec,
    /* class_name	  */	"UKCMouse",
    /* widget_size	  */	sizeof(UKCMouseRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	XtInheritRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	Resize,
    /* expose		  */	Redisplay,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table		  */	NULL,
    /* query_geometry	  */	QueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL
  },
  { /* simple class fields */
    /* change_sensitive */	XtInheritChangeSensitive
  }
};

WidgetClass ukcMouseWidgetClass = (WidgetClass)&ukcMouseClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

#define DEFAULT_WIDTH 190
#define DEFAULT_HEIGHT 60
#define BUTTON_WIDTH_RATIO 15
#define BUTTON_HEIGHT_RATIO 4
#define MH_SEP_SPACE (mw->core.height / BUTTON_WIDTH_RATIO)

/* ARGSUSED */
static void Initialize(request, new)
Widget request, new;
{
	UKCMouseWidget mw = (UKCMouseWidget) new;
	XGCValues val;

#ifdef DEBUG
	printf("Initialize(mouse)\n");
#endif DEBUG

	if (mw->core.width == 0)
		mw->core.width = DEFAULT_WIDTH;
	if (mw->core.height == 0)
		mw->core.height = DEFAULT_HEIGHT;
	val.foreground = mw->mouse.foreground;
	val.font = mw->mouse.font->fid;
	mw->mouse.gc = XtGetGC(new, GCForeground | GCFont, &val);
	mw->mouse.clients = NULL;
	mw->mouse.num_clients = 0;
	mw->mouse.num_slots = 0;
	mw->mouse.current_client = NO_CLIENT;
	mw->mouse.timer = NO_TIMER_SET;
	mw->mouse.blank_mdc.mw = mw;

	Resize(mw);
	XukcMouseAddClient(mw, (Widget)mw, MH_No_Function, MH_No_Function,
				MH_No_Function);
}


static void
Destroy(w)
Widget w;
{
	UKCMouseWidget mw = (UKCMouseWidget)w;
	_XukcMouseClients *mpc;
	Cardinal i;

	if (mw->mouse.timer != NO_TIMER_SET)
		XtRemoveTimeOut(mw->mouse.timer);

	/* remove all event handlers and callbacks registered with clients */
	for (i=mw->mouse.num_clients; i > 0; i--) {
		mpc = mw->mouse.clients[i-1];
		mouseClientDestroy(mpc->widget, (XtPointer)mpc->client_data,
							(XtPointer)NULL);
	}

#ifdef DEBUG
	if (mw->mouse.num_clients > 0) {
		(void)sprintf(mouseerror,
		    "Dying UKCMouseWidget \"%s\" unable to delete all clients",
				mw->core.name);
		XtWarning(mouseerror);
	}
#endif DEBUG

	XtFree((char *)mw->mouse.clients);
	XtReleaseGC(w, mw->mouse.gc);
}


static void
NameLayout(mw, this_client)
UKCMouseWidget mw;
Cardinal this_client;
{
	_XukcMouseClients *mpc;
	Position mid_x, mid_y;
	Cardinal ltw, mtw;
	unsigned long val;
	Boolean ok;
	
	mpc = mw->mouse.clients[this_client];

	ok = XGetFontProperty(mw->mouse.font, XA_CAP_HEIGHT, &val);
	if (mpc->name_lengths[LEFT] > 0)
		ltw = XTextWidth(mw->mouse.font, mpc->left_name,
						mpc->name_lengths[LEFT]);
	else
		ltw = 0;
	if (mpc->name_lengths[MIDDLE] > 0)
		mtw = XTextWidth(mw->mouse.font, mpc->middle_name,
						mpc->name_lengths[MIDDLE]);
	else
		mtw = 0;

	mid_x = mw->core.width / 2;
	mid_y = mw->core.height / 2 + 5;

	mpc->name_pos_x[MIDDLE] = mid_x - (Position)(mtw/2);
	mpc->name_pos_y[MIDDLE] = mw->mouse.boxes[MIDDLE].y -
				    (mw->core.height * 2 / BUTTON_WIDTH_RATIO);

	mpc->name_pos_x[LEFT] = mw->mouse.boxes[LEFT].x
						- MH_SEP_SPACE - ltw;
	if (ok)
		mpc->name_pos_y[LEFT] = mw->mouse.boxes[LEFT].y
			+ (mw->core.height/BUTTON_WIDTH_RATIO)
			+ val/2;
	else
		mpc->name_pos_y[LEFT] = mid_y;
	
	mpc->name_pos_x[RIGHT] = mw->mouse.boxes[RIGHT].x
			+ (mw->core.height * 2 / BUTTON_WIDTH_RATIO)
			+ MH_SEP_SPACE;
	mpc->name_pos_y[RIGHT] = mpc->name_pos_y[LEFT];
}

	
static XtGeometryResult
QueryGeometry(w, request, preferred_return)
Widget w;
XtWidgetGeometry *request, *preferred_return;
{
	XtGeometryMask current = 0;
	XtGeometryMask dont_care = 0;

	preferred_return->width = DEFAULT_WIDTH;
	preferred_return->height = DEFAULT_HEIGHT;
	preferred_return->border_width = 1;
	preferred_return->x = w->core.x;
	preferred_return->y = w->core.y;

	dont_care = (request->request_mode & (CWSibling | CWStackMode));
	preferred_return->request_mode = CWWidth | CWHeight | CWX | CWY;

	if (request->request_mode & CWWidth) {
		if (request->width == w->core.width)
			current |= CWWidth;
	}
	if (request->request_mode & CWHeight) {
		if (request->height == w->core.height)
			current |= CWHeight;
	}
	if (request->request_mode & CWX) {
		if (request->x == preferred_return->x)
			current |= CWX;
	}
	if (request->request_mode & CWY) {
		if (request->y == preferred_return->y)
			current |= CWY;
	}
	if ((current & dont_care) == request->request_mode)
		return (XtGeometryYes);

	if ((preferred_return->request_mode & dont_care)
	    != request->request_mode)
		return (XtGeometryAlmost);

	if ((preferred_return->x == w->core.x) &&
	    (preferred_return->y == w->core.y) &&
	    (preferred_return->width == w->core.width) &&
	    (preferred_return->height == w->core.height))
		return (XtGeometryNo);

	return (XtGeometryAlmost);
}


static void Resize(mw)
UKCMouseWidget mw;
{
	Position mid_x, mid_y;
	Cardinal i;

	for (i=0; i < 3; i++) {
		mw->mouse.boxes[i].width = mw->core.height * 2
							/ BUTTON_WIDTH_RATIO;
		mw->mouse.boxes[i].height = mw->core.height * 2
							/ BUTTON_HEIGHT_RATIO;
	}

	mid_x = mw->core.width / 2;
	mid_y = mw->core.height / 2 + 5;

	mw->mouse.boxes[MIDDLE].x = mid_x -
				(mw->core.height/BUTTON_WIDTH_RATIO);
	mw->mouse.boxes[MIDDLE].y = mid_y -
				(mw->core.height/BUTTON_HEIGHT_RATIO);
	mw->mouse.boxes[LEFT].x = mid_x -
		(mw->core.height * 3 / BUTTON_WIDTH_RATIO) - MH_SEP_SPACE;
	mw->mouse.boxes[LEFT].y = mw->mouse.boxes[MIDDLE].y;
	mw->mouse.boxes[RIGHT].x = mid_x +
		(mw->core.height/BUTTON_WIDTH_RATIO) + MH_SEP_SPACE;
	mw->mouse.boxes[RIGHT].y = mw->mouse.boxes[MIDDLE].y;

	for(i=0; i < mw->mouse.num_clients; i++)
		NameLayout(mw, i);
}


/* ARGSUSED */
static void
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
	UKCMouseWidget mw = (UKCMouseWidget) w;

	XDrawRectangles(XtDisplay(w), XtWindow(w), mw->mouse.gc,
						mw->mouse.boxes, 3);
	SetMouseName(mw, LEFT);
	SetMouseName(mw, MIDDLE);
	SetMouseName(mw, RIGHT);
	SetButtons(mw);
}


/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
	UKCMouseWidget cw = (UKCMouseWidget) current;
	UKCMouseWidget rw = (UKCMouseWidget) request;
	UKCMouseWidget nw = (UKCMouseWidget) new;
	XGCValues val;
	Boolean redisplay = FALSE;

	if (cw->mouse.buttons_down != rw->mouse.buttons_down) {
		if (rw->mouse.buttons_down >= 0
		    && rw->mouse.buttons_down <= 7) {
			SetButtons(rw);
			nw->mouse.buttons_down = rw->mouse.buttons_down;
		}
	}

	if ((cw->mouse.foreground != rw->mouse.foreground)
	    || (cw->mouse.font != rw->mouse.font)) {
		XtReleaseGC(new, nw->mouse.gc);
		val.foreground = rw->mouse.foreground;
		val.font = rw->mouse.font->fid;
		nw->mouse.gc = XtGetGC(new, GCForeground | GCFont, &val);
		redisplay = TRUE;
	}

	return (redisplay);
}


static void SetMouseName(w, button)
UKCMouseWidget w;
int button;
{
	_XukcMouseClients *mc;

	if (w->mouse.current_client == NO_CLIENT)
		return;

	mc = w->mouse.clients[w->mouse.current_client];
	switch (button) {
		case LEFT :
			if (*(mc->left_name) != '\0')
				XDrawString(XtDisplay(w), XtWindow(w),
					w->mouse.gc,
					mc->name_pos_x[LEFT],
					mc->name_pos_y[LEFT],
					mc->left_name,
					mc->name_lengths[LEFT]);
			break;
		case MIDDLE :
			if (*(mc->middle_name) != '\0')
				XDrawString(XtDisplay(w), XtWindow(w),
					w->mouse.gc,
					mc->name_pos_x[MIDDLE],
					mc->name_pos_y[MIDDLE],
					mc->middle_name,
					mc->name_lengths[MIDDLE]);
			break;
		case RIGHT :
			if (*(mc->right_name) != '\0')
				XDrawString(XtDisplay(w), XtWindow(w),
					w->mouse.gc,
					mc->name_pos_x[RIGHT],
					mc->name_pos_y[RIGHT],
					mc->right_name,
					mc->name_lengths[RIGHT]);
			break;
	}
}


static void ClearMouseNameArea(w, button)
UKCMouseWidget w;
int button;
{
	switch (button) {
		case LEFT :
			XClearArea(XtDisplay(w), XtWindow(w), 0, 
				w->mouse.boxes[LEFT].y,
				w->mouse.boxes[LEFT].x-1,
				w->core.height - w->mouse.boxes[LEFT].y, FALSE);
			break;
		case MIDDLE :
			XClearArea(XtDisplay(w), XtWindow(w), 0, 0,
				w->core.width,
				w->mouse.boxes[MIDDLE].y-1, FALSE);
			break;
		case RIGHT :
			XClearArea(XtDisplay(w), XtWindow(w),
			    w->mouse.boxes[RIGHT].x + 
			     (w->core.height*2/BUTTON_WIDTH_RATIO) + 1,
			    w->mouse.boxes[RIGHT].y,
			    w->core.width - w->mouse.boxes[MIDDLE].x,
			    w->core.height - w->mouse.boxes[RIGHT].y, FALSE);
			break;
	}
}


static void SetMouseButton(mw, button, turn_on)
UKCMouseWidget mw;
int button;
Boolean turn_on;
{
	if (turn_on)
		XFillRectangles(XtDisplay(mw), XtWindow(mw), mw->mouse.gc,
				&mw->mouse.boxes[button], 1);
	else
		XClearArea(XtDisplay(mw), XtWindow(mw),
		   mw->mouse.boxes[button].x+1,
		   mw->mouse.boxes[button].y+1,
		   mw->mouse.boxes[button].width-1,
		   mw->mouse.boxes[button].height-1,FALSE);
}


static void SetButtons(mw)
UKCMouseWidget mw;
{
	int buttons_down = mw->mouse.buttons_down;

	SetMouseButton(mw, MIDDLE, buttons_down & MH_Middle_Button);
	SetMouseButton(mw, LEFT, buttons_down & MH_Left_Button);
	SetMouseButton(mw, RIGHT, buttons_down & MH_Right_Button);
}


/* ARGSUSED */
static void
mouseClientDestroy(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	_XukcMouseClients *temp;
	_XukcMouseDyingClient *mdc = (_XukcMouseDyingClient *) client_data;
	UKCMouseWidget mw;

	if (mdc->widget != w)
		XtError("Bad client Widget passed to _mouseClientDestroy");

	temp = mdc->mw->mouse.clients[mdc->this_client];

	if (temp->widget != w)
		XtError("Corrupted data in internals of _mouseClientDestroy");

	XtRemoveEventHandler(w, EnterWindowMask, FALSE,
				mwWindowEntry, (XtPointer)mdc);
	XtRemoveEventHandler(w, LeaveWindowMask, FALSE,
				mwWindowLeave, (XtPointer)mdc);
	XtRemoveEventHandler(w, ButtonPressMask, FALSE,
				mwSetButton, (XtPointer)mdc->mw);
	XtRemoveEventHandler(w, ButtonReleaseMask, FALSE,
				mwClearButton, (XtPointer)mdc->mw);
	XtFree(temp->left_name);
	XtFree(temp->middle_name);
	XtFree(temp->right_name);

	mw = mdc->mw;
	mw->mouse.num_clients--;
	for (; mdc->this_client < mw->mouse.num_clients; mdc->this_client++)
	{
		mw->mouse.clients[mdc->this_client]
			= mw->mouse.clients[mdc->this_client + 1];
		mw->mouse.clients[mdc->this_client]
			->client_data->this_client--;
	}
	
	XtFree((char *)mdc);
}
	

static void
UpdateMouse(mw, new_client)
UKCMouseWidget mw;
Cardinal new_client;
{
	_XukcMouseClients *cc, *nc;

	if (mw->mouse.current_client == new_client)
		return;

	if (mw->mouse.current_client == NO_CLIENT) {
		mw->mouse.current_client = new_client;
		SetMouseName(mw, LEFT);
		SetMouseName(mw, MIDDLE);
		SetMouseName(mw, RIGHT);
		return;
	}

	cc = mw->mouse.clients[mw->mouse.current_client];
	nc = mw->mouse.clients[new_client];
	mw->mouse.current_client = new_client;

	if (strcmp(cc->left_name, nc->left_name) != 0) {
		ClearMouseNameArea(mw, LEFT);
		SetMouseName(mw, LEFT);
	}
	if (strcmp(cc->middle_name, nc->middle_name) != 0) {
		ClearMouseNameArea(mw, MIDDLE);
		SetMouseName(mw, MIDDLE);
	}
	if (strcmp(cc->right_name, nc->right_name) != 0) {
		ClearMouseNameArea(mw, RIGHT);
		SetMouseName(mw, RIGHT);
	}
}


/* timeout routine */
/* ARGSUSED */
static void
timeoutDisplay(client_data, id)
XtPointer client_data;
XtIntervalId *id;
{
	_XukcMouseDyingClient *mdc = (_XukcMouseDyingClient *)client_data;
	
	if (mdc == &mdc->mw->mouse.blank_mdc) {
		ClearMouseNameArea(mdc->mw, LEFT);
		ClearMouseNameArea(mdc->mw, MIDDLE);
		ClearMouseNameArea(mdc->mw, RIGHT);
		mdc->mw->mouse.current_client = NO_CLIENT;
	} else
		UpdateMouse(mdc->mw, mdc->this_client);
	mdc->mw->mouse.timer = NO_TIMER_SET;
}


/* Event Handler routines ... */
/* ARGSUSED */
static void mwWindowEntry(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	_XukcMouseDyingClient *mdc = (_XukcMouseDyingClient *)client_data;

#ifdef DEBUG
	if (mdc->widget != w)
		XtError("Garbled client_data passed to _mwWindowEntry");
	if (mdc->mw == NULL) 
		XtError("Bad Mouse Widget passed to _mwWindowEntry");
	if (mdc->this_client > mdc->mw->mouse.num_clients)
		XtError("Bad Client number passed to _mwWindowEntry");
#endif DEBUG
	if (mdc->mw->mouse.timer != NO_TIMER_SET)
		XtRemoveTimeOut(mdc->mw->mouse.timer);

	mdc->mw->mouse.timer = XtAppAddTimeOut(
				XtWidgetToApplicationContext((Widget)mdc->mw),
				mdc->mw->mouse.update_rate,
				timeoutDisplay, (XtPointer)mdc);
}


/* Event Handler routines ... */
/* ARGSUSED */
static void mwWindowLeave(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	_XukcMouseDyingClient *mdc = (_XukcMouseDyingClient *)client_data;

#ifdef DEBUG
	if (mdc->widget != w)
		XtError("Garbled client_data passed to _mwWindowLeave");
	if (mdc->mw == NULL) 
		XtError("Bad Mouse Widget passed to _mwWindowLeave");
	if (mdc->this_client > mdc->mw->mouse.num_clients)
		XtError("Bad Client number passed to _mwWindowLeave");
#endif DEBUG
	if (mdc->mw->mouse.timer != NO_TIMER_SET)
		XtRemoveTimeOut(mdc->mw->mouse.timer);

	mdc->mw->mouse.timer = XtAppAddTimeOut(
				XtWidgetToApplicationContext((Widget)mdc->mw),
				mdc->mw->mouse.update_rate,
				timeoutDisplay,
				(XtPointer)&mdc->mw->mouse.blank_mdc);
}

		
#define PowerOf2(num) (1 << (num - 1))

/* ARGSUSED */
static void mwSetButton(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	UKCMouseWidget mw = (UKCMouseWidget)client_data;

#ifdef DEBUG
	if (mw == NULL)
		XtError("Bad Mouse Widget passed to _mwSetButton");
#endif DEBUG

	mw->mouse.buttons_down |= PowerOf2(event->xbutton.button);
	SetButtons(mw);
}


/* ARGSUSED */
static void mwClearButton(w, client_data, event)
Widget w;
XtPointer client_data;
XEvent *event;
{
	UKCMouseWidget mw = (UKCMouseWidget)client_data;

#ifdef DEBUG
	if (mw == NULL)
		XtError("Bad Mouse Widget passed to mwClearButton");
#endif DEBUG

	mw->mouse.buttons_down &= ~PowerOf2(event->xbutton.button);
	SetButtons(mw);
}


/**** Public Routines ****/


void XukcMouseAddClient(mw, w, left_name, middle_name, right_name)
UKCMouseWidget mw;
Widget w;
String left_name;
String middle_name;
String right_name;
{
	int i;
	_XukcMouseClients	*mclient;
	_XukcMouseDyingClient *mdc;

	if (!XtIsSubclass((Widget)mw, ukcMouseWidgetClass)) {
		XtAppWarning(XtWidgetToApplicationContext(w),
			"XukcMouseAddClient() called with non UKCMouseWidget");
		return;
	}

	if (!XtIsWidget(w)) {
		XtAppWarning(XtWidgetToApplicationContext(w),
			"XukcMouseAddClient() called with non Widget client");
		return;
	}

	for (i=0; i < mw->mouse.num_clients; i++) {
		/* update existing client by overwriting */
		if (mw->mouse.clients[i]->widget == w) {
			mclient = mw->mouse.clients[i];

			XtFree((char *)mclient->left_name);
			XtFree((char *)mclient->middle_name);
			XtFree((char *)mclient->right_name);

			mclient->left_name = XtNewString(left_name);
			mclient->middle_name = XtNewString(middle_name);
			mclient->right_name = XtNewString(right_name);
			mclient->name_lengths[LEFT]
				= XtStrlen(mclient->left_name);
			mclient->name_lengths[MIDDLE]
				= XtStrlen(mclient->middle_name);
			mclient->name_lengths[RIGHT]
				= XtStrlen(mclient->right_name);
			NameLayout(mw, i);
			return;
		}
	}

	if (mw->mouse.clients == NULL) {
		mw->mouse.clients = (_XukcMouseClients **)
			XtCalloc(sizeof(_XukcMouseClients *), SLOT_INCREMENT);
		mw->mouse.num_slots = SLOT_INCREMENT;
	} else {
		if (mw->mouse.num_clients >= mw->mouse.num_slots) {
			mw->mouse.num_slots += SLOT_INCREMENT;
			mw->mouse.clients = (_XukcMouseClients **)
				XtRealloc((char *)mw->mouse.clients,
					  mw->mouse.num_slots
					  * sizeof(_XukcMouseClients *));
		}
	}

	/* save client data in array */
	mclient = mw->mouse.clients[mw->mouse.num_clients]
		= XtNew(_XukcMouseClients);
	mclient->widget = w;
	mclient->left_name = XtNewString(left_name);
	mclient->middle_name = XtNewString(middle_name);
	mclient->right_name = XtNewString(right_name);
	mclient->name_lengths[LEFT] = XtStrlen(mclient->left_name);
	mclient->name_lengths[MIDDLE] = XtStrlen(mclient->middle_name);
	mclient->name_lengths[RIGHT] = XtStrlen(mclient->right_name);
	/* workout string positioning */
	NameLayout(mw, mw->mouse.num_clients);

	/* save call data for use by callbacks and event handlers */
	mdc = XtNew(_XukcMouseDyingClient);
	mdc->widget = w;
	mdc->mw = mw;
	mdc->this_client = mw->mouse.num_clients++;
	mclient->client_data = mdc;

	/* client death clean up callback */
	XtAddCallback(w, XtNdestroyCallback, mouseClientDestroy, (XtPointer)mdc);

	/* the following events should be handled using a translation string
	 * however due to the fact that XtAugmentTranslations will not add
	 * to existing event sequence actions these have to be done by hand.
	 * Grrr...this needs to change I think.
	 */

	/* add handlers to change the names of the buttons in the mouse hole */
	XtAddEventHandler(w, EnterWindowMask, FALSE,
				mwWindowEntry, (XtPointer)mdc);
	XtAddEventHandler(w, LeaveWindowMask, FALSE,
				mwWindowLeave, (XtPointer)mdc);

	/* add handlers to automatically un/highlight mouse buttons */
	XtAddEventHandler(w, ButtonPressMask, FALSE,
				mwSetButton, (XtPointer)mw);
	XtAddEventHandler(w, ButtonReleaseMask, FALSE,
				mwClearButton, (XtPointer)mw);
}


/* ARGUSED */
void XukcMouseRemoveClient(mw, w, remove_children)
UKCMouseWidget mw;
Widget w;
Boolean remove_children;
{
	_XukcMouseClients *mclient = NULL;
	CompositeWidget cw = (CompositeWidget) w;
	Cardinal i;

	if (!XtIsSubclass((Widget)mw, ukcMouseWidgetClass))
		XtError("XukcMouseRemoveClient() called with non UKCMouseWidget");
	for (i=0; i < mw->mouse.num_clients; i++) {
		if (mw->mouse.clients[i]->widget == w) {
			mclient = mw->mouse.clients[i];
			break;
		}
	}

	if ((mclient == NULL) && !remove_children) {
		char error[100];

		(void)sprintf(error, "XukcMouseRemoveClient: Widget \"%s\" is not a client of \"%s\"", w->core.name, mw->core.name);
		XtWarning(error);
		return;
	}

	if (mclient != NULL) {
		mouseClientDestroy(w, (XtPointer)mclient->client_data, (XtPointer)NULL);
	}

	if (remove_children && XtIsComposite(w) && (cw->composite.num_children > 0)) {
		for (i=0; i < mw->mouse.num_clients; i++) {
			if (mw->mouse.clients[i]->widget->core.parent == w) {
				XukcMouseRemoveClient(mw,
					mw->mouse.clients[i]->widget, TRUE);
				i--;
			}
		}
	}
}
