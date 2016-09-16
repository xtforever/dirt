#ifndef lint
static char rcsid[] =
   "$Xukc: Value.c,v 1.7 91/12/17 10:02:02 rlh2 Rel $";
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
 * Value.c -  UKCValue composite widget subclass of ukcLayout
 * Creates a layout with a text widget and some arrows to allow the
 * value in the text widget to be decremented or increment.
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Convert.h>
#include <X11/Xukc/ValueP.h>

static void ClassInitialize();
static void Initialize();
static void Realize();
static Boolean SetValues();
static void updateValue();

/****************************************************************
 *
 * UKCValue Resources
 *
 ****************************************************************/

#define DEFAULT_BORDERWIDTH 2

#define offset(field) XtOffset(UKCValueWidget, value.field)
static XtResource resources[] = {
/* change core resources defaults */
	{ XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
	  XtOffset(UKCValueWidget, core.border_width),
	  XtRImmediate, (XtPointer)0 },
/* value resources */
	{ XtNvalueCallback, XtCValueCallback, XtRCallback, sizeof(XtPointer),
	  offset(callbacks), XtRCallback, (XtPointer)NULL },
	{ XtNarrowsPosition, XtCArrowsPosition, XtRArrowsPosition,
	  sizeof(int), offset(orientation), XtRImmediate, (XtPointer)EAST },
	{ XtNnoBorder, XtCNoBorder, XtRBoolean, sizeof(Boolean),
	  offset(no_border), XtRImmediate, (XtPointer)FALSE },
	{ XtNincArrow, XtCReadOnly, XtRWidget, sizeof(Widget),
	  offset(inc_arrow), XtRString, (XtPointer)NULL },
	{ XtNdecArrow, XtCReadOnly, XtRWidget, sizeof(Widget),
	  offset(dec_arrow), XtRString, (XtPointer)NULL },
	{ XtNvalueText, XtCReadOnly, XtRWidget, sizeof(Widget),
	  offset(value_text), XtRString, (XtPointer)NULL },
	{ XtNvalue, XtCValue, XtRInt, sizeof(int),
	  offset(value), XtRImmediate, (XtPointer)0 },
	{ XtNmin, XtCMin, XtRInt, sizeof(int),
	  offset(min_value), XtRImmediate, (XtPointer)0 },
	{ XtNmax, XtCMax, XtRInt, sizeof(int),
	  offset(max_value), XtRImmediate, (XtPointer)100 },
};
#undef offset

static XawTextSelectType selectionArray[] = { XawselectNull };

static String textTrans =
	"<ButtonPress>: end-of-line() \n\
	 <Key>Return: updateValue(TRUE) \n\
	 <LeaveWindow>: display-caret(off) updateValue(TRUE) \n\
	 <EnterWindow>: display-caret(on) \n\
	 <Key>0: insert-char() \n\
	 <Key>1: insert-char() \n\
	 <Key>2: insert-char() \n\
	 <Key>3: insert-char() \n\
	 <Key>4: insert-char() \n\
	 <Key>5: insert-char() \n\
	 <Key>6: insert-char() \n\
	 <Key>7: insert-char() \n\
	 <Key>8: insert-char() \n\
	 <Key>9: insert-char() \n\
	 <Key>-: insert-char() \n\
	 <Key>Delete: delete-previous-character() \n\
	 Ctrl<Key>f: forward-character() \n\
	 Ctrl<Key>b: backward-character()";

static XtActionsRec actionsList[] = {
	{ "updateValue",	updateValue },
};


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

UKCValueClassRec ukcValueClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &ukcLayoutClassRec,
    /* class_name         */    "UKCValue",
    /* widget_size        */    sizeof(UKCValueRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    Realize,
    /* actions            */    actionsList,
    /* num_actions	  */	XtNumber(actionsList),
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
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
    /* constraint_size    */   sizeof(UKCLayoutConstraintRecord),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL,
  }, {
/* UKCLayout class fields */
    /* empty		  */	0,
  }, {
/* UKCValue class fields */
    /* empty		  */	0,
  }
};

WidgetClass ukcValueWidgetClass = (WidgetClass)&ukcValueClassRec;

static EnumListRec valueList[] = {
	{ XtENorth, NORTH },
	{ XtESouth, SOUTH },
	{ XtEEast, EAST },
	{ XtEWest, WEST },
};

static XtConvertArgRec convertArgs[] = {
	{ XtAddress,	(XtPointer)valueList,	sizeof(EnumList) },
	{ XtImmediate,	(XtPointer)XtNumber(valueList),	sizeof(Cardinal) },
	{ XtAddress,	XtRArrowsPosition,	sizeof(String) },
};


/****************************************************************
 *
 * Private Routines
 *
 ****************************************************************/

static void
set_a_value(vw, inform)
UKCValueWidget vw;
Boolean inform;
{
	char tmp[STRLEN];

	if (vw->value.value > vw->value.max_value)
		vw->value.value = vw->value.max_value;
	else {
		if (vw->value.value < vw->value.min_value)
			vw->value.value = vw->value.min_value;
	}

	/* inform any interested parties *before* the change has really
	 * been done so as to allow the interested party to check the
	 * old value using XtGetValues(). */
	if (inform)
		XtCallCallbacks((Widget)vw, XtNvalueCallback,
					(XtPointer)vw->value.value);
	(void)sprintf(tmp, "%d", vw->value.value);
	XukcNewTextWidgetString(vw->value.value_text, tmp);
}
		

/* ARGSUSED */
static void
updateValue(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	UKCValueWidget vw = (UKCValueWidget)XtParent(w);
	String str_val;
	Arg args[1];

	if (!XtIsSensitive(w))
		return;

	XtSetArg(args[0], XtNstring, &str_val);
	XtGetValues(w, (ArgList)args, 1);

	vw->value.value = atoi(str_val);
	set_a_value(vw, *num_params != 0);
}


/* ARGSUSED */
static void
changeValue(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	UKCValueWidget vw = (UKCValueWidget)client_data;

	if (w == vw->value.inc_arrow)
		vw->value.value++;
	else
		vw->value.value--;

	set_a_value(vw, TRUE);
}


set_connections(vw)
UKCValueWidget vw;
{
	Widget w = (Widget)vw;
	Widget iw = vw->value.inc_arrow;
	Widget dw = vw->value.dec_arrow;
	Widget tw = vw->value.value_text;
#define AddC XukcLayoutAddConnection

	switch (vw->value.orientation) {
		case NORTH:
			AddC(w, dw, vw, horizontal_before);
			AddC(w, tw, vw, horizontal_before);
			AddC(w, iw, tw, horizontal_before);
			AddC(w, dw, vw, vertical_above);
			AddC(w, iw, vw, vertical_above);
			AddC(w, tw, iw, vertical_above);
			break;
		case SOUTH:
			AddC(w, dw, vw, horizontal_before);
			AddC(w, tw, vw, horizontal_before);
			AddC(w, iw, tw, horizontal_before);
			AddC(w, dw, tw, vertical_above);
			AddC(w, iw, tw, vertical_above);
			AddC(w, tw, vw, vertical_above);
			break;
		case EAST:
			AddC(w, tw, vw, horizontal_before);
			AddC(w, iw, tw, horizontal_before);
			AddC(w, dw, tw, horizontal_before);
			AddC(w, tw, vw, vertical_above);
			AddC(w, iw, vw, vertical_above);
			AddC(w, dw, tw, vertical_above);
			break;
		case WEST:
			AddC(w, tw, iw, horizontal_before);
			AddC(w, tw, dw, horizontal_before);
			AddC(w, iw, vw, horizontal_before);
			AddC(w, dw, vw, horizontal_before);
			AddC(w, tw, vw, vertical_above);
			AddC(w, iw, vw, vertical_above);
			AddC(w, dw, tw, vertical_above);
			break;
	}
}


remove_connections(vw, orientation)
UKCValueWidget vw;
Cardinal orientation;
{
	Widget w = (Widget)vw;
	Widget iw = vw->value.inc_arrow;
	Widget dw = vw->value.dec_arrow;
	Widget tw = vw->value.value_text;
#define RemoveC XukcLayoutRemoveConnection

	switch (orientation) {
		case NORTH:
			RemoveC(w, dw, vw, horizontal_before);
			RemoveC(w, tw, vw, horizontal_before);
			RemoveC(w, iw, tw, horizontal_before);
			RemoveC(w, dw, vw, vertical_above);
			RemoveC(w, iw, vw, vertical_above);
			RemoveC(w, tw, iw, vertical_above);
			break;
		case SOUTH:
			RemoveC(w, dw, vw, horizontal_before);
			RemoveC(w, tw, vw, horizontal_before);
			RemoveC(w, iw, tw, horizontal_before);
			RemoveC(w, dw, tw, vertical_above);
			RemoveC(w, iw, tw, vertical_above);
			RemoveC(w, tw, vw, vertical_above);
			break;
		case EAST:
			RemoveC(w, tw, vw, horizontal_before);
			RemoveC(w, iw, tw, horizontal_before);
			RemoveC(w, dw, tw, horizontal_before);
			RemoveC(w, tw, vw, vertical_above);
			RemoveC(w, iw, vw, vertical_above);
			RemoveC(w, dw, tw, vertical_above);
			break;
		case WEST:
			RemoveC(w, tw, iw, horizontal_before);
			RemoveC(w, tw, dw, horizontal_before);
			RemoveC(w, iw, vw, horizontal_before);
			RemoveC(w, dw, vw, horizontal_before);
			RemoveC(w, tw, vw, vertical_above);
			RemoveC(w, iw, vw, vertical_above);
			RemoveC(w, dw, tw, vertical_above);
			break;
	}
}


set_arrow_positions(vw)
UKCValueWidget vw;
{
#define MAX(a,b) ((a) > (b) ? (a) : (b))
	Dimension tw, th, tbw, iaw, iah, dah;

	XtVaGetValues(vw->value.value_text, XtNwidth, &tw, XtNheight, &th,
			XtNborderWidth, &tbw, NULL);
	tbw += tbw; tw += tbw; th += tbw;

	XtVaGetValues(vw->value.inc_arrow, XtNwidth, &iaw,
			XtNheight, &iah, NULL);

	XtVaGetValues(vw->value.dec_arrow, 
			XtNheight, &dah, NULL);

	/* layout out the increment arrow, decrement arrow and value text
	 * components within the layout dependent upon the orientation */
	if (vw->value.orientation == NORTH || vw->value.orientation == SOUTH) {
		XtVaSetValues(vw->value.inc_arrow,
				XtNx, (tw - iaw),
				XtNy, (vw->value.orientation == NORTH ? 0 : th),
				XtNorientation, EAST,
				NULL);
		XtVaSetValues(vw->value.dec_arrow,
				XtNx, 0,
				XtNy, (vw->value.orientation == NORTH ? 0 : th),
				XtNorientation, WEST,
				NULL);
		XtVaSetValues(vw->value.value_text,
				XtNx, 0,
				XtNy, (vw->value.orientation == NORTH ?
					MAX(iah, dah) : 0),
				NULL);
	} else { /* EAST or WEST arrow positioning */
		XtVaSetValues(vw->value.dec_arrow,
				XtNx, (vw->value.orientation == EAST ? tw : 0),
				XtNy, (th - dah),
				XtNorientation, SOUTH,
				NULL);
		XtVaSetValues(vw->value.inc_arrow,
				XtNx, (vw->value.orientation == EAST ? tw : 0),
				XtNy, 0,
				XtNorientation, NORTH,
				NULL);
		XtVaSetValues(vw->value.value_text,
				XtNx, 0,
				XtNy, 0,
				NULL);
	}
}


/* > ClassInitialize < */
static void
ClassInitialize()
{
	XtSetTypeConverter(XtRString, XtRArrowsPosition,
			XukcCvtStringToNamedValue,
			convertArgs, XtNumber(convertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
	XtSetTypeConverter(XtRArrowsPosition, XtRString,
			XukcCvtNamedValueToString,
			convertArgs, XtNumber(convertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
}


/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	static XtTranslations textTT = NULL;
	Dimension min_width, min_height;
	XFontStruct *font;
	UKCValueWidget vw = (UKCValueWidget)new;
	char tmp[100];
	char tmp2[100];
	char *tmp3;

	if (textTT == NULL)
		textTT = XtParseTranslationTable(textTrans);

	vw->value.value_text = XtVaCreateManagedWidget("valueText",
			asciiTextWidgetClass, new,
			XtNeditType, XawtextEdit,
			XtNtranslations, textTT,
			XtNlength, STRLEN,
			XtNdisplayCaret, FALSE,
			XtNselectTypes, selectionArray,
			XtNborderWidth, (vw->value.no_border
						? 0 : DEFAULT_BORDERWIDTH),
			NULL);

	vw->value.inc_arrow = XtVaCreateManagedWidget("incValueArrow",
				ukcArrowWidgetClass, new,
				XtNborderWidth, 0,
				XtNvertFixed, TRUE,
				XtNhorizFixed, TRUE,
				NULL);
	vw->value.dec_arrow = XtVaCreateManagedWidget("decValueArrow",
				ukcArrowWidgetClass, new, 
				XtNborderWidth, 0,
				XtNvertFixed, TRUE,
				XtNhorizFixed, TRUE,
				NULL);

	/* calculate the default size of the text widget */
	font = NULL;
	XtVaGetValues(vw->value.value_text, XtNfont, &font, NULL);

	(void)sprintf(tmp, "%d", vw->value.max_value);
	(void)sprintf(tmp2, "%d", vw->value.min_value);
	tmp3 = (strlen(tmp2) > strlen(tmp)) ? tmp2 : tmp;

	XtResizeWidget(vw->value.value_text,
			font != NULL ? XTextWidth(font, tmp3, strlen(tmp3)) + 5
				     : 40,
			request->core.height == 0 ?
			vw->value.value_text->core.height :
			(request->core.height -
			(vw->value.value_text->core.border_width * 2)),
			vw->value.value_text->core.border_width);

	/* calculate the minimum default sizes */
	min_width = vw->value.value_text->core.width +
				 vw->value.value_text->core.border_width * 2;
	if (vw->value.orientation == EAST || vw->value.orientation == WEST)
		min_width += vw->value.inc_arrow->core.width;
			
	min_height = vw->value.value_text->core.height +
				  vw->value.value_text->core.border_width * 2;
	if (vw->value.orientation == NORTH || vw->value.orientation == SOUTH)
		min_height += vw->value.inc_arrow->core.height;

	/* assign default sizes */
	if (min_width > vw->layout.minimum_width)
		vw->layout.minimum_width = min_width;
	if (min_height > vw->layout.minimum_height)
		vw->layout.minimum_height = min_height;

	if (request->core.width == 0)
		vw->core.width = min_width;
	if (request->core.height == 0)
		vw->core.height = min_height;

	XtAddCallback(vw->value.inc_arrow, XtNcallback,
						changeValue, (XtPointer)vw);
	XtAddCallback(vw->value.dec_arrow, XtNcallback,
						changeValue, (XtPointer)vw);
	set_a_value(vw, FALSE);
	set_arrow_positions(vw);
	set_connections(vw);
}


static void
Realize(w, valueMask, attributes)
Widget w;
XtValueMask *valueMask;
XSetWindowAttributes *attributes;
{
	UKCValueWidget vw = (UKCValueWidget)vw;

	XtCreateWindow(w, InputOutput, (Visual *)CopyFromParent,
			(XtValueMask)*valueMask, attributes);

#if 0 /* ||| R4 bug ||| */
	XtSetKeyboardFocus(vw, vw->value.value_text);
#endif /* ||| */
}


/* ARGSUSED */
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
	UKCValueWidget cvw = (UKCValueWidget)current;
	UKCValueWidget nvw = (UKCValueWidget)new;

	if (cvw->value.inc_arrow != nvw->value.inc_arrow ||
	    cvw->value.dec_arrow != nvw->value.dec_arrow ||
	    cvw->value.value_text != nvw->value.value_text) {
		XtWarningMsg("ukcValue", "readOnlyResource",
			"XukcWidgetError",
			"Tried to modify read-only Widget Id. resource",
			(String *)NULL, (Cardinal *)NULL);
		return False;
	}

	if (cvw->core.sensitive != nvw->core.sensitive) {
		XtVaSetValues(nvw->value.inc_arrow, 
				XtNsensitive, nvw->core.sensitive,
				NULL);
		XtVaSetValues(nvw->value.dec_arrow,
				XtNsensitive, nvw->core.sensitive,
				NULL);
		XtVaSetValues(nvw->value.value_text,
				XtNsensitive, nvw->core.sensitive,
				NULL);
	}

	if (cvw->value.no_border != nvw->value.no_border) {
		XtVaSetValues(cvw->value.value_text,
				XtNborderWidth, (nvw->value.no_border ? 0 : 1),
				NULL);
	}

	if (cvw->value.value != nvw->value.value ||
	    cvw->value.min_value != nvw->value.min_value ||
	    cvw->value.max_value != nvw->value.max_value)
		set_a_value(nvw, FALSE);

	if (cvw->value.orientation != nvw->value.orientation) {
		remove_connections(nvw, cvw->value.orientation);
		set_arrow_positions(nvw);
		set_connections(nvw);
	}

	return False;
}
