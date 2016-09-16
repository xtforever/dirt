#ifndef lint
static char sccsid[] = "%W% (UKC) %G%";
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
 * ScrollList.c - UKC Scrollable List.  Combines the functions of a viewport
 *		  and a list to produce a scrollable list with thumb-bar marks.
 *		  Press the first mouse button on an entry highlights it and
 *		  a callback routine is called.
 *		  The list is scrolled to the newly highlighted entry.
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/ScrollListP.h>

static void ClassInitialize();
static void Initialize();
static void Destroy();
static void Realize();
static void Resize();
static Boolean SetValues();

/****************************************************************
 *
 * UKCScrList Resources
 *
 ****************************************************************/

#define offset(field) XtOffset(UKCScrListWidget, scrlist.field)
static XtResource resources[] = {
	{ XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(list_callback), XtRCallback, (XtPointer)NULL },
	{ XtNlistWidget, XtCReadOnly, XtRWidget, sizeof(Widget),
	  offset(list_widget), XtRWidget, (XtPointer)NULL },
	{ XtNlist, XtCList, XtRPointer, sizeof(char **),
	  offset(string_list), XtRPointer, (XtPointer)NULL },
	{ XtNhighlighted, XtCHighlighted, XtRInt, sizeof(int),
	  offset(highlighted), XtRImmediate, (XtPointer)0 },
	{ XtNnumberStrings, XtCNumberStrings, XtRCardinal, sizeof(Cardinal),
	  offset(num_strings), XtRImmediate, (XtPointer)0 },
	{ XtNlistName, XtCListName, XtRString, sizeof(String),
	  offset(list_name), XtRPointer, (XtPointer)NULL },
	{ XtNlistAutoScroll, XtCAutoScroll, XtRBoolean, sizeof(Boolean),
	  offset(list_scroll), XtRImmediate, (XtPointer)TRUE },

/* override superclass defaults */
	{ XtNforceBars, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  XtOffset(UKCViewportWidget,viewport.forcebars),
	  XtRImmediate, (XtPointer)True },
	{ XtNallowHoriz, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  XtOffset(UKCViewportWidget,viewport.allowhoriz),
	  XtRImmediate, (XtPointer)True },
	{ XtNallowVert, XtCBoolean, XtRBoolean, sizeof(Boolean),
	  XtOffset(UKCViewportWidget,viewport.allowvert),
	  XtRImmediate, (XtPointer)True},
};
#undef offset

static XtTranslations listTrans = NULL;
static String listTransStr = "<Btn1Down>,<Btn1Up>: Set() Notify()";


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

#define SuperClass ((UKCViewportWidgetClass)&ukcViewportClassRec)

UKCScrListClassRec ukcScrListClassRec = {
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) SuperClass,
    /* class_name         */    "UKCScrList",
    /* widget_size        */    sizeof(UKCScrListRec),
    /* class_initialize   */    ClassInitialize,
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
    /* resize             */    Resize,
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
    /* extension          */	NULL
  },{
/* composite_class fields */
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension          */	NULL
  },{
/* constraint_class fields */
    /* subresourses       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(UViewportConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
  }, {
    /* form_class fields */
    /* empty		  */	0
  }, {
/* UKCViewport class fields */
    /* empty		  */	0
  }, {
/* UKCScrList class fields */
    /* empty		  */	0
  }
};

WidgetClass ukcScrListWidgetClass = (WidgetClass)&ukcScrListClassRec;

/****************************************************************
 *
 * Private Routines
 *
 ****************************************************************/

static void
set_marks(slw, highlighted, call, call_data)
UKCScrListWidget slw;
Cardinal highlighted;
Boolean call;
XtPointer call_data;
{
	Dimension sh, lh;
	Position scroll_to;
	double mark_pos, mark_size;
	XtWidgetGeometry intend, pref;

	XtVaGetValues(slw->viewport.vert_bar, XtNheight, &sh, NULL);
	XtVaGetValues(slw->scrlist.list_widget, XtNwidth, &lh, NULL);

	intend.request_mode = CWWidth;
	intend.width = lh;
	(void)XtQueryGeometry(slw->scrlist.list_widget, &intend, &pref);
	lh = pref.height;

	mark_size = 1.0 / (double)slw->scrlist.num_strings;
	mark_pos = (double)highlighted /
				(double)slw->scrlist.num_strings;
	if (lh <= sh) {
		mark_pos = mark_pos * ((double)lh / (double)sh);
		mark_size = mark_size * ((double)lh / (double)sh);
	}

	if (slw->scrlist.mark != NULL)
		XukcScrollbarRemoveMark(slw->viewport.vert_bar,
					slw->scrlist.mark);
	slw->scrlist.mark = XukcScrollbarUpdateMark(slw->viewport.vert_bar,
				NULL, mark_pos,
				mark_pos + mark_size);
	XukcScrollbarDisplayMarks(slw->viewport.vert_bar, TRUE);

	if (slw->scrlist.list_scroll) {
		/* scroll the list to this new entry string */
		scroll_to = (mark_pos * lh) - (sh / 2)
				+ ((double)mark_size * sh) / 2;
		XtVaSetValues(slw->scrlist.list_widget, XtNx, 0,
						XtNy, -scroll_to, NULL);
	}
	if (call)
		XtCallCallbacks((Widget)slw, XtNcallback, call_data);
}


static void
list_touched(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	XawListReturnStruct *picked_string = (XawListReturnStruct *)call_data;
	UKCScrListWidget slw = (UKCScrListWidget)client_data;

	set_marks(slw, picked_string->list_index, TRUE, call_data);
}


static void
create_list(slw)
UKCScrListWidget slw;
{
	if (slw->scrlist.string_list != NULL) {
		int length = slw->scrlist.num_strings;

		slw->scrlist.list_widget = XtVaCreateManagedWidget(
			slw->scrlist.list_name == NULL ? "scrList" :
						slw->scrlist.list_name,
			listWidgetClass, (Widget)slw,
			XtNlist, slw->scrlist.string_list,
			XtNnumberStrings, length,
			XtNdefaultColumns, 1,
			XtNforceColumns, TRUE,
			XtNtranslations, listTrans,
			NULL);
		XtVaGetValues(slw->scrlist.list_widget,
				XtNnumberStrings, &length,
				NULL);
		slw->scrlist.num_strings = (Cardinal)length;
		XtAddCallback(slw->scrlist.list_widget, XtNcallback,
				list_touched, (XtPointer)slw);
	}
}


/* > ClassInitialize < */
static void
ClassInitialize()
{
	listTrans = XtParseTranslationTable(listTransStr);
}


/* ARGSUSED */
static void
Resize(w)
Widget w;
{
	XawListReturnStruct *current_string;
	UKCScrListWidget slw = (UKCScrListWidget)w;

	(*SuperClass->core_class.resize)(w);

	if (slw->scrlist.list_widget != NULL) {
		current_string = XawListShowCurrent(slw->scrlist.list_widget);
		if (current_string->list_index >= 0)
			set_marks(slw, current_string->list_index,
					FALSE, (XtPointer)current_string);
	}
}


/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCScrListWidget slw = (UKCScrListWidget)new;

	XtVaSetValues(slw->viewport.horiz_bar, XtNmarkSize, 0, NULL);

	if (slw->scrlist.list_name != NULL)
		slw->scrlist.list_name = XtNewString(slw->scrlist.list_name);

	slw->scrlist.mark = NULL;
	create_list(slw);
}


static void
Destroy(w)
Widget w;
{
	UKCScrListWidget slw = (UKCScrListWidget)w;

	if (slw->scrlist.list_name != NULL)
		XtFree(slw->scrlist.list_name);
}


/* ARGSUSED */
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
	Boolean redisplay = FALSE;
	UKCScrListWidget clw = (UKCScrListWidget)current;
	UKCScrListWidget nlw = (UKCScrListWidget)new;

	if (clw->scrlist.list_widget != nlw->scrlist.list_widget) {
		/* XtAppWarningMsg(); ||| finish this ||| */
		nlw->scrlist.list_widget = clw->scrlist.list_widget;
	}
	if (clw->scrlist.string_list != nlw->scrlist.string_list ||
	    clw->scrlist.num_strings != nlw->scrlist.num_strings) {
		if (clw->scrlist.list_widget == NULL)
			/* a new list */
			create_list(nlw);
		else { /* update an existing list */
			int length = (int)nlw->scrlist.num_strings;

			XawListChange(clw->scrlist.list_widget,
					nlw->scrlist.string_list,
					length, 0, TRUE);
			XtVaGetValues(clw->scrlist.list_widget,
					XtNnumberStrings, &length, NULL);
			nlw->scrlist.num_strings = (Cardinal)length;
			XukcScrollbarRemoveMark(nlw->viewport.vert_bar,
					nlw->scrlist.mark);
			XukcScrollbarDisplayMarks(nlw->viewport.vert_bar, TRUE);
			nlw->scrlist.mark = NULL;
		}
		redisplay = TRUE;
	}
	if (clw->scrlist.list_name != nlw->scrlist.list_name) {
		XtFree(clw->scrlist.list_name);
		if (nlw->scrlist.list_name != NULL)
			nlw->scrlist.list_name =
					XtNewString(nlw->scrlist.list_name);
	}
	if ((clw->scrlist.highlighted != nlw->scrlist.highlighted
	    || (clw->core.sensitive != nlw->core.sensitive &&
		nlw->core.sensitive))
	    && nlw->scrlist.list_widget != NULL) {
		if (nlw->scrlist.highlighted <= 0
		    || (nlw->scrlist.num_strings > 0
		    && nlw->scrlist.highlighted > nlw->scrlist.num_strings)) {
		       XawListUnhighlight(nlw->scrlist.list_widget);
			nlw->scrlist.highlighted = 0;
		} else
			(void)XukcScrListSelectItem(new,
						    nlw->scrlist.highlighted,
						    FALSE);
	}
	return (redisplay);
}


/**** Public Routines ****/

Boolean
XukcScrListSelectItem(w, item_num, call, auto_scroll)
Widget w;
Cardinal item_num;
Boolean call, auto_scroll;
{
	XawListReturnStruct *picked_string;
	UKCScrListWidget slw = (UKCScrListWidget)w;
	Boolean old_scroll;

	if (slw->scrlist.list_widget == NULL)
		return FALSE;

	if (item_num < 1 || item_num > slw->scrlist.num_strings)
		return FALSE; /* out of range */

	old_scroll = slw->scrlist.list_scroll;
	slw->scrlist.list_scroll = auto_scroll;
	XawListHighlight(slw->scrlist.list_widget, item_num - 1);
	picked_string = XawListShowCurrent(slw->scrlist.list_widget);
	set_marks(slw, item_num - 1, call, (XtPointer)picked_string);
	slw->scrlist.list_scroll = old_scroll;
	return TRUE;
}


void
XukcScrListUnselectItems(w)
Widget w;
{
	UKCScrListWidget slw = (UKCScrListWidget)w;

	if (slw->scrlist.list_widget == NULL)
		return;

	XawListUnhighlight(slw->scrlist.list_widget);
	XukcScrollbarRemoveMark(slw->viewport.vert_bar, slw->scrlist.mark);
	XukcScrollbarDisplayMarks(slw->viewport.vert_bar, TRUE);
	slw->scrlist.mark = NULL;
	slw->scrlist.highlighted = 0;
}


void
XukcScrListChange(w, new_list, new_list_length)
Widget w;
String *new_list;
Cardinal new_list_length;
{
	XukcScrListUnselectItems(w);
	XtVaSetValues(w, XtNlist, new_list,
			 XtNnumberStrings, new_list_length,
			 NULL);
}
