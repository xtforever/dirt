#ifndef lint
/* Derived from XConsortium: Label.c,v 1.62 88/10/21 08:15:13 swick Exp */
/* and from $Xukc: Label.c,v 1.13 91/12/17 10:01:14 rlh2 Rel */
static char sccsid[] = "@(#)Label.c	1.2 (UKC) 5/10/92";
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
 * Label.c - UKCLabel widget
 */

#define XtStrlen(s)		((s) ? strlen(s) : 0)

#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/Drawing.h>
#include <X11/Xukc/Convert.h>
#include "LabelP.h"

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

#define offset(field) XtOffset(UKCLabelWidget, label.field)
static XtResource resources[] = {
    {XtNbitmap, XtCBitmap, XtRBitmap, sizeof(Pixmap),
	offset(bitmap), XtRImmediate, (XtPointer)None},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(foreground), XtRString, XtExtdefaultforeground },
    {XtNfont,  XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(font),XtRString, XtDefaultFont},
    {XtNlabel,  XtCLabel, XtRString, sizeof(String),
	offset(label), XtRString, NULL},
    {XtNoldLabel, XtCOldLabel, XtRString, sizeof(String),
	offset(old_label), XtRString, NULL},
    {XtNjustify, XtCJustify, XtRJustify, sizeof(XtJustify),
	offset(justify), XtRImmediate, (XtPointer)XtJustifyCenter},
    {XtNinternalWidth, XtCWidth, XtRDimension,  sizeof(Dimension),
	offset(internal_width), XtRImmediate, (XtPointer)4},
    {XtNinternalHeight, XtCHeight, XtRDimension, sizeof(Dimension),
	offset(internal_height), XtRImmediate, (XtPointer)2},
    {XtNresize, XtCResize, XtRBoolean, sizeof(Boolean),
	offset(resize), XtRImmediate, (XtPointer)FALSE},

/* UKC Added resources */
    {XtNleftBitmap, XtCLeftBitmap, XtRBitmap, sizeof(Pixmap),
	offset(left_bitmap), XtRImmediate, (XtPointer)None},
    {XtNrightBitmap, XtCRightBitmap, XtRBitmap, sizeof(Pixmap),
	offset(right_bitmap), XtRImmediate, (XtPointer)None},
    {XtNpixmap, XtCPixmap, XtRPixmap, sizeof(Pixmap),
	offset(pixmap), XtRImmediate, (XtPointer)None},
    {XtNleftPixmap, XtCLeftPixmap, XtRPixmap, sizeof(Pixmap),
	offset(left_pixmap), XtRImmediate, (XtPointer)None},
    {XtNrightPixmap, XtCRightPixmap, XtRPixmap, sizeof(Pixmap),
	offset(right_pixmap), XtRImmediate, (XtPointer)None},
    {XtNoverflowRight, XtCOverflowRight, XtRBoolean, sizeof(Boolean),
	offset(overflow_right), XtRImmediate, (XtPointer)FALSE},
    {XtNoverflowTop, XtCOverflowTop, XtRBoolean, sizeof(Boolean),
	offset(overflow_top), XtRImmediate, (XtPointer)FALSE},
    {XtNverticalJustify, XtCVerticalJustify, XtRVerticalJustify,
	sizeof(int), offset(vert_justify), XtRImmediate,
	(XtPointer)XukcEcenter},
    {XtNeditable, XtCEditable, XtRBoolean, sizeof(Boolean),
	offset(is_editable), XtRImmediate, (XtPointer)FALSE},
    {XtNbarColour, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(bar_colour), XtRString, XtExtdefaultforeground},
    {XtNediting, XtCEditing, XtRBoolean, sizeof(Boolean),
	offset(editing), XtRImmediate, (XtPointer)FALSE},
    {XtNeditCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	offset(edit_callback), XtRString, (XtPointer)NULL},
    {XtNgrabKeyboard, XtCGrabKeyboard, XtRBoolean, sizeof(Boolean),
	offset(grab_keyboard), XtRImmediate, (XtPointer)FALSE},
};
#undef offset

static Region empty_region = NULL;

static void Initialize();
static void Realize();
static void Resize();
static void Redisplay();
static Boolean SetValues();
static void ClassInitialize();
static void ClassPartInitialize();
static void Destroy();
static XtGeometryResult QueryGeometry();
static XukcUpdateBitmapProc UpdateBitmap();

/* Actions */
static void insert_char();
static void delete_char();
static void position_bar();
static void label_edit();
static void undo_edit();
static void label_update();

static XtActionsRec actions[] = {
	{ "insert_char", insert_char },
	{ "delete_char", delete_char },
	{ "position_bar", position_bar },
	{ "label_edit", label_edit },
	{ "label_update", label_update },
	{ "undo_edit", undo_edit }
};

#define DELETE_ALL "all"
#define BEGINNING "beginning"
#define END "end"

static char translations[] = "\
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
	:<Key>: insert_char()\n\
";

#define Superclass	(&simpleClassRec)

UKCLabelClassRec ukcLabelClassRec = {
  {
/* core_class fields */	
    /* superclass	  	*/	(WidgetClass) Superclass,
    /* class_name	  	*/	"UKCLabel",
    /* widget_size	  	*/	sizeof(UKCLabelRec),
    /* class_initialize   	*/	ClassInitialize,
    /* class_part_initialize	*/	ClassPartInitialize,
    /* class_inited       	*/	FALSE,
    /* initialize	  	*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize		  	*/	XtInheritRealize,
    /* actions		  	*/	actions,
    /* num_actions	  	*/	XtNumber(actions),
    /* resources	  	*/	resources,
    /* num_resources	  	*/	XtNumber(resources),
    /* xrm_class	  	*/	NULLQUARK,
    /* compress_motion	  	*/	TRUE,
    /* compress_exposure  	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest	  	*/	FALSE,
    /* destroy		  	*/	Destroy,
    /* resize		  	*/	Resize,
    /* expose		  	*/	Redisplay,
    /* set_values	  	*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus	 	*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private   	*/	NULL,
    /* tm_table		   	*/	translations,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL,
  },{ /* simple class */
    /* no class fields */		XtInheritChangeSensitive
  },{ /* label class */
    /* update_bitmap */			(XukcUpdateBitmapProc)UpdateBitmap
  }
};

WidgetClass ukcLabelWidgetClass = (WidgetClass)&ukcLabelClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static EnumListRec xukc_justify[] = {
	{ XukcJustifyTop, (int)XukcEtop },
	{ XukcJustifyCenter, (int)XukcEcenter },
	{ XukcJustifyBottom, (int)XukcEbottom }
};

static XtConvertArgRec xukc_justify_Args[] = {
	{ XtAddress,    (XtPointer)xukc_justify, sizeof(EnumList) },
	{ XtImmediate, (XtPointer)XtNumber(xukc_justify), sizeof(Cardinal) },
	{ XtAddress,    XtRVerticalJustify, sizeof(String) },
};

static XtConvertArgRec pixmap_Args[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.screen),
						sizeof(Screen *)},
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.widget_class),
						sizeof(WidgetClass)},
    {XtWidgetBaseOffset, (XtPointer)XtOffset(UKCLabelWidget, label.foreground),
						sizeof(Pixel)},
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.background_pixel),
						sizeof(Pixel)}
};

#define BAR_WIDTH 2
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define LEFT_WIDTH (lw->label.left_width + (lw->label.left_width > 0 ? \
				lw->label.internal_width : 0))
#define RIGHT_WIDTH (lw->label.right_width + (lw->label.right_width > 0 ? \
				lw->label.internal_width : 0))

static void
ClassInitialize()
{
	static XtConvertArgRec screenConvertArg[] = {
		{XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.screen),
		     sizeof(Screen *)}
	};

#if 1 /* ||| Change this when Xmu gets updated to R4 ||| */
	XtAddConverter(XtRString, XtRJustify, XmuCvtStringToJustify, NULL, 0);
	XtAddConverter(XtRString, XtRBitmap, XmuCvtStringToBitmap,
			screenConvertArg, XtNumber(screenConvertArg));
#else
	XtSetTypeConverter(XtRString, XtRJustify, XmuCvtStringToJustify,
			(XtConvertArgList)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
	XtSetTypeConverter(XtRString, XtRBitmap, XmuCvtStringToBitmap,
			screenConvertArg, XtNumber(screenConvertArg),
#ifdef CACHE_REF_COUNT
                        XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay, (XtDestructor)NULL);
#endif
	XtSetTypeConverter(XtRString, XtRVerticalJustify,
			XukcCvtStringToNamedValue,
			xukc_justify_Args, XtNumber(xukc_justify_Args),
#ifdef CACHE_REF_COUNT
                        XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
	XtSetTypeConverter(XtRVerticalJustify, XtRString,
			XukcCvtNamedValueToString,
			xukc_justify_Args, XtNumber(xukc_justify_Args),
#ifdef CACHE_REF_COUNT
                        XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
	XtSetTypeConverter(XtRString, XtRPixmap, XukcCvtStringToPixmap,
			pixmap_Args, XtNumber(pixmap_Args),
#ifdef CACHE_REF_COUNT
                        XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay, (XtDestructor)NULL);

	empty_region = XCreateRegion();
} /* ClassInitialize */


/* do inheritance chaining of UpdateBitmap method */
static void
ClassPartInitialize(wc)
WidgetClass wc;
{
	UKCLabelWidgetClass lwc = (UKCLabelWidgetClass)wc;
	UKCLabelWidgetClass super =
			   (UKCLabelWidgetClass)lwc->core_class.superclass;

	if (lwc->label_class.update_bitmap == XtInheritUpdateBitmap)
	    lwc->label_class.update_bitmap = super->label_class.update_bitmap;
}


static void
preferred_size(lw, width, height)
UKCLabelWidget lw;
Dimension *width, *height;
{
	/* Preferred size is calculated from the size of the text or bitmap
	 * of the label plus the size of any left or right bitmaps plus
	 * some spacing amounts.
	 */
	*width = lw->label.internal_width + lw->label.label_width +
		 (lw->label.label_width > 0 ? lw->label.internal_width : 0) +
		 LEFT_WIDTH + RIGHT_WIDTH;
	*height = MAX(lw->label.label_height, lw->label.left_height);
	*height = MAX(*height, lw->label.right_height);
	*height += (2 * lw->label.internal_height);
}

/*
 * Calculate width and height of displayed text in pixels
 */
static void
SetTextWidthAndHeight(lw)
UKCLabelWidget lw;
{
    register XFontStruct	*fs = lw->label.font;
    Window root;
    int x, y;
    unsigned int width, height, bw, depth;
    char *nl;

    lw->label.left_width = lw->label.right_width = 0;
    lw->label.left_height = lw->label.right_height = 0;
    lw->label.left_depth = lw->label.right_depth = 0;

    if (lw->core.depth > 1 && lw->label.left_pixmap != None) {
	if (XGetGeometry(XtDisplay(lw), lw->label.left_pixmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.left_width = width;
	    lw->label.left_height = height;
	    lw->label.left_depth = depth;
	}
    } else if (lw->label.left_bitmap != None) {
	if (XGetGeometry(XtDisplay(lw), lw->label.left_bitmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.left_width = width;
	    lw->label.left_height = height;
	    lw->label.left_depth = depth;
	}
    }

    if (lw->core.depth > 1 && lw->label.right_pixmap != None) {
	if (XGetGeometry(XtDisplay(lw), lw->label.right_pixmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.right_width = width;
	    lw->label.right_height = height;
	    lw->label.right_depth = depth;
	}
    } else if (lw->label.right_bitmap != None) {
	if (XGetGeometry(XtDisplay(lw), lw->label.right_bitmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.right_width = width;
	    lw->label.right_height = height;
	    lw->label.right_depth = depth;
	}
    }

    if (lw->core.depth > 1 && lw->label.pixmap != None) {
	if (XGetGeometry(XtDisplay(lw), lw->label.pixmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.label_height = height;
	    lw->label.label_width = width;
	    lw->label.label_len = depth;
	    return;
	}
    } else if (lw->label.bitmap != None) {
	if (XGetGeometry(XtDisplay(lw), lw->label.bitmap, &root, &x, &y,
			 &width, &height, &bw, &depth)) {
	    lw->label.label_height = height;
	    lw->label.label_width = width;
	    lw->label.label_len = depth;
	    return;
	}
    }

    lw->label.label_len = XtStrlen(lw->label.label);
    lw->label.label_height = fs->max_bounds.ascent + fs->max_bounds.descent;

    if (lw->label.label_len == 0) {
	lw->label.label_width = 0;
	return;
    }

    if ((nl = index(lw->label.label, '\n')) != NULL) {
	char *label;

	lw->label.label_width = 0;
	for (label = lw->label.label; nl != NULL; nl = index(label, '\n')) {
	    int width = XTextWidth(fs, label, (int)(nl - label));
	    if (width > lw->label.label_width) lw->label.label_width = width;
	    label = nl + 1;
	    if (*label)
		lw->label.label_height +=
		    fs->max_bounds.ascent + fs->max_bounds.descent;
	}
	if (*label) {
	    int width = XTextWidth(fs, label, strlen(label));
	    if (width > lw->label.label_width) lw->label.label_width = width;
	}
    } else lw->label.label_width =
	    XTextWidth(fs, lw->label.label, (int) lw->label.label_len);
}

static void
GetnormalGC(lw)
UKCLabelWidget lw;
{
    XGCValues	values;

    values.foreground	= lw->label.foreground;
    values.background	= lw->core.background_pixel;
    values.font		= lw->label.font->fid;

    lw->label.normal_GC = XtGetGC((Widget)lw,
		(unsigned) GCForeground | GCBackground | GCFont,
		&values);
}

static void
GetstippleGC(lw)
UKCLabelWidget lw;
{
	XGCValues   values;

	values.function   = GXand;
	values.fill_style = FillTiled;
	values.tile       = XmuCreateStippledPixmap(XtScreen((Widget)lw),
			lw->label.foreground ^ lw->core.background_pixel,
			!(lw->label.foreground ^ lw->core.background_pixel),
					lw->core.depth);

	lw->label.stipple_GC = XtGetGC((Widget)lw,
				(unsigned) GCFillStyle | GCTile 
				| GCFunction, &values);
}


static void
GetgrayGC(lw)
UKCLabelWidget lw;
{
	XGCValues	values;

	values.foreground = lw->label.foreground;
	values.background = lw->core.background_pixel;
	values.font	  = lw->label.font->fid;
	values.fill_style = FillTiled;
	values.tile       = XmuCreateStippledPixmap(XtScreen((Widget)lw),
						lw->label.foreground, 
						lw->core.background_pixel,
						lw->core.depth);
	lw->label.gray_GC = XtGetGC((Widget)lw,
				(unsigned) GCForeground | GCBackground |
					   GCFont | GCTile | GCFillStyle,
				&values);
}

static void
GetbarGC(lw)
UKCLabelWidget lw;
{
	XGCValues   values;

	values.foreground = lw->label.bar_colour;
	values.background = lw->core.background_pixel;
	values.line_width = BAR_WIDTH;
	values.function   = GXinvert;

	lw->label.bar_GC = XtGetGC((Widget)lw,
				(unsigned) GCForeground | GCBackground |
					   GCLineWidth | GCFunction,
				&values);
}

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCLabelWidget lw = (UKCLabelWidget) new;
	Dimension dummy;

	if (lw->label.label == NULL) 
		lw->label.label = XtNewString(lw->core.name);
	else {
		String label = XtNewString(lw->label.label);
		lw->label.label = label;
	}

	SetTextWidthAndHeight(lw);
	GetnormalGC(lw);
	GetgrayGC(lw);
	GetstippleGC(lw);
	GetbarGC(lw);
	lw->label.left_region = lw->label.right_region = NULL;
	lw->label.old_label = XtNewString(lw->label.label);
	lw->label.bar_char = 0;

	if (lw->core.width == 0 || lw->core.height == 0)
		preferred_size(lw, (lw->core.width == 0 ? &(lw->core.width)
			: &dummy),
			(lw->core.height == 0 ? &(lw->core.height) : &dummy));
	Resize((Widget)lw);
}


static void
draw_label(lw)
UKCLabelWidget lw;
{
	char *nl, *label;
	Position y;

	label = lw->label.label;
	y = lw->label.label_y;
	while ((nl = index(label, '\n')) != NULL) {
	       XDrawString(XtDisplay(lw), XtWindow(lw),
			XtIsSensitive((Widget)lw) ? lw->label.normal_GC
						  : lw->label.gray_GC,
			  lw->label.label_x,
			  y + lw->label.font->max_bounds.ascent,
			  label, (int)(nl - label));
	       y += lw->label.font->max_bounds.ascent + lw->label.font->max_bounds.descent;
	       label = nl + 1;
	}
	XDrawString(XtDisplay(lw), XtWindow(lw),
		XtIsSensitive((Widget)lw) ? lw->label.normal_GC
					  : lw->label.gray_GC,
			lw->label.label_x,
			y + lw->label.font->max_bounds.ascent,
			label, strlen(label));

}


static void
clear_label(lw, exposures)
UKCLabelWidget lw;
Bool exposures;
{
	XClearArea(XtDisplay(lw), XtWindow(lw),
			lw->label.label_x - (BAR_WIDTH/2),
			lw->label.label_y,
			lw->label.label_width + BAR_WIDTH,
#if 0
			lw->label.label_height, exposures);
#else
			lw->label.font->max_bounds.ascent
			+ lw->label.font->max_bounds.descent, exposures);
#endif
}


static void
draw_bar(lw)
UKCLabelWidget lw;
{
	XDrawLine(XtDisplay(lw), XtWindow(lw), lw->label.bar_GC,
			lw->label.bar_x,
			lw->label.label_y,
			lw->label.bar_x,
			lw->label.label_y + lw->label.font->max_bounds.ascent
			+ lw->label.font->max_bounds.descent);
}


/* ARGSUSED */
static void Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
   UKCLabelWidget lw = (UKCLabelWidget) w;
   GC gc;
   static Region scratch = NULL;
   Pixmap pixmap, left_pixmap, right_pixmap;

   gc = XtIsSensitive(w) ? lw->label.normal_GC : lw->label.gray_GC;

   if (lw->core.depth > 1) {
	pixmap = lw->label.pixmap != None
			? lw->label.pixmap : lw->label.bitmap;
	left_pixmap = lw->label.left_pixmap != None
			? lw->label.left_pixmap : lw->label.left_bitmap;
	right_pixmap = lw->label.right_pixmap != None
			? lw->label.right_pixmap : lw->label.right_bitmap;
   } else {
	pixmap = lw->label.bitmap;
	left_pixmap = lw->label.left_bitmap;
	right_pixmap = lw->label.right_bitmap;
   }

   /* Repaint the Label String or Bitmap/Pixmap */
   if (region == NULL ||
       XRectInRegion(region, lw->label.label_x, lw->label.label_y,
       lw->label.label_width == 0 ? BAR_WIDTH : lw->label.label_width,
       lw->label.label_height) != RectangleOut) {
	   if (pixmap == None) {
		if (lw->label.label_len > 0)
			draw_label(lw);
		if (lw->label.editing && XtIsSensitive(w))
			draw_bar(lw);
	   } else {
	       if (lw->label.label_len == 1) { /* depth */
		  XCopyPlane(XtDisplay(w), pixmap, XtWindow(w), gc,
		  0, 0, lw->label.label_width, lw->label.label_height,
		  lw->label.label_x, lw->label.label_y, 1L);
	       } else {
		 XCopyArea(XtDisplay(w), pixmap, XtWindow(w), gc,
		 0, 0, lw->label.label_width, lw->label.label_height,
		 lw->label.label_x, lw->label.label_y);
	       }
	       if (!XtIsSensitive(w))
			XFillRectangle(XtDisplay(w), XtWindow(w),
				lw->label.stipple_GC,
				lw->label.label_x, lw->label.label_y,
				lw->label.label_width, lw->label.label_height);
	   }
   }

   if (scratch == NULL)
	scratch = XCreateRegion();

   if (region != NULL && left_pixmap != None)
	XIntersectRegion(region, lw->label.left_region, scratch);

   /* Repaint the left hand bitmap */
   if (left_pixmap != None &&
       (region == NULL || !XEmptyRegion(scratch))) {
	if (lw->label.left_depth == 1)
	       XCopyPlane(XtDisplay(w), left_pixmap, XtWindow(w),
				gc, 0, 0,
				lw->label.left_width, lw->label.left_height,
				lw->label.internal_width,
				(lw->core.height - lw->label.left_height)/2,
				1L);
	else
	       XCopyArea(XtDisplay(w), left_pixmap, XtWindow(w),
				gc, 0, 0,
				lw->label.left_width, lw->label.left_height,
				lw->label.internal_width,
				(lw->core.height - lw->label.left_height)/2);
	if (!XtIsSensitive(w))
		XFillRectangle(XtDisplay(w), XtWindow(w),
			lw->label.stipple_GC,
			lw->label.internal_width, (lw->core.height -
			lw->label.left_height)/2,
			lw->label.left_width, lw->label.left_height);
   }

   /* Repaint the right hand bitmap */
   if (region != NULL && right_pixmap != None)
	   XIntersectRegion(region, lw->label.right_region, scratch);

   if (right_pixmap != None &&
       (region == NULL || !XEmptyRegion(scratch))) {
	if (lw->label.right_depth == 1)
	       XCopyPlane(XtDisplay(w), right_pixmap, XtWindow(w),
				gc, 0, 0,
				lw->label.right_width, lw->label.right_height,
				lw->core.width - lw->label.right_width -
					lw->label.internal_width,
				(lw->core.height - lw->label.right_height)/2,
				1L);
	else
	       XCopyArea(XtDisplay(w), right_pixmap, XtWindow(w),
				gc, 0, 0,
				lw->label.right_width, lw->label.right_height,
				lw->core.width - lw->label.right_width -
					lw->label.internal_width,
				(lw->core.height - lw->label.right_height)/2);
	if (!XtIsSensitive(w))
		XFillRectangle(XtDisplay(w), XtWindow(w),
			lw->label.stipple_GC,
			lw->core.width - lw->label.right_width -
					 lw->label.internal_width,
			(lw->core.height - lw->label.right_height)/2,
			lw->label.right_width, lw->label.right_height);
   }
}


static void
set_bitmap_regions(lw)
UKCLabelWidget lw;
{
	XRectangle rect;

	if (lw->label.left_pixmap != None ||
	    lw->label.left_bitmap != None) {
		if (lw->label.left_region == NULL)
			lw->label.left_region = XCreateRegion();
		rect.x = lw->label.internal_width;
		rect.y = (lw->core.height - lw->label.left_height)/2;
		rect.width = lw->label.left_width;
		rect.height = lw->label.left_height;
		XUnionRectWithRegion(&rect, empty_region,
					   lw->label.left_region);
	} else {
		if (lw->label.left_region != NULL) {
			XDestroyRegion(lw->label.left_region);
			lw->label.left_region = NULL;
		}
	}

	if (lw->label.right_pixmap != None ||
	    lw->label.right_bitmap != None) {
		if (lw->label.right_region == NULL)
			lw->label.right_region = XCreateRegion();
		rect.x = lw->core.width - lw->label.right_width -
			 lw->label.internal_width;
		rect.y = (lw->core.height - lw->label.right_height)/2;
		rect.width = lw->label.right_width;
		rect.height = lw->label.right_height;
		XUnionRectWithRegion(&rect, empty_region,
					   lw->label.right_region);
	} else {
		if (lw->label.right_region != NULL) {
			XDestroyRegion(lw->label.right_region);
			lw->label.right_region = NULL;
		}
	}
}


static void
set_bar_position_by_character(lw, cp)
UKCLabelWidget lw;
Cardinal cp;
{
	Position pos = 0;

	if (lw->label.label_len == 0)
		cp = 0;
		
	if (cp > 0)
		pos = XTextWidth(lw->label.font, lw->label.label, cp);
		
	lw->label.bar_x = pos + lw->label.label_x;
	lw->label.bar_char = cp;
}


static void
set_bar_position_by_pixel(lw, x, y)
UKCLabelWidget lw;
Position x, y;
{
#define CHAR_WIDTH(ch_num) \
	(lw->label.font->per_char[lw->label.label[ch_num]].width)

	if (x <= lw->label.label_x) {
		/* position bar as start of label */
		lw->label.bar_x = lw->label.label_x;
		lw->label.bar_char = 0;
	} else if (x >= (lw->label.label_x + lw->label.label_width)) {
		/* position bar at end of label */
		lw->label.bar_x = lw->label.label_x + lw->label.label_width;
		lw->label.bar_char = lw->label.label_len;
	} else {
		/* find closest character to position bar next to */
		Position pos = 0;
		Cardinal ch_num;

		x -= lw->label.label_x;

		for (ch_num = 1; ch_num <= lw->label.label_len; ch_num++) {
#ifdef notdef /* this should work and be quicker but it doesn't 8-( */
			if ((pos + (CHAR_WIDTH(ch_num)/2)) > x)
				break;
			pos += CHAR_WIDTH(ch_num);
#else
			if ((pos = XTextWidth(lw->label.font, lw->label.label, ch_num)) > x)
				break;
#endif
		}
		lw->label.bar_x = pos + lw->label.label_x;
		lw->label.bar_char = ch_num;
	}
}


static void
RepositionLabel(lw, width, height)
register UKCLabelWidget lw;
Dimension width, height;
{
	Position newPos;

	switch (lw->label.justify) {
	    case XtJustifyLeft   :
		newPos = lw->label.internal_width + LEFT_WIDTH;
		if (lw->label.overflow_right || (lw->label.label_width +
			LEFT_WIDTH + RIGHT_WIDTH + 2*lw->label.internal_width)
			<= width)
			    break;
	    case XtJustifyRight  :
		newPos = width - lw->label.internal_width
				- lw->label.label_width - RIGHT_WIDTH;
		break;
	    case XtJustifyCenter :
		newPos = (width - lw->label.label_width - lw->label.left_width -
				     lw->label.right_width) / 2 + LEFT_WIDTH;
		break;
	}
	if (newPos < (Position)(lw->label.internal_width + LEFT_WIDTH)) {
		if (lw->label.overflow_right)
			newPos = lw->label.internal_width + LEFT_WIDTH;
		else if (lw->label.justify == XtJustifyCenter)
			newPos = width - lw->label.internal_width
				- lw->label.label_width - RIGHT_WIDTH;
	}
	lw->label.label_x = newPos;
	switch (lw->label.vert_justify) {
	    case XukcEtop :
		newPos = lw->label.internal_height;
		break;
	    case XukcEcenter :
		newPos = (height - lw->label.label_height) / 2;
		break;
	    case XukcEbottom :
		newPos = height - lw->label.internal_height
				- lw->label.label_height;
		break;
	}
	if (newPos < (Position)(lw->label.internal_height)) {
		if (!lw->label.overflow_top)
			newPos = lw->label.internal_height;
		else if (lw->label.justify == XtJustifyCenter)
			newPos = height - lw->label.internal_height
				- lw->label.label_height;
	}
	lw->label.label_y = newPos;
	set_bar_position_by_character(lw, lw->label.bar_char);
}


static void Resize(w)
Widget w;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;

	RepositionLabel(lw, w->core.width, w->core.height);
	set_bitmap_regions(lw);
	if (XtIsRealized(w))
		Redisplay(w, (XEvent *)NULL, (Region)NULL);
}

/*
 * Set specified arguments into widget
 */

/* ARGSUSED */
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
    UKCLabelWidget curlw = (UKCLabelWidget) current;
    UKCLabelWidget reqlw = (UKCLabelWidget) request;
    UKCLabelWidget newlw = (UKCLabelWidget) new;
    Boolean was_resized = False;
    Boolean redisplay = False;

    if (newlw->label.label == NULL)
	newlw->label.label = XtNewString(newlw->core.name);

    if (curlw->label.label != newlw->label.label) {
	String new_label;
	XtFree(curlw->label.label);

	new_label = XtNewString(newlw->label.label);
	newlw->label.label = new_label;

	was_resized = True;
	XtFree(newlw->label.old_label);
	newlw->label.old_label = XtNewString(newlw->label.label);
    }

    if (was_resized
	|| (curlw->label.font != newlw->label.font)
	|| (curlw->label.left_bitmap != newlw->label.left_bitmap)
	|| (curlw->label.right_bitmap != newlw->label.right_bitmap)
	|| (curlw->label.bitmap != newlw->label.bitmap)
	|| (curlw->label.left_pixmap != newlw->label.left_pixmap)
	|| (curlw->label.right_pixmap != newlw->label.right_pixmap)
	|| (curlw->label.pixmap != newlw->label.pixmap)) {
		SetTextWidthAndHeight(newlw);
		was_resized = True;
	}

    /* calculate the window size */
    if (newlw->label.resize) {
	Dimension dummy;
	if (curlw->core.width == reqlw->core.width)
		preferred_size(newlw, &(newlw->core.width), &dummy);

	if (curlw->core.height == reqlw->core.height)
		preferred_size(newlw, &dummy, &(newlw->core.height));
    }

    if (curlw->label.foreground != newlw->label.foreground
	|| curlw->label.font->fid != newlw->label.font->fid
	|| curlw->core.background_pixel != newlw->core.background_pixel) {
	XtReleaseGC(new, curlw->label.normal_GC);
	XtReleaseGC(new, curlw->label.gray_GC);
	XtReleaseGC(new, curlw->label.stipple_GC);
	GetnormalGC(newlw);
	GetgrayGC(newlw);
	GetstippleGC(newlw);
	redisplay = True;
    }

    if (curlw->label.editing != newlw->label.editing) {
	if (!newlw->label.is_editable)
		newlw->label.editing = FALSE;
	else
		redisplay = TRUE;
    }

    if (curlw->label.is_editable != newlw->label.is_editable) {
	newlw->label.editing = FALSE;
	redisplay = True;
    }

    if ((curlw->label.vert_justify != newlw->label.vert_justify)
	|| (curlw->label.justify != newlw->label.justify))
		redisplay = True;

    if (curlw->label.bar_colour != newlw->label.bar_colour
	|| curlw->core.background_pixel != newlw->core.background_pixel) {
	XtReleaseGC(new, curlw->label.bar_GC);
	GetbarGC(newlw);
	redisplay |= newlw->label.editing;
    }

    if ((curlw->label.internal_width != newlw->label.internal_width)
        || (curlw->label.internal_height != newlw->label.internal_height)
	|| (curlw->label.justify != newlw->label.justify)
	|| (curlw->label.vert_justify != newlw->label.vert_justify)
	|| (curlw->label.overflow_right != newlw->label.overflow_right)
	|| (curlw->label.overflow_top != newlw->label.overflow_top)
	|| was_resized) {
	/* Resize() will be called if geometry changes succeed */
	RepositionLabel(newlw, newlw->core.width, newlw->core.height);
	set_bitmap_regions(newlw);
    }

    if (newlw->label.editing &&
	(curlw->label.grab_keyboard != newlw->label.grab_keyboard)) {
	Widget top = new;

	while (XtParent(top) != NULL)
		top = XtParent(top);
	if (XtIsRealized(top))
		XtSetKeyboardFocus(top, newlw->label.grab_keyboard ?
							new : (Widget)None);
    }

    return was_resized || redisplay ||
	   XtIsSensitive(current) != XtIsSensitive(new);
}


static void Destroy(w)
Widget w;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;

	XtFree(lw->label.label);
	XtFree(lw->label.old_label);
	XtReleaseGC(w, lw->label.normal_GC);
	XtReleaseGC(w, lw->label.gray_GC);
	XtReleaseGC(w, lw->label.stipple_GC);
	if (lw->label.left_region != NULL)
		XDestroyRegion(lw->label.left_region);
	if (lw->label.right_region != NULL)
		XDestroyRegion(lw->label.right_region);
}


static XtGeometryResult QueryGeometry(w, intended, preferred)
Widget w;
XtWidgetGeometry *intended, *preferred;
{
    preferred->request_mode = CWWidth | CWHeight;
    preferred_size((UKCLabelWidget)w, &(preferred->width),
			&(preferred->height));

    if (  ((intended->request_mode & (CWWidth | CWHeight))
	   	== (CWWidth | CWHeight)) &&
	  intended->width == preferred->width &&
	  intended->height == preferred->height)
	return XtGeometryYes;
    else if (preferred->width == w->core.width &&
	     preferred->height == w->core.height)
	return XtGeometryNo;
    else
	return XtGeometryAlmost;
}


static XukcUpdateBitmapProc
UpdateBitmap(w, new_bitmap, is_left)
Widget w;
Pixmap new_bitmap;
Boolean is_left;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;
	Dimension dummy;

	if (is_left)
		lw->label.left_bitmap = new_bitmap;
	else
		lw->label.right_bitmap = new_bitmap;

	SetTextWidthAndHeight(lw);

	/* hook for subclass initialization */
	if (lw->core.width == 0 || lw->core.height == 0)
		preferred_size(lw, (lw->core.width == 0 ? &(lw->core.width)
			: &dummy),
			(lw->core.height == 0 ? &(lw->core.height) : &dummy));
	RepositionLabel(lw, w->core.width, w->core.height);
	set_bitmap_regions(lw);

        /* this generates an exposure event within the new pixmaps region
	 * so only the new pixmap is redisplayed. */
	if (XtIsRealized(w)) {
		XRectangle rect;

		XClipBox(is_left ? lw->label.left_region :
				   lw->label.right_region, &rect);

		XClearArea(XtDisplay(w), XtWindow(w),
				rect.x, rect.y, 1, 1, TRUE);
	}
}

/****** Editing of Label routines ******/

static void
insert_char(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;
	char strbuf[128];
	String new_label;
	int len, i;
	KeySym keycode;
	XComposeStatus compose_status;

	if (!lw->label.is_editable || !lw->label.editing)
		return;

	if ((len = XLookupString((XKeyEvent *)event, strbuf, 127, &keycode,
	     &compose_status)) == 0)
		return;

	new_label = XtMalloc(lw->label.label_len + len + 1);

	/* copy over anything to the left of the insertion bar */
	for (i = 0; i < lw->label.bar_char; i++)
		new_label[i] = lw->label.label[i];

	/* copy over the newly inserted characters */
	for (i = 0; i < len; i++)
		new_label[lw->label.bar_char + i] = strbuf[i];

	/* copy over anything to the right of the insertion bar */
	for (i = lw->label.bar_char; i < lw->label.label_len; i++)
		new_label[len + i] = lw->label.label[i];

	XtFree(lw->label.label);
	new_label[lw->label.label_len + len] = '\0';
	lw->label.bar_char++;
	lw->label.label = new_label;
	SetTextWidthAndHeight(lw);	
	RepositionLabel(lw, lw->core.width, lw->core.height);

	if (XtIsRealized(w))
		clear_label(lw, TRUE);
}


static void
delete_char(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;
	Cardinal i;

	if (!lw->label.is_editable || !lw->label.editing || !lw->label.bar_char)
		return;

	if (*num_params == 1 && strcmp(params[0], DELETE_ALL) == 0) {
		lw->label.label[0] = '\0';
		lw->label.label_len = 0;
		lw->label.bar_char = 0;
	} else {
		for (i = lw->label.bar_char; i < lw->label.label_len; i++)
			lw->label.label[i-1] = lw->label.label[i];
		lw->label.label[--lw->label.label_len] = '\0';
		lw->label.bar_char--;
	}

	if (XtIsRealized(w))
		clear_label(lw, FALSE);

	SetTextWidthAndHeight(lw);	
	RepositionLabel(lw, lw->core.width, lw->core.height);

	if (XtIsRealized(w))
		clear_label(lw, TRUE);
}


static void
position_bar(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;
	Cardinal old_bar, new_bar;
	Position old_xpos, new_xpos;
	Position old_ypos, new_ypos;

	if (!lw->label.is_editable || !lw->label.editing)
		return;

	old_xpos = lw->label.bar_x;
	old_ypos = lw->label.bar_y;
	old_bar = lw->label.bar_char;
	if (event->type == MotionNotify)
		set_bar_position_by_pixel(lw, event->xmotion.x,
						event->xmotion.y);
	else if (*num_params == 1) {
		int new_pos;
		
		if (strcmp(params[0], BEGINNING) == 0) {
			set_bar_position_by_character(lw, 0);
		} else if (strcmp(params[0], END) == 0) {
			set_bar_position_by_character(lw, lw->label.label_len);
		} else {
			new_pos = atoi(params[0]);

			if ((new_pos + lw->label.bar_char) < 0)
				set_bar_position_by_character(lw, 0);
			else
				set_bar_position_by_character(lw,
						new_pos + lw->label.bar_char);
		}
	} else
		return; /* error?? */
	new_bar = lw->label.bar_char;
	new_xpos = lw->label.bar_x;
	new_ypos = lw->label.bar_y;

	if (old_bar == new_bar)
		return; /* optimize bar redraw */

	lw->label.bar_x = old_xpos;
	lw->label.bar_y = old_ypos;
	lw->label.bar_char = old_bar;
	draw_bar(lw);
	lw->label.bar_char = new_bar;
	lw->label.bar_x = new_xpos;
	lw->label.bar_y = new_ypos;
	draw_bar(lw);
}


static void
label_edit(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;
	Boolean start_editing;

	if (!lw->label.is_editable || (lw->label.pixmap != None))
		return;

	if (*num_params != 1) {
		XtAppWarning(XtWidgetToApplicationContext(w),
		"wrong number of parameters to label_edit() in UKCLabelWidget : Can be either (on) or (off)");
		return;
	}

	start_editing = (strcmp(params[0], "on") == 0);

	if (!start_editing && !lw->label.editing)
		return; /* return pressed whilst not editing? */
	else if (start_editing && lw->label.editing)
		draw_bar(lw);	/* erase current position of the bar */

	lw->label.editing = start_editing;

	if (start_editing) {
		/* save the label incase of an undo */
		XtFree(lw->label.old_label);
		lw->label.old_label = XtNewString(lw->label.label);
		set_bar_position_by_pixel(lw, event->xbutton.x,
						event->xbutton.y);
		if (XtIsRealized(w)) {
			draw_bar(lw);
			if (lw->label.grab_keyboard) {
				/* send all keyboard events to this editing
				 * label */
				Widget top = w;

				while (XtParent(top) != NULL)
					top = XtParent(top);
				XtSetKeyboardFocus(top, w);
			}
		}
	} else {
		if (XtIsRealized(w)) {
			draw_bar(lw);	/* erase position of the bar */
			if (lw->label.grab_keyboard) {
				/* remove any keyboard grab when editing
				 * finished */
				Widget top = w;

				while (XtParent(top) != NULL)
					top = XtParent(top);
				XtSetKeyboardFocus(top, (Widget)None);
			}
			if (lw->label.resize) {
				Dimension width, height;

				SetTextWidthAndHeight(lw);	
				preferred_size(lw, &width, &height);
				XtVaSetValues((Widget)lw, XtNwidth, width,
						  XtNheight, height, NULL);
			}
		}
	}
}


static void
label_update(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;

	if (!lw->label.is_editable || !lw->label.editing)
		return;

	if (XtHasCallbacks(w, XtNeditCallback) == XtCallbackHasSome)
		XtCallCallbacks(w, XtNeditCallback, (XtPointer)lw->label.label);
}


static void
undo_edit(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCLabelWidget lw = (UKCLabelWidget)w;

	if (!lw->label.is_editable || !lw->label.editing)
		return;

	if (XtIsRealized(w))
		clear_label(lw, FALSE);

	XtFree(lw->label.label);
	lw->label.label = XtNewString(lw->label.old_label);
	lw->label.label_len = strlen(lw->label.label);
	SetTextWidthAndHeight(lw);	
	RepositionLabel(lw, lw->core.width, lw->core.height);
	lw->label.editing = FALSE;

	if (XtIsRealized(w)) {
		clear_label(lw, TRUE);
		if (lw->label.grab_keyboard) {
			Widget top = w;

			while (XtParent(top) != NULL)
				top = XtParent(top);
			XtSetKeyboardFocus(top, (Widget)None);
		}
	}
}


/* Public Routines */

void
XukcLabelNewBitmap(w, new_bitmap, is_left)
Widget w;
Pixmap new_bitmap;
Boolean is_left;
{
	/* this calls this widgets original update bitmap routine and 
	 * chains back to the label class update bitmap routine. */
	(*((UKCLabelWidgetClass)XtClass(w))->label_class.update_bitmap)
						(w, new_bitmap, is_left);
}
