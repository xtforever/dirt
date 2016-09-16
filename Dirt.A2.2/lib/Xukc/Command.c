#ifndef lint
static char sccsid[] = "@(#)Command.c	1.2 (UKC) 5/10/92";
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


/*
 * Command.c - UKCCommand button widget
 *
 * Updated from Athena Widget Set commandWidgetClass by rlh2@ukc.ac.uk
 * Richard Hesketh - Computing Lab. University of Kent at Canterbury, UK
 *
 * Just added a highlight colour so that the highlighting border can
 * be set to a different colour. This was done so that a toggle widget
 * could be sub-classed from this widget and you could see the difference
 * between a toggled and untoggled button when it is highlighted.
 *
 * Redisplay with regions re-written.
 */

#define XtStrlen(s)	((s) ? strlen(s) : 0)

  /* The following are defined for the reader's convenience.  Any
     Xt..Field macro in this code just refers to some field in
     one of the substructures of the WidgetRec.  */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include "CommandP.h"
#include "CommandI.h"

static void Resize();
static XukcUpdateBitmapProc UpdateBitmap();

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char defaultTranslations[] =
       "~Button1<EnterWindow>:	highlight() \n\
	Button1<EnterWindow>:	set() highlight() \n\
	~Button1<LeaveWindow>:	unhighlight()	\n\
	Button1<LeaveWindow>:	reset()		\n\
	<Btn1Down>:		set()		\n\
	<Btn1Up>:		notify() unset()	\n\
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

#define offset(field) XtOffset(UKCCommandWidget, field)
static XtResource resources[] = { 
   {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer), 
      offset(command.callbacks), XtRCallback, (XtPointer)NULL},
   {XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      offset(command.highlight_thickness), XtRImmediate, (XtPointer)2},
   {XtNhighlightColor, XtCForeground, XtRPixel, sizeof(Pixel),
      offset(command.highlight_color), XtRString, XtExtdefaultforeground},
   {XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
      offset(simple.cursor), XtRString, "hand2" },
};
#undef offset

static XtActionsRec actionsList[] = {
  {"set",		Set},
  {"notify",		Notify},
  {"highlight",		Highlight},
  {"reset",		Reset},
  {"unset",		Unset},
  {"unhighlight",	Unhighlight},
};

static Region spare_region = NULL;

#define SuperClass ((UKCLabelWidgetClass)&ukcLabelClassRec)

UKCCommandClassRec ukcCommandClassRec = {
  {
    (WidgetClass) SuperClass,		/* superclass		  */	
    "UKCCommand",			/* class_name		  */
    sizeof(UKCCommandRec),		/* size			  */
    NULL,				/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    FALSE,				/* class_inited		  */
    Initialize,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* resource_count	  */
    NULLQUARK,				/* xrm_class		  */
    FALSE,				/* compress_motion	  */
    TRUE,				/* compress_exposure	  */
    TRUE,				/* compress_enterleave    */
    FALSE,				/* visible_interest	  */
    Destroy,				/* destroy		  */
    Resize,				/* resize		  */
    Redisplay,				/* expose		  */
    SetValues,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,		/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL,				/* extension		  */
  },  /* CoreClass fields initialization */
  {
    XtInheritChangeSensitive,
  },  /* SimpleClass fields initialization */
  {
    (XukcUpdateBitmapProc)UpdateBitmap,    /* update_bitmap_proc    */
  },  /* LabelClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* UKCCommandClass fields initialization */
};

  /* for public consumption */

WidgetClass ukcCommandWidgetClass = (WidgetClass) &ukcCommandClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static GC Get_GC(cbw, fg, bg)
UKCCommandWidget cbw;
Pixel fg, bg;
{
    XGCValues	values;

    values.foreground   = fg;
    values.background	= bg;
    values.font		= ComWfont->fid;
    values.line_width   = ComWhighlightThickness > 1
			  ? ComWhighlightThickness : 0;

    return XtGetGC((Widget)cbw,
		   GCForeground | GCBackground | GCFont | GCLineWidth,
		   &values);
}


/* ARGSUSED */
static void Initialize(request, new)
Widget request, new;
{
    UKCCommandWidget cbw = (UKCCommandWidget) new;

    ComWnormalGC = Get_GC(cbw, ComWforeground, ComWbackground);
    ComWinverseGC = Get_GC(cbw, ComWbackground, ComWforeground);
    ComWhighlightGC = Get_GC(cbw, ComWhighlightColor, ComWbackground);
    XtReleaseGC(new, ComWlabelGC);
    ComWlabelGC = ComWnormalGC;

    ComWset = FALSE;
    ComWhighlighted = FALSE;
    ComWhighlightRegion = ComWsetRegion = NULL;

    SetRegion(cbw);
    HighlightRegion(cbw);
}


HighlightRegion(cbw)
UKCCommandWidget cbw;
{
    static Region innerRegion = NULL, emptyRegion;
    XRectangle rect;

    /* save time by allocating scratch regions only once. */
    if (innerRegion == NULL) {
	innerRegion = XCreateRegion();
	emptyRegion = XCreateRegion();
    }

    if (spare_region == NULL)
	spare_region = XCreateRegion();

    if (ComWhighlightRegion == NULL)
	ComWhighlightRegion = XCreateRegion();

    rect.x = rect.y = 0;
    rect.width = cbw->core.width;
    rect.height = cbw->core.height;
    XUnionRectWithRegion( &rect, emptyRegion, ComWhighlightRegion );
    rect.x = rect.y = ComWhighlightThickness;
    rect.width -= ComWhighlightThickness * 2;
    rect.height -= ComWhighlightThickness * 2;
    XUnionRectWithRegion(&rect, emptyRegion, innerRegion);
    XUnionRegion(ComWhighlightRegion, emptyRegion, spare_region);
    XSubtractRegion(spare_region, innerRegion, ComWhighlightRegion);
}


SetRegion(cbw)
UKCCommandWidget cbw;
{
    static Region emptyRegion = NULL;
    XRectangle rect;

    /* save time by allocating scratch regions only once. */
    if (emptyRegion == NULL)
	emptyRegion = XCreateRegion();
    if (ComWsetRegion == NULL)
	ComWsetRegion = XCreateRegion();
    rect.x = rect.y = ComWhighlightThickness;
    rect.width = cbw->core.width - (ComWhighlightThickness * 2);
    rect.height = cbw->core.height - (ComWhighlightThickness * 2);
    XUnionRectWithRegion(&rect, emptyRegion, ComWsetRegion);

    if (cbw->label.left_region != NULL || cbw->label.right_region != NULL) {
	if (spare_region == NULL)
		spare_region = XCreateRegion();
	XUnionRectWithRegion(&rect, emptyRegion, spare_region);

	if (cbw->label.left_region != NULL)
		XSubtractRegion(spare_region, cbw->label.left_region,
				ComWsetRegion);
	if (cbw->label.right_region != NULL)
		XSubtractRegion(spare_region, cbw->label.right_region,
				ComWsetRegion);
    }
}


/***************************
*
*  Action Procedures
*
***************************/

/* ARGSUSED */
static void Set(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  UKCCommandWidget cbw = (UKCCommandWidget)w;
  if (!ComWset) {
      ComWset = TRUE;
      ComWlabelGC = ComWinverseGC;
      Redisplay(w, event, ComWsetRegion);
  }
}

/* ARGSUSED */
static void Unset(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;
{
  UKCCommandWidget cbw = (UKCCommandWidget)w;
  if (ComWset) {
      ComWset = FALSE;
      ComWlabelGC = ComWnormalGC;
      Redisplay(w, event, ComWsetRegion);
  }
}

/* ARGSUSED */
static void Reset(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;
{
	UKCCommandWidget cbw = (UKCCommandWidget)w;

	if (ComWset) {
		ComWset = FALSE;
		ComWlabelGC = ComWnormalGC;
	}
	if (ComWhighlighted)
		ComWhighlighted = FALSE;
	Redisplay(w, event, (Region)NULL);
}

/* ARGSUSED */
static void Highlight(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  UKCCommandWidget cbw = (UKCCommandWidget)w;
  if (!ComWhighlighted) {
      ComWhighlighted = TRUE;
      Redisplay(w, event, ComWhighlightRegion);
  }
}

/* ARGSUSED */
static void Unhighlight(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  UKCCommandWidget cbw = (UKCCommandWidget)w;
  if (ComWhighlighted) {
      ComWhighlighted = FALSE;
      Redisplay(w, event, ComWhighlightRegion);
  }
}

/* ARGSUSED */
static void Notify(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  UKCCommandWidget cbw = (UKCCommandWidget)w;
  if (ComWset)
      /* now passes the buttons text "label" as the call data */
      XtCallCallbacks(w, XtNcallback, (XtPointer)cbw->label.label);
}
/*
 * Repaint the widget window
 */

/************************
*
*  REDISPLAY (DRAW)
*
************************/

#define MIN(x, y)       (((x) < (y)) ? (x) : (y))

/* ARGSUSED */
static void Redisplay(w, event, region)
    Widget w;
    XEvent *event;
    Region region;
{
   UKCCommandWidget cbw = (UKCCommandWidget) w;
   Boolean very_thick = ComWhighlightThickness > MIN(ComWwidth,ComWheight)/2;
   static Region scratchRegion = NULL;

   /* create this only once */
   if (scratchRegion == NULL)
	scratchRegion = XCreateRegion();

   if (region != NULL)
	XIntersectRegion(region, ComWhighlightRegion, scratchRegion);

   if (ComWhighlightThickness > 0
       && (region == NULL || !XEmptyRegion(scratchRegion))) {
       if (very_thick) {
	   ComWlabelGC = ComWhighlighted ? ComWhighlightGC : ComWnormalGC;
	   XFillRectangle(XtDisplay(w),XtWindow(w), 
			  (ComWhighlighted ? ComWhighlightGC : ComWinverseGC),
			  0,0,ComWwidth,ComWheight);
       }
       else {
	   /* wide lines are centered on the path, so indent it */
	   int offset = ComWhighlightThickness/2;
	   XDrawRectangle(XtDisplay(w),XtWindow(w),
			  (ComWhighlighted ? ComWhighlightGC : ComWinverseGC),
			  offset, offset,
			  ComWwidth - ComWhighlightThickness,
			  ComWheight - ComWhighlightThickness);
       }
   }

   if (region != NULL)
	XIntersectRegion(region, ComWsetRegion, scratchRegion);

   if (region == NULL || !XEmptyRegion(scratchRegion)) {
       XFillRectangle(XtDisplay(w), XtWindow(w),
		      ComWset ? ComWnormalGC : ComWinverseGC,
		      ComWhighlightThickness, ComWhighlightThickness,
		      ComWwidth - ComWhighlightThickness*2,
		      ComWheight - ComWhighlightThickness*2);
	/* force label to be re-displayed */
	region = NULL;
   }

   (*SuperClass->core_class.expose)(w, event, region);
}


static void Destroy(w)
Widget w;
{
    UKCCommandWidget cbw = (UKCCommandWidget)w;
    ComWlabelGC = ComWnormalGC;	/* so Label can release it */
    XtReleaseGC(w, ComWinverseGC);
    XtReleaseGC(w, ComWhighlightGC);
    XDestroyRegion(ComWhighlightRegion);
    XDestroyRegion(ComWsetRegion);
}


static void Resize(w)
Widget w;
{
	UKCCommandWidget cbw = (UKCCommandWidget)w;
	HighlightRegion(cbw);
	SetRegion(cbw);
	(*SuperClass->core_class.resize)(w);
}

/*
 * Set specified arguments into widget
 */
/* ARGSUSED */
static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    UKCCommandWidget oldcbw = (UKCCommandWidget) current;
    UKCCommandWidget cbw = (UKCCommandWidget) new;
    Boolean redisplay = False;

    if (XtCField(oldcbw,sensitive) != ComWsensitive && !ComWsensitive) {
	/* about to become insensitive */
	ComWset = FALSE;
	ComWhighlighted = FALSE;
    }

    if (XtCField(oldcbw,width) != ComWwidth ||
	XtCField(oldcbw,height) != ComWheight ||
	XtCBField(oldcbw,highlight_thickness) != ComWhighlightThickness)
    {
	SetRegion(cbw);
	HighlightRegion(cbw);
    } else {
	if (oldcbw->label.left_pixmap != cbw->label.left_pixmap
	    || oldcbw->label.right_pixmap != cbw->label.right_pixmap
	    || oldcbw->label.left_bitmap != cbw->label.left_bitmap
	    || oldcbw->label.right_bitmap != cbw->label.right_bitmap)
		SetRegion(cbw);
    }

    if (XtLField(oldcbw,foreground) != ComWforeground ||
	XtCField(oldcbw,background_pixel) != ComWbackground ||
	XtLField(oldcbw,font) != ComWfont)
    {
	if (ComWset)		/* Label has release one of these */
	    XtReleaseGC(new, ComWnormalGC);
	else
	    XtReleaseGC(new, ComWinverseGC);
	ComWnormalGC = Get_GC(cbw, ComWforeground, ComWbackground);
	ComWinverseGC = Get_GC(cbw, ComWbackground, ComWforeground);
	redisplay = True;
    }

    if (XtCBField(oldcbw,highlight_color) != ComWhighlightColor ||
	XtCBField(oldcbw,highlight_thickness) != ComWhighlightThickness)
    {
	XtReleaseGC(new, ComWhighlightGC);
	ComWhighlightGC = Get_GC(cbw, ComWhighlightColor, ComWbackground);
    }

    ComWlabelGC = ComWset ? ComWinverseGC : ComWnormalGC;

    return (redisplay ||
	    XtCField(oldcbw, sensitive) != ComWsensitive ||
	    XtCBField(oldcbw, set) != ComWset ||
	    XtCBField(oldcbw, highlighted) != ComWhighlighted);
}


static XukcUpdateBitmapProc
UpdateBitmap(w, new_bitmap, is_left)
Widget w;
Pixmap new_bitmap;
Boolean is_left;
{
	/* always do the superclass's update_bitmap method first */
	(*SuperClass->label_class.update_bitmap)(w, new_bitmap, is_left);
	SetRegion((UKCCommandWidget)w);
	HighlightRegion((UKCCommandWidget)w);
}
