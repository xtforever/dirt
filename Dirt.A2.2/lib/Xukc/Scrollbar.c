#ifndef lint
static char sccsid[] = "@(#)Scrollbar.c	1.3 (UKC) 5/17/92";
#endif /* !lint */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* Xukc/Scrollbar.c */
/* created by weissman, Mon Jul  7 13:20:03 1986 */
/* converted by swick, Thu Aug 27 1987 */
/* Added markings support by Richard Hesketh rlh2@ukc.ac.uk, Tue Dec 6 1988 */

#include <X11/IntrinsicP.h>
#include <X11/Xresource.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Drawing.h>
#include "ScrollbarP.h"

/* Private definitions. */

static char defaultTranslations[] =
    "<Btn1Down>:   StartScroll(Forward) \n\
     <Btn2Down>:   StartScroll(Continuous) MoveThumb() NotifyThumb() \n\
     <Btn3Down>:   StartScroll(Backward) \n\
     <Btn2Motion>: MoveThumb() NotifyThumb() \n\
     <BtnUp>:      NotifyScroll(Proportional) EndScroll()";

#ifdef bogusScrollTKeys
    /* examples */
    "<KeyPress>f:  StartScroll(Forward) NotifyScroll(FullLength) EndScroll()"
    "<KeyPress>b:  StartScroll(Backward) NotifyScroll(FullLength) EndScroll()"
#endif

static float floatZero = 0.0;

#define SEP_DIST 1
#define Offset(field) XtOffset(UKCScrollbarWidget, field)

static XtResource resources[] = {
  {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	     Offset(core.background_pixel), XtRString, XtExtdefaultbackground},
  {XtNfillMarks, XtCFillMarks, XtRBoolean, sizeof(Boolean),
	     Offset(scrollbar.fillMarks), XtRImmediate, (XtPointer)TRUE},
  {XtNlength, XtCLength, XtRDimension, sizeof(Dimension),
	     Offset(scrollbar.length), XtRImmediate, (XtPointer)1},
  {XtNmarkColor, XtCForeground, XtRPixel, sizeof(Pixel),
	     Offset(scrollbar.markColor), XtRString, XtExtdefaultforeground},
  {XtNmarkSize, XtCThickness, XtRDimension, sizeof(Dimension),
	     Offset(scrollbar.markSize), XtRImmediate, (XtPointer)5},
  {XtNthickness, XtCThickness, XtRDimension, sizeof(Dimension),
	     Offset(scrollbar.thickness), XtRImmediate, (XtPointer)7},
  {XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
	     Offset(scrollbar.orientation), XtRString, "vertical"},
  {XtNscrollProc, XtCCallback, XtRCallback, sizeof(XtPointer),
	     Offset(scrollbar.scrollProc), XtRCallback, NULL},
  {XtNthumbProc, XtCCallback, XtRCallback, sizeof(XtPointer),
	     Offset(scrollbar.thumbProc), XtRCallback, NULL},
  {XtNjumpProc, XtCCallback, XtRCallback, sizeof(XtPointer),
	     Offset(scrollbar.jumpProc), XtRCallback, NULL},
  {XtNthumb, XtCThumb, XtRBitmap, sizeof(Pixmap),
	     Offset(scrollbar.thumb), XtRPixmap, (XtPointer)None},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	     Offset(scrollbar.foreground), XtRString, XtExtdefaultforeground},
  {XtNshown, XtCShown, XtRFloat, sizeof(float),
	     Offset(scrollbar.shown), XtRFloat, (XtPointer)&floatZero},
  {XtNtop, XtCTop, XtRFloat, sizeof(float),
	     Offset(scrollbar.top), XtRFloat, (XtPointer)&floatZero},
  {XtNscrollVCursor, XtCCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.verCursor), XtRString, "sb_v_double_arrow"},
  {XtNscrollHCursor, XtCCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.horCursor), XtRString, "sb_h_double_arrow"},
  {XtNscrollUCursor, XtCCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.upCursor), XtRString, "sb_up_arrow"},
  {XtNscrollDCursor, XtCCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.downCursor), XtRString, "sb_down_arrow"},
  {XtNscrollLCursor, XtCCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.leftCursor), XtRString, "sb_left_arrow"},
  {XtNscrollRCursor, XtCCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.rightCursor), XtRString, "sb_right_arrow"},
  {XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof(Boolean),
	     Offset(scrollbar.reverse_video), XtRImmediate, (XtPointer)FALSE},
  {XtNcentre, XtCCentre, XtRBoolean, sizeof(Boolean),
	     Offset(scrollbar.centre), XtRImmediate, (XtPointer)FALSE},
};

static void ClassInitialize();
static void Initialize();
static void Realize();
static void Resize();
static void Redisplay();
static void Destroy();
static Boolean SetValues();

static void StartScroll();
static void MoveThumb();
static void NotifyThumb();
static void NotifyScroll();
static void EndScroll();

static XtActionsRec actions[] = {
	{"StartScroll",		StartScroll},
	{"MoveThumb",		MoveThumb},
	{"NotifyThumb",		NotifyThumb},
	{"NotifyScroll",	NotifyScroll},
	{"EndScroll",		EndScroll}
};


UKCScrollbarClassRec ukcScrollbarClassRec = {
/* core fields */
    /* superclass       */      (WidgetClass) &widgetClassRec,
    /* class_name       */      "UKCScrollbar",
    /* size             */      sizeof(UKCScrollbarRec),
    /* class_initialize	*/	ClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited	*/	FALSE,
    /* initialize       */      Initialize,
    /* initialize_hook  */	NULL,
    /* realize          */      Realize,
    /* actions          */      actions,
    /* num_actions	*/	XtNumber(actions),
    /* resources        */      resources,
    /* num_resources    */      XtNumber(resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion	*/	TRUE,
    /* compress_exposure*/	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      Destroy,
    /* resize           */      Resize,
    /* expose           */      Redisplay,
    /* set_values       */      SetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost */	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */      NULL,
    /* version          */	XtVersion,
    /* callback_private */      NULL,
    /* tm_table         */      defaultTranslations,
    /* query_geometry	*/	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension        */	NULL
};

WidgetClass ukcScrollbarWidgetClass = (WidgetClass)&ukcScrollbarClassRec;

#define MINBARHEIGHT	7     /* How many pixels of scrollbar to always show */
#define NoButton -1
#define PICKLENGTH(widget, x, y) \
    ((widget->scrollbar.orientation == XtorientHorizontal) ? x : y)
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))

static void ClassInitialize()
{
	/* ||| this needs to be changed when Xmu gets updated to R4 ||| */
    XtAddConverter(XtRString, XtROrientation, XmuCvtStringToOrientation,
		    NULL, (Cardinal)0);
}



/*
 * Make sure the first number is within the range specified by the other
 * two numbers.
 */

static int InRange(num, small, big)
int num, small, big;
{
    return (num < small) ? small : ((num > big) ? big : num);
}

/*
 * Same as above, but for floating numbers. 
 */

static float FloatInRange(num, small, big)
float num, small, big;
{
    return (num < small) ? small : ((num > big) ? big : num);
}


/* Fill the area specified by top and bottom with the given pattern. */
static float FractionLoc(w, x, y)
UKCScrollbarWidget w;
int x, y;
{
    float   result;

    result = PICKLENGTH(w, (float) x/w->core.width,
			(float) y/w->core.height);
    return FloatInRange(result, 0.0, 1.0);
}


static FillArea(w, top, bottom, thumb)
UKCScrollbarWidget w;
Position top, bottom;
int thumb;
{
    Dimension length = bottom-top;
    Dimension width = w->scrollbar.markSize
			+ (w->scrollbar.markSize > 0 ? SEP_DIST : 0) + 2;

    if (bottom < 0) return;

    switch(thumb) {
	/* Fill the new Thumb location */
      case 1:
	if (w->scrollbar.orientation == XtorientHorizontal) 
	    XFillRectangle(XtDisplay(w), XtWindow(w),
			   w->scrollbar.gc, top, 1, length,
			   w->core.height-width);
	
	else XFillRectangle(XtDisplay(w), XtWindow(w), w->scrollbar.gc, 1, top,
			    w->core.width-width, length);

	break;
	/* Clear the old Thumb location */
      case 0:
	if (w->scrollbar.orientation == XtorientHorizontal) 
	    XClearArea(XtDisplay(w), XtWindow(w), top, 1,
		       length, w->core.height-width, FALSE);
	
	else XClearArea(XtDisplay(w), XtWindow(w), 1, top,
			w->core.width-width, length, FALSE);

    }  
}


/* Paint the thumb in the area specified by w->top and
   w->shown.  The old area is erased.  The painting and
   erasing is done cleverly so that no flickering will occur. */

static void PaintThumb( w )
UKCScrollbarWidget w;
{
    Position oldtop, oldbot, newtop, newbot;

    oldtop = w->scrollbar.topLoc;
    oldbot = oldtop + w->scrollbar.shownLength;
    newtop = w->scrollbar.length * w->scrollbar.top;
    newbot = newtop + (int)(w->scrollbar.length * w->scrollbar.shown);
    if (newbot < newtop + MINBARHEIGHT) newbot = newtop + MINBARHEIGHT;
    w->scrollbar.topLoc = newtop;
    w->scrollbar.shownLength = newbot - newtop;

    if (XtIsRealized((Widget)w)) {
	if (newtop < oldtop) FillArea(w, newtop, MIN(newbot, oldtop), 1);
	if (newtop > oldtop) FillArea(w, oldtop, MIN(newtop, oldbot), 0);
	if (newbot < oldbot) FillArea(w, MAX(newbot, oldtop), oldbot, 0);
	if (newbot > oldbot) FillArea(w, MAX(newtop, oldbot), newbot, 1);
    }
}


/* Remove a thumb mark */
static void do_remove(gw, mark)
Widget gw;
MarkList mark;
{
	UKCScrollbarWidget w = (UKCScrollbarWidget)gw;
	MarkList ml = w->scrollbar.markList;

	if (mark == NULL || ml == NULL)
		return;

	if (ml == mark)
		w->scrollbar.markList = mark->next;
	else {
		while (ml->next != NULL && ml->next != mark)
			ml = ml->next;

		if (ml->next == NULL)
			XtError("Unable to remove mark in XtScrollTBarRemoveMark");
		ml->next = ml->next->next;
	}
	XtFree((char *)mark);
}


static void PaintMarks(w, redisplay)
UKCScrollbarWidget w;
Boolean redisplay;
{
	Dimension start, end, thick;
	MarkList ml;
	Boolean is_vert;

	if (w->scrollbar.markSize == 0 ||
	    (!w->scrollbar.marks_changed && !redisplay) ||
	    !XtIsRealized((Widget)w))
		return;

	w->scrollbar.marks_changed = FALSE;

	is_vert = w->scrollbar.orientation == XtorientVertical;
	thick = w->scrollbar.thickness
			+ (w->scrollbar.markSize > 0 ? SEP_DIST : 0);

	if (w->scrollbar.markList == NULL || redisplay)
		XClearArea(XtDisplay(w), XtWindow(w), (int)(is_vert ? thick : 0),
		   (int)(is_vert ? 0 : thick), 0, 0, FALSE);

	for (ml = w->scrollbar.markList; ml != NULL; ml = ml->next) {
		/* ignore unchanged entries unless we are doing a redisplay */
		if (!ml->changed && !redisplay)
			continue;

		/* remove dead entries from the display unless we have
		 * already cleared it.
		 */
		if (redisplay && ml->dead)
			continue;

		/* dimensions of box */
		start = (Dimension)((is_vert ? w->core.height : w->core.width) *
			 ml->start_percent);
		end = (Dimension)((is_vert ? w->core.height : w->core.width) *
			 ml->end_percent);

		/* Draw a filled rectangle if fillMarks is true */
		if (w->scrollbar.fillMarks && start != end)
			XFillRectangle(XtDisplay(w), XtWindow(w),
			    ml->dead ? w->scrollbar.removeGC : w->scrollbar.markGC,
			    (int)(is_vert ? thick : start),
			    (int)(is_vert ? start : thick),
			    (unsigned int)(is_vert ? w->scrollbar.markSize :
				end - start),
			    (unsigned int)(is_vert ? end - start :
				w->scrollbar.markSize));
		else
			XDrawRectangle(XtDisplay(w), XtWindow(w),
			    ml->dead ? w->scrollbar.removeGC : w->scrollbar.markGC,
			    (int)(is_vert ? thick : start),
			    (int)(is_vert ? start : thick),
			    (unsigned int)(is_vert ? w->scrollbar.markSize :
				end - start),
			    (unsigned int)(is_vert ? end - start :
				w->scrollbar.markSize));
		ml->changed = FALSE;
	}

	/* remove "marked as dead" thumb marks */
	for (ml = w->scrollbar.markList; ml != NULL; ) {
		if (ml->dead) {
			MarkList t = ml->next;
			do_remove(w, ml);
			ml = t;
		} else
			ml = ml->next;
	}
}


static void SetDimensions(w)
UKCScrollbarWidget w;
{
    if (w->scrollbar.orientation == XtorientVertical) {
	w->scrollbar.length = w->core.height;
	w->scrollbar.thickness = w->core.width - w->scrollbar.markSize
				   - (w->scrollbar.markSize > 0 ? SEP_DIST : 0);
    }
    else {
	w->scrollbar.length = w->core.width;
	w->scrollbar.thickness = w->core.height - w->scrollbar.markSize
				   - (w->scrollbar.markSize > 0 ? SEP_DIST : 0);
    }
}


/* ARGSUSED */
static void Initialize( request, new )
Widget request;		/* what the client asked for */
Widget new;			/* what we're going to give him */
{
    Dimension thick;
    UKCScrollbarWidget w = (UKCScrollbarWidget) new;
    XGCValues gcValues;

    thick = w->scrollbar.markSize > 0 ? w->scrollbar.markSize + SEP_DIST : 0;

    if (w->scrollbar.reverse_video) {
	Pixel fg = w->scrollbar.foreground;
	Pixel bg = w->core.background_pixel;

	w->core.background_pixel = fg;
	w->scrollbar.foreground = bg;
    }

    if (w->scrollbar.thumb == NULL) {
        w->scrollbar.thumb = XmuCreateStippledPixmap(XtScreen(w),
					w->scrollbar.foreground,
					w->core.background_pixel,
					DefaultDepthOfScreen(XtScreen(w)));
    }

    gcValues.foreground = w->scrollbar.foreground;
    gcValues.fill_style = FillTiled;
    gcValues.tile = w->scrollbar.thumb;
    w->scrollbar.gc = XtGetGC( new,
			       GCForeground | GCFillStyle | GCTile,
			       &gcValues);

    gcValues.foreground = w->scrollbar.markColor;
    w->scrollbar.markGC = XtGetGC( new, GCForeground, &gcValues);
    gcValues.foreground = w->core.background_pixel;
    w->scrollbar.removeGC = XtGetGC( new, GCForeground, &gcValues);

    if (w->core.width == 0)
	w->core.width = (w->scrollbar.orientation == XtorientVertical)
	    ? w->scrollbar.thickness + thick : w->scrollbar.length;

    if (w->core.height == 0)
	w->core.height = (w->scrollbar.orientation == XtorientHorizontal)
	    ? w->scrollbar.thickness + thick : w->scrollbar.length;

    SetDimensions( w );
    w->scrollbar.direction = 0;
    w->scrollbar.topLoc = 0;
    w->scrollbar.shownLength = 0;
    w->scrollbar.markList = NULL;
    w->scrollbar.marks_changed = FALSE;
}

static void Destroy(w)
Widget w;
{
	UKCScrollbarWidget sw = (UKCScrollbarWidget)w;

	XtReleaseGC(w, sw->scrollbar.gc);
	XtReleaseGC(w, sw->scrollbar.markGC);
	XukcScrollbarResetMarks(w);
}

static void Realize( gw, valueMask, attributes )
Widget gw;
Mask *valueMask;
XSetWindowAttributes *attributes;
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;

    w->scrollbar.inactiveCursor =
      (w->scrollbar.orientation == XtorientVertical)
	? w->scrollbar.verCursor
	: w->scrollbar.horCursor;

    attributes->cursor = w->scrollbar.inactiveCursor;
    *valueMask |= CWCursor;
    
    XtCreateWindow( gw, InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes );
}


/* ARGSUSED */
static Boolean SetValues( current, request, desired )
Widget current,		/* what I am */
       request,		/* what he wants me to be */
       desired;		/* what I will become */
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) current;
    UKCScrollbarWidget rw = (UKCScrollbarWidget) request;
    UKCScrollbarWidget dw = (UKCScrollbarWidget) desired;
    Boolean redraw = FALSE;
    Boolean realized = XtIsRealized (desired);
    XGCValues gcValues;

    if (w->scrollbar.reverse_video != rw->scrollbar.reverse_video) {
	Pixel fg = dw->scrollbar.foreground;
	Pixel bg = dw->core.background_pixel;

	dw->scrollbar.foreground = bg;
	dw->core.background_pixel = fg;
	if (realized)
	  XSetWindowBackground (XtDisplay (desired), XtWindow (desired), 
				dw->core.background_pixel);
	redraw = TRUE;
    }

    if (w->scrollbar.foreground != rw->scrollbar.foreground ||
	w->scrollbar.thumb != rw->scrollbar.thumb) {
	gcValues.foreground = dw->scrollbar.foreground;
	gcValues.fill_style = FillTiled;
	if (dw->scrollbar.thumb == None)
		dw->scrollbar.thumb = XmuCreateStippledPixmap(XtScreen(dw),
					dw->scrollbar.foreground,
					dw->core.background_pixel,
					DefaultDepthOfScreen(XtScreen(dw)));
	gcValues.tile = dw->scrollbar.thumb;

	XtReleaseGC((Widget)dw, dw->scrollbar.gc);
	dw->scrollbar.gc = XtGetGC((Widget)dw, GCForeground |
					GCFillStyle | GCTile,
					&gcValues);
	redraw = TRUE;
    }

    if (w->core.background_pixel != rw->core.background_pixel) {
	gcValues.foreground = dw->core.background_pixel;
	XtReleaseGC((Widget)dw, dw->scrollbar.removeGC);
	dw->scrollbar.removeGC = XtGetGC((Widget)dw, GCForeground, &gcValues);
	redraw = TRUE;
    }

    if (dw->scrollbar.top < 0.0 || dw->scrollbar.top > 1.0)
        dw->scrollbar.top = w->scrollbar.top;

    if (dw->scrollbar.shown < 0.0 || dw->scrollbar.shown > 1.0)
        dw->scrollbar.shown = w->scrollbar.shown;

    if (w->scrollbar.top != dw->scrollbar.top ||
        w->scrollbar.shown != dw->scrollbar.shown ||
	w->scrollbar.fillMarks != dw->scrollbar.fillMarks ||
	w->scrollbar.markSize != dw->scrollbar.markSize)
		redraw = TRUE;


    if (w->scrollbar.markColor != dw->scrollbar.markColor)
    {
	    gcValues.foreground = dw->scrollbar.markColor;
	    XtReleaseGC((Widget)dw, dw->scrollbar.markGC);
	    dw->scrollbar.markGC = XtGetGC((Widget)dw, GCForeground, &gcValues);
	    redraw = TRUE;
    }

    return( redraw );
}

/* ARGSUSED */
static void Resize( gw )
Widget gw;
{
    /* ForgetGravity has taken care of background, but thumb may
     * have to move as a result of the new size. */
    SetDimensions( (UKCScrollbarWidget)gw );
    Redisplay( gw, (XEvent*)NULL, (Region)NULL );
}


/* ARGSUSED */
static void Redisplay( gw, event, region )
Widget gw;
XEvent *event;
Region region;
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;
    int x, y;
    unsigned int width, height;
    Dimension m_width = w->scrollbar.markSize
			+ (w->scrollbar.markSize > 0 ? SEP_DIST : 0) + 2;

    if (w->scrollbar.orientation == XtorientHorizontal) {
	x = w->scrollbar.topLoc;
	y = 1;
	width = w->scrollbar.shownLength;
	height = w->core.height - m_width;
    } else {
	x = 1;
	y = w->scrollbar.topLoc;
	width = w->core.width - m_width;
	height = w->scrollbar.shownLength;
    }

    if (region == NULL ||
	  XRectInRegion(region, x, y, width, height) != RectangleOut) {
	if (region != NULL) XSetRegion(XtDisplay(w), w->scrollbar.gc, region);
	/* Forces entire thumb to be painted. */
	w->scrollbar.topLoc = -(w->scrollbar.length + 1);
	PaintThumb( w ); 
	if (region != NULL) XSetClipMask(XtDisplay(w), w->scrollbar.gc, None);
    }

    PaintMarks(w, TRUE);
}


/* ARGSUSED */
static void
StartScroll( gw, event, params, num_params )
Widget gw;
XEvent *event;
String *params;		/* direction: Back|Forward|Smooth */
Cardinal *num_params;		/* we only support 1 */
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;
    Cursor cursor;
    char direction;

    if (w->scrollbar.direction != 0) return; /* if we're already scrolling */
    if (*num_params > 0) direction = *params[0];
    else		 direction = 'C';

    w->scrollbar.direction = direction;

    switch( direction ) {
	case 'B':
	case 'b':	cursor = (w->scrollbar.orientation == XtorientVertical)
				   ? w->scrollbar.downCursor
				   : w->scrollbar.rightCursor; break;

	case 'F':
	case 'f':	cursor = (w->scrollbar.orientation == XtorientVertical)
				   ? w->scrollbar.upCursor
				   : w->scrollbar.leftCursor; break;

	case 'C':
	case 'c':	cursor = (w->scrollbar.orientation == XtorientVertical)
				   ? w->scrollbar.rightCursor
				   : w->scrollbar.upCursor; break;

	default:	return; /* invalid invocation */
    }

    XDefineCursor(XtDisplay(w), XtWindow(w), cursor);

    XFlush(XtDisplay(w));
}


static Boolean CompareEvents( oldEvent, newEvent )
XEvent *oldEvent, *newEvent;
{
#define Check(field) if (newEvent->field != oldEvent->field) return False;

    Check(xany.display);
    Check(xany.type);
    Check(xany.window);

    switch( newEvent->type ) {
      case MotionNotify:
		Check(xmotion.state); break;
      case ButtonPress:
      case ButtonRelease:
		Check(xbutton.state);
		Check(xbutton.button); break;
      case KeyPress:
      case KeyRelease:
		Check(xkey.state);
		Check(xkey.keycode); break;
      case EnterNotify:
      case LeaveNotify:
		Check(xcrossing.mode);
		Check(xcrossing.detail);
		Check(xcrossing.state); break;
    }
#undef Check

    return True;
}

struct EventData {
	XEvent *oldEvent;
	int count;
};

static Bool PeekNotifyEvent( dpy, event, args )
Display *dpy;
XEvent *event;
char *args;
{
    struct EventData *eventData = (struct EventData*)args;

    return ((++eventData->count == QLength(dpy)) /* since PeekIf blocks */
	    || CompareEvents(event, eventData->oldEvent));
}


static Boolean LookAhead( w, event )
Widget w;
XEvent *event;
{
    XEvent newEvent;
    struct EventData eventData;

    if (QLength(XtDisplay(w)) == 0) return False;

    eventData.count = 0;
    eventData.oldEvent = event;

    XPeekIfEvent(XtDisplay(w), &newEvent, PeekNotifyEvent, (char*)&eventData);

    return (CompareEvents(event, &newEvent));
}


static void ExtractPosition(w, event, x, y)
UKCScrollbarWidget w;
XEvent *event;
Position *x, *y;		/* RETURN */
{
    switch(event->type) {
      case MotionNotify:
		*x = event->xmotion.x;	 *y = event->xmotion.y;	  break;
      case ButtonPress:
      case ButtonRelease:
		*x = event->xbutton.x;   *y = event->xbutton.y;   break;
      case KeyPress:
      case KeyRelease:
		*x = event->xkey.x;      *y = event->xkey.y;	  break;
      case EnterNotify:
      case LeaveNotify:
		*x = event->xcrossing.x; *y = event->xcrossing.y; break;
      default:
		*x = 0; *y = 0;
    }
    if (w->scrollbar.centre) {
	if (w->scrollbar.orientation == XtorientHorizontal)
		*x -= w->scrollbar.shown*w->scrollbar.length/2;
	else
		*y -= w->scrollbar.shown*w->scrollbar.length/2;
    }
}

static void NotifyScroll( gw, event, params, num_params   )
Widget gw;
XEvent *event;
String *params;		/* style: Proportional|FullLength */
Cardinal *num_params;	/* we only support 1 */
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;
    int call_data;
    char style;
    Position x, y;

    if (w->scrollbar.direction == 0) return; /* if no StartScroll */

    if (LookAhead(gw, event)) return;

    if (*num_params > 0) style = *params[0];
    else		 style = 'P';

    switch( style ) {
        case 'P':    /* Proportional */
        case 'p':    ExtractPosition(w, event, &x, &y);
		     call_data = InRange( PICKLENGTH( w, x, y ),
					  0,
					  (int) w->scrollbar.length); break;

        case 'F':    /* FullLength */
        case 'f':    call_data = w->scrollbar.length; break;
    }

    switch( w->scrollbar.direction ) {
        case 'B':
        case 'b':    call_data = -call_data;
	  	     /* fall through */
        case 'F':
	case 'f':    XtCallCallbacks( gw, XtNscrollProc, (XtPointer)call_data );
	             break;

        case 'C':
	case 'c':    /* NotifyThumb has already called the thumbProc(s) */
		     break;
    }
}

/* ARGSUSED */
static void EndScroll(gw, event, params, num_params )
Widget gw;
XEvent *event;		/* unused */
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;

    XDefineCursor(XtDisplay(w), XtWindow(w), w->scrollbar.inactiveCursor);
    XFlush(XtDisplay(w));

    w->scrollbar.direction = 0;
}


/* ARGSUSED */
static void MoveThumb( gw, event, params, num_params )
Widget gw;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;
    Position x, y;

    if (w->scrollbar.direction == 0) return; /* if no StartScrollT */

    if (LookAhead(gw, event)) return;

    ExtractPosition(w, event, &x, &y);
    w->scrollbar.top = FractionLoc(w, x, y);
    PaintThumb(w);
    XFlush(XtDisplay(w));	/* re-draw it before Notifying */
}


/* ARGSUSED */
static void
NotifyThumb( gw, event, params, num_params )
Widget gw;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
    UKCScrollbarWidget w = (UKCScrollbarWidget) gw;

    if (w->scrollbar.direction == 0) return; /* if no StartScrollT */

    if (LookAhead(gw, event)) return;

    XtCallCallbacks(gw, XtNthumbProc, *(XtPointer *)&w->scrollbar.top);
    XtCallCallbacks(gw, XtNjumpProc, (XtPointer)&w->scrollbar.top);
}



/* Public routines. */

/* Add a thumb mark */
PubMark
XukcScrollbarAddMark(gw)
Widget gw;
{
	UKCScrollbarWidget w = (UKCScrollbarWidget)gw;
	MarkList ml = w->scrollbar.markList;
	MarkList temp;

	if (ml == NULL)
	{
		ml = XtNew(MarkListRec);
		ml->start_percent = ml->end_percent = 0;
		ml->dead = ml->changed = FALSE;
		ml->next = NULL;
		w->scrollbar.markList = ml;
		return ((PubMark)ml);
	}

	while (ml->next != NULL)
		ml = ml->next;

	temp = ml->next;
	ml->next = XtNew(MarkListRec);
	ml = ml->next;
	ml->start_percent = ml->end_percent = 0;
	ml->dead = ml->changed = FALSE;
	ml->next = temp;

	return ((PubMark)ml);
}



void
XukcScrollbarRemoveMark(gw, mark)
Widget gw;
PubMark mark;
{
	if (mark != NULL) {
		((MarkList)mark)->dead = TRUE;
		((MarkList)mark)->changed = TRUE;
		((UKCScrollbarWidget)gw)->scrollbar.marks_changed = TRUE;
	}
}
	
void
XukcScrollbarResetMarks(gw)
Widget gw;
{
	UKCScrollbarWidget w = (UKCScrollbarWidget)gw;
	MarkList ml = w->scrollbar.markList;
	MarkList temp;

	if (w->scrollbar.markList != NULL) {
		while (ml->next != NULL) {
			temp = ml;
			ml = ml->next;
			XtFree((char *)temp);
		}

		XtFree((char *)w->scrollbar.markList);
		w->scrollbar.markList = NULL;
	}
	w->scrollbar.marks_changed = TRUE;
}


PubMark
XukcScrollbarUpdateMark(gw, mark, start, end)
Widget gw;
PubMark mark;
double start, end;
{
	UKCScrollbarWidget w = (UKCScrollbarWidget)gw;

	if (mark == NULL)
		mark = XukcScrollbarAddMark(gw);

	if (mark->start_percent != start || mark->end_percent != end) {
		mark->start_percent = start;
		mark->end_percent = end;
		((MarkList)mark)->changed = TRUE;
		w->scrollbar.marks_changed = TRUE;
	}

	return (mark);
}

void XukcScrollbarDisplayMarks(gw, redisplay_all)
Widget gw;
Boolean redisplay_all;
{
	UKCScrollbarWidget w = (UKCScrollbarWidget)gw;

	if (XtIsRealized(gw) && w->scrollbar.marks_changed)
		PaintMarks(w, redisplay_all);
}

/* Set the scroll bar to the given location. */
void XukcScrollbarSetThumb( gw, top, shown )
Widget gw;
float top, shown;
{
    UKCScrollbarWidget w = (UKCScrollbarWidget)gw;

    if (w->scrollbar.direction == 'c') return; /* if still thumbing */

    w->scrollbar.top = (top > 1.0) ? 1.0 :
		       (top >= 0.0) ? top :
			   w->scrollbar.top;

    w->scrollbar.shown = (shown > 1.0) ? 1.0 :
			 (shown >= 0.0) ? shown :
			     w->scrollbar.shown;

    PaintThumb( w );
}
