/* SCCSID: @(#)Converters.h	1.1 (UKC) 5/10/92 */

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


#include <X11/Xmu/Converters.h>
#define XMU 1

#include <X11/Xukc/Convert.h>

static XtConvertArgRec screen_Args[] = {
    {XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.screen),
							sizeof(Screen *)}
};


#ifdef XAW
/* Athena Widgets */

/* ||| the following should not be in Xaw/TextP.h but in StringDefs.h ||| */
/* XtRScrollMode, XtRWrapMode, XtRResizeMode */
#include<X11/Xaw/TextP.h>

#include <X11/Xaw/AsciiText.h>

static EnumListRec xaw_text_scroll[] = {
        { XtEtextScrollNever, (int)XawtextScrollNever },
	{ XtEtextScrollWhenNeeded, (int)XawtextScrollWhenNeeded },
	{ XtEtextScrollAlways, (int)XawtextScrollAlways },
};

static XtConvertArgRec xaw_text_scroll_Args[] = {
        { XtAddress,    (XtPointer)xaw_text_scroll, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_text_scroll), sizeof(Cardinal) },
        { XtAddress,    XtRScrollMode,  sizeof(String) },
};

/******/

static EnumListRec xaw_text_wrap[] = {
	{ XtEtextWrapNever, (int)XawtextWrapNever },
	{ XtEtextWrapLine, (int)XawtextWrapLine },
	{ XtEtextWrapWord, (int)XawtextWrapWord },
};

static XtConvertArgRec xaw_text_wrap_Args[] = {
        { XtAddress,    (XtPointer)xaw_text_wrap, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_text_wrap), sizeof(Cardinal) },
        { XtAddress,    XtRWrapMode,  sizeof(String) },
};

/******/

static EnumListRec xaw_text_resize[] = {
	{ XtEtextResizeNever, (int)XawtextResizeNever },
	{ XtEtextResizeWidth, (int)XawtextResizeWidth },
	{ XtEtextResizeHeight, (int)XawtextResizeHeight },
	{ XtEtextResizeBoth, (int)XawtextResizeBoth },
};

static XtConvertArgRec xaw_text_resize_Args[] = {
        { XtAddress,    (XtPointer)xaw_text_resize, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_text_resize), sizeof(Cardinal) },
        { XtAddress,    XtRResizeMode,  sizeof(String) },
};

/*** Xaw/AsciiText ***/

static EnumListRec xaw_edit_modes[] = {
	{ XtEtextRead, (int)XawtextRead },
	{ XtEtextAppend, (int)XawtextAppend },
	{ XtEtextEdit, (int)XawtextEdit },
};

static XtConvertArgRec xaw_edit_modes_Args[] = {
        { XtAddress,    (XtPointer)xaw_edit_modes, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_edit_modes), sizeof(Cardinal) },
        { XtAddress,    XtREditMode,  sizeof(String) },
};

/*** Xaw/AsciiSrc ***/

static EnumListRec xaw_ascii_type[] = {
	{ XtEstring, (int)XawAsciiString },
	{ XtEfile, (int)XawAsciiFile }
};

static XtConvertArgRec xaw_ascii_type_Args[] = {
        { XtAddress,    (XtPointer)xaw_ascii_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_ascii_type), sizeof(Cardinal) },
        { XtAddress,    XtRAsciiType,  sizeof(String) }
};
#endif /* XAW */

/*** Xt/Shells ***/

static EnumListRec xt_initial_state[] = {
	{ "NormalState", NormalState },
	{ "IconicState", IconicState }
};

static XtConvertArgRec xt_initial_state_Args[] = {
	{ XtAddress,	(XtPointer)xt_initial_state, sizeof(EnumList) },
	{ XtImmediate,	(XtPointer)XtNumber(xt_initial_state), sizeof(Cardinal) },
	{ XtAddress,	XtRInitialState, sizeof(String) }
};

#ifdef XAW
/*** Xaw/Label but defined in Xt/Xmu? ***/
#include <X11/Xaw/Label.h>

static EnumListRec xt_justify[] = {
	{ XtEleft, (int)XtJustifyLeft },
	{ XtEcenter, (int)XtJustifyCenter },
	{ XtEright, (int)XtJustifyRight }
};

static XtConvertArgRec xt_justify_Args[] = {
        { XtAddress,    (XtPointer)xt_justify, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xt_justify), sizeof(Cardinal) },
        { XtAddress,    XtRJustify,  sizeof(String) }
};

/** Xaw/Command but defined in Xmu? ***/
#include <X11/Xaw/Command.h>

static EnumListRec xaw_shape[] = {
	{ "Rectangle", (int)XawShapeRectangle },
	{ "Oval", (int)XawShapeOval },
	{ "Ellipse", (int)XawShapeEllipse },
	{ "RoundedRectangle", (int)XawShapeRoundedRectangle }
};

static XtConvertArgRec xaw_shape_style_Args[] = {
        { XtAddress,    (XtPointer)xaw_shape, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_shape), sizeof(Cardinal) },
        { XtAddress,    XtRShapeStyle,  sizeof(String) },
};

/*** Xaw/Paned/Scroll ***/
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Scrollbar.h>

static EnumListRec xaw_orientation[] = {
	{ XtEhorizontal, (int)XtorientHorizontal },
	{ XtEvertical, (int)XtorientVertical },
};

static XtConvertArgRec xaw_orientation_Args[] = {
        { XtAddress,    (XtPointer)xaw_orientation, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_orientation), sizeof(Cardinal) },
        { XtAddress,    XtROrientation,  sizeof(String) },
};

/*** Xaw/Form ***/
#include <X11/Xaw/Form.h>

static EnumListRec xaw_form_edges[] = {
	{ "chaintop", (int)XtChainTop },
	{ "chainleft", (int)XtChainLeft },
	{ "chainright", (int)XtChainRight },
	{ "chainbottom", (int)XtChainBottom },
	{ "rubber", (int)XtRubber },
};

#ifndef XtREdgeType /* ||| */
#define XtREdgeType "EdgeType"
#endif !XtREdgeType

static XtConvertArgRec xaw_form_edges_Args[] = {
        { XtAddress,    (XtPointer)xaw_form_edges, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_form_edges), sizeof(Cardinal) },
        { XtAddress,    XtREdgeType,  sizeof(String) },
};

#if 0
/*** Xaw/SimpleMenu ***/

static EnumListRec xaw_simple_menu_type[] = {
	{ XtEblank, (int)XawMenuBlank },
	{ XtEnone, (int)XawMenuNone },
	{ XtEseparator, (int)XawMenuSeparator },
	{ XtEtext, (int)XawMenuText },
};

static XtConvertArgRec xaw_simple_menu_type_Args[] = {
        { XtAddress,    (XtPointer)xaw_simple_menu_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xaw_simple_menu_type), sizeof(Cardinal) },
        { XtAddress,    XtRSimpleMenuType,  sizeof(String) },
};
#endif 0
#endif /* XAW */

#ifdef XMU
/*** Xmu/Backing Store ***/

static EnumListRec xmu_backing_store[] = {
	{ XtEnotUseful, (int)NotUseful },
	{ XtEwhenMapped, (int)WhenMapped },
	{ XtEalways, (int)Always },
	{ XtEdefault, (int)(Always + WhenMapped + NotUseful) },
};

static XtConvertArgRec xmu_backing_store_Args[] = {
        { XtAddress,    (XtPointer)xmu_backing_store, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xmu_backing_store), sizeof(Cardinal) },
        { XtAddress,    XtRBackingStore,  sizeof(String) },
};
#endif /* XMU */

#ifdef HP_WIDGETS
/* HP Widget Set */
#include <X11/Xw/Xw.h>
#include <X11/Xw/Arrow.h>
#include <X11/Xw/BBoard.h>
#include <X11/Xw/Cascade.h>
#include <X11/Xw/Form.h>
#include <X11/Xw/ImageEdit.h>
#include <X11/Xw/List.h>
#include <X11/Xw/ListRC.h>
#include <X11/Xw/MenuBtn.h>
#include <X11/Xw/MenuSep.h>
#include <X11/Xw/PButton.h>
#include <X11/Xw/Panel.h>
#include <X11/Xw/PopupMgr.h>
#include <X11/Xw/RCManager.h>
#include <X11/Xw/SRaster.h>
#include <X11/Xw/SText.h>
#include <X11/Xw/SWindow.h>
#include <X11/Xw/Sash.h>
#include <X11/Xw/ScrollBar.h>
#include <X11/Xw/TextEdit.h>
#include <X11/Xw/TitleBar.h>
#include <X11/Xw/Toggle.h>
#include <X11/Xw/VPW.h>
#include <X11/Xw/Valuator.h>
#include <X11/Xw/WorkSpace.h>

/*** Xhp/Alignment ***/

static EnumListRec xhp_alignment[] = {
	{ "left", (int)XwALIGN_LEFT },
	{ "center", (int)XwALIGN_CENTER },
	{ "right", (int)XwALIGN_RIGHT },
	{ "none", (int)XwALIGN_NONE },
};

static XtConvertArgRec xhp_alignment_Args[] = {
        { XtAddress,    (XtPointer)xhp_alignment, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_alignment), sizeof(Cardinal) },
        { XtAddress,    XtRAlignment,  sizeof(String) },
};

/*** Xhp/ArrowDirection ***/

static EnumListRec xhp_arrow_direction[] = {
	{ "arrow_down", (int)XwARROW_DOWN },
	{ "arrow_up", (int)XwARROW_UP },
	{ "arrow_left", (int)XwARROW_LEFT },
	{ "arrow_right", (int)XwARROW_RIGHT },
};

static XtConvertArgRec xhp_arrow_direction_Args[] = {
        { XtAddress,    (XtPointer)xhp_arrow_direction, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_arrow_direction), sizeof(Cardinal) },
        { XtAddress,    XtRArrowDirection,  sizeof(String) },
};

/*** Xhp/DestroyMode ***/

static EnumListRec xhp_destroy_mode[] = {
	{ "no_shrink", (int)XwNO_SHRINK },
	{ "shrink_all", (int)XwSHRINK_ALL },
	{ "shrink_column", (int)XwSHRINK_COLUMN },
};

static XtConvertArgRec xhp_destroy_mode_Args[] = {
        { XtAddress,    (XtPointer)xhp_destroy_mode, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_destroy_mode), sizeof(Cardinal) },
        { XtAddress,    XtRDestroyMode,  sizeof(String) },
};

/*** Xhp/ElementHighlight ***/

static EnumListRec xhp_element_highlight[] = {
	{ "border", (int)XwBORDER },
	{ "invert", (int)XwINVERT },
};

static XtConvertArgRec xhp_element_highlight_Args[] = {
        { XtAddress,    (XtPointer)xhp_element_highlight, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_element_highlight), sizeof(Cardinal) },
        { XtAddress,    XtRElementHighlight,  sizeof(String) },
};

/*** Xhp/Gravity ***/

static EnumListRec xhp_gravity[] = {
	{ "northgravity", (int)NorthGravity },
	{ "northwestgravity", (int)NorthWestGravity },
	{ "northeastgravity", (int)NorthEastGravity },
	{ "southgravity", (int)SouthGravity },
	{ "southwestgravity", (int)SouthWestGravity },
	{ "southeastgravity", (int)SouthEastGravity },
	{ "westgravity", (int)WestGravity },
	{ "eastgravity", (int)EastGravity },
	{ "centergravity", (int)CenterGravity },
};

static XtConvertArgRec xhp_gravity_Args[] = {
        { XtAddress,    (XtPointer)xhp_gravity, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_gravity), sizeof(Cardinal) },
        { XtAddress,    XtRGravity,  sizeof(String) },
};

/*** Xhp/Grow ***/

static EnumListRec xhp_grow[] = {
	{ "growoff", (int)XwGrowOff },
	{ "growhorizontal", (int)XwGrowHorizontal },
	{ "growvertical", (int)XwGrowVertical },
	{ "growboth", (int)XwGrowBoth },
};

static XtConvertArgRec xhp_grow_Args[] = {
        { XtAddress,    (XtPointer)xhp_grow, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_grow), sizeof(Cardinal) },
        { XtAddress,    XtRGrow,  sizeof(String) },
};

/*** Xhp/HighlightStyle ***/

static EnumListRec xhp_highlight_style[] = {
	{ "pattern_border", (int)XwPATTERN_BORDER },
	{ "widget_defined", (int)XwWIDGET_DEFINED },
};

static XtConvertArgRec xhp_highlight_style_Args[] = {
        { XtAddress,    (XtPointer)xhp_highlight_style, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_highlight_style), sizeof(Cardinal) },
        { XtAddress,    XtRHighlightStyle,  sizeof(String) },
};

#if 0
/*** Xhp/Image ***/

static EnumListRec xhp_image[] = {
	{ "pattern_border", (int)XwPATTERN_BORDER },
	{ "widget_defined", (int)XwWIDGET_DEFINED },
};

static XtConvertArgRec xhp_image_Args[] = {
        { XtAddress,    (XtPointer)xhp_image, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_image), sizeof(Cardinal) },
        { XtAddress,    XtRImage,  sizeof(String) },
};
#endif 0

/*** Xhp/LabelLocation ***/

static EnumListRec xhp_label_location[] = {
	{ "left", (int)XwLEFT },
	{ "right", (int)XwRIGHT },
};

static XtConvertArgRec xhp_label_location_Args[] = {
        { XtAddress,    (XtPointer)xhp_label_location, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_label_location), sizeof(Cardinal) },
        { XtAddress,    XtRLabelLocation,  sizeof(String) },
};

/*** Xhp/LabelType ***/

static EnumListRec xhp_label_type[] = {
	{ "string", (int)XwSTRING },
	{ "image", (int)XwIMAGE },
};

static XtConvertArgRec xhp_label_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_label_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_label_type), sizeof(Cardinal) },
        { XtAddress,    XtRLabelType,  sizeof(String) },
};

/*** Xhp/Layout ***/

static EnumListRec xhp_layout[] = {
	{ "minimize", (int)XwMINIMIZE },
	{ "maximize", (int)XwMAXIMIZE },
	{ "ignore", (int)XwIGNORE },
};

static XtConvertArgRec xhp_layout_Args[] = {
        { XtAddress,    (XtPointer)xhp_layout, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_layout), sizeof(Cardinal) },
        { XtAddress,    XtRLayout,  sizeof(String) },
};

/*** Xhp/LayoutType ***/

static EnumListRec xhp_layout_type[] = {
	{ "requested_columns", (int)XwMINIMIZE },
	{ "maximum_columns", (int)XwMAXIMIZE },
	{ "maximum_unaligned", (int)XwIGNORE },
};

static XtConvertArgRec xhp_layout_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_layout_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_layout_type), sizeof(Cardinal) },
        { XtAddress,    XtRLayoutType,  sizeof(String) },
};

/*** Xhp/Mode ***/

#if 0
static EnumListRec xhp_mode[] = {
	{ "requested_columns", (int)XwMINIMIZE },
	{ "maximum_columns", (int)XwMAXIMIZE },
	{ "maximum_unaligned", (int)XwIGNORE },
};

static XtConvertArgRec xhp_mode_Args[] = {
        { XtAddress,    (XtPointer)xhp_mode, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_mode), sizeof(Cardinal) },
        { XtAddress,    XtRMode,  sizeof(String) },
};
#endif 0

/*** Xhp/RCMode ***/

static EnumListRec xhp_rc_mode[] = {
	{ "one_of_many", (int)XwONE_OF_MANY },
	{ "n_of_many", (int)XwN_OF_MANY },
};

static XtConvertArgRec xhp_rc_mode_Args[] = {
        { XtAddress,    (XtPointer)xhp_rc_mode, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_rc_mode), sizeof(Cardinal) },
        { XtAddress,    XtRRCMode,  sizeof(String) },
};

/*** Xhp/Scroll ***/

static EnumListRec xhp_scroll[] = {
	{ "autoscrolloff", (int)XwAutoScrollOff },
	{ "autoscrollvertical", (int)XwAutoScrollVertical },
	{ "autoscrollhorizontal", (int)XwAutoScrollHorizontal },
	{ "autoscrollboth", (int)XwAutoScrollBoth },
};

static XtConvertArgRec xhp_scroll_Args[] = {
        { XtAddress,    (XtPointer)xhp_scroll, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_scroll), sizeof(Cardinal) },
        { XtAddress,    XtRScroll,  sizeof(String) },
};

/*** Xhp/SelectionBias ***/

static EnumListRec xhp_selection_bias[] = {
	{ "col_bias", (int)XwCOL_BIAS },
	{ "row_bias", (int)XwROW_BIAS },
	{ "no_bias", (int)XwNO_BIAS },
};

static XtConvertArgRec xhp_selection_bias_Args[] = {
        { XtAddress,    (XtPointer)xhp_selection_bias, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_selection_bias), sizeof(Cardinal) },
        { XtAddress,    XtRSelectionBias,  sizeof(String) },
};

/*** Xhp/SelectionMethod ***/

static EnumListRec xhp_selection_method[] = {
	{ "single", (int)XwSINGLE },
	{ "multiple", (int)XwMULTIPLE },
};

static XtConvertArgRec xhp_selection_method_Args[] = {
        { XtAddress,    (XtPointer)xhp_selection_method, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_selection_method), sizeof(Cardinal) },
        { XtAddress,    XtRSelectionMethod,  sizeof(String) },
};

/*** Xhp/SelectionStyle ***/

static EnumListRec xhp_selection_style[] = {
	{ "instant", (int)XwINSTANT },
	{ "sticky", (int)XwSTICKY },
};

static XtConvertArgRec xhp_selection_style_Args[] = {
        { XtAddress,    (XtPointer)xhp_selection_style, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_selection_style), sizeof(Cardinal) },
        { XtAddress,    XtRSelectionStyle,  sizeof(String) },
};

/*** Xhp/SeparatorType ***/

static EnumListRec xhp_separator_type[] = {
	{ "double_line", (int)XwDOUBLE_LINE },
	{ "double_dashed_line", (int)XwDOUBLE_DASHED_LINE },
	{ "single_dashed_line", (int)XwSINGLE_DASHED_LINE },
	{ "no_line", XwNO_LINE },
};

static XtConvertArgRec xhp_separator_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_separator_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_separator_type), sizeof(Cardinal) },
        { XtAddress,    XtRSeparatorType,  sizeof(String) },
};

/*** Xhp/SlideOrientation ***/

static EnumListRec xhp_slide_orientation[] = {
	{ "horizontal", (int)XwHORIZONTAL },
	{ "vertical", (int)XwVERTICAL },
};

static XtConvertArgRec xhp_slide_orientation_Args[] = {
        { XtAddress,    (XtPointer)xhp_slide_orientation, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_slide_orientation), sizeof(Cardinal) },
        { XtAddress,    XtRSlideOrientation,  sizeof(String) },
};

/*** Xhp/SourceType ***/

static EnumListRec xhp_source_type[] = {
	{ "stringsrc", (int)XwstringSrc },
	{ "disksrc", (int)XwdiskSrc },
	{ "userdefined", (int)XwprogDefinedSrc },
};

static XtConvertArgRec xhp_source_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_source_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_source_type), sizeof(Cardinal) },
        { XtAddress,    XtRSourceType,  sizeof(String) },
};

/*** Xhp/TileType ***/

static EnumListRec xhp_tile_type[] = {
	{ "foreground", (int)XwBOTTOM },
	{ "background", (int)XwTOP },
	{ "25_foreground", (int)Xw25_FOREGROUND },
	{ "50_foreground", (int)Xw50_FOREGROUND },
	{ "75_foreground", (int)Xw75_FOREGROUND },
	{ "vertical_tile", (int)XwVERTICAL_TILE },
	{ "horizontal_tile", (int)XwHORIZONTAL_TILE },
	{ "slant_left", (int)XwSLANT_LEFT },
	{ "slant_right", (int)XwSLANT_RIGHT },
};

static XtConvertArgRec xhp_tile_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_tile_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_tile_type), sizeof(Cardinal) },
        { XtAddress,    XtRTileType,  sizeof(String) },
};

/*** Xhp/TitlePositionType ***/

static EnumListRec xhp_title_position_type[] = {
	{ "bottom", (int)XwBOTTOM },
	{ "top", (int)XwTOP },
	{ "both", (int)(XwBOTTOM | XwTOP) },
};

static XtConvertArgRec xhp_title_position_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_title_position_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_title_position_type), sizeof(Cardinal) },
        { XtAddress,    XtRTitlePositionType,  sizeof(String) },
};

/*** Xhp/TitleType ***/

static EnumListRec xhp_title_type[] = {
	{ "string", (int)XwSTRING },
	{ "image", (int)XwIMAGE },
};

static XtConvertArgRec xhp_title_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_title_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_title_type), sizeof(Cardinal) },
        { XtAddress,    XtRTitleType,  sizeof(String) },
};

/*** Xhp/TraversalType ***/

static EnumListRec xhp_traversal_type[] = {
	{ "highlight_off", (int)XwHIGHLIGHT_OFF },
	{ "highlight_traversal", (int)XwHIGHLIGHT_TRAVERSAL },
	{ "highlight_enter", (int)XwHIGHLIGHT_ENTER },
};

static XtConvertArgRec xhp_traversal_type_Args[] = {
        { XtAddress,    (XtPointer)xhp_traversal_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_traversal_type), sizeof(Cardinal) },
        { XtAddress,    XtRTraversalType,  sizeof(String) },
};

/*** Xhp/Wrap ***/

static EnumListRec xhp_wrap[] = {
	{ "wrapoff", (int)XwWrapOff },
	{ "softwrap", (int)XwSoftWrap },
	{ "hardwrap", (int)XwHardWrap },
};

static XtConvertArgRec xhp_wrap_Args[] = {
        { XtAddress,    (XtPointer)xhp_wrap, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_wrap), sizeof(Cardinal) },
        { XtAddress,    XtRWrap,  sizeof(String) },
};

/*** Xhp/WrapBreak ***/

static EnumListRec xhp_wrap_break[] = {
	{ "wrapany", (int)XwWrapAny },
	{ "wrapwhitespace", (int)XwWrapWhiteSpace },
};

static XtConvertArgRec xhp_wrap_break_Args[] = {
        { XtAddress,    (XtPointer)xhp_wrap_break, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_wrap_break), sizeof(Cardinal) },
        { XtAddress,    XtRWrapBreak,  sizeof(String) },
};

/*** Xhp/WrapForm ***/

static EnumListRec xhp_wrap_form[] = {
	{ "sourceform", (int)XwSourceForm },
	{ "displayform", (int)XwDisplayForm },
};

static XtConvertArgRec xhp_wrap_form_Args[] = {
        { XtAddress,    (XtPointer)xhp_wrap_form, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xhp_wrap_form), sizeof(Cardinal) },
        { XtAddress,    XtRWrapForm,  sizeof(String) },
};
#endif /* HP_WIDGETS */

#ifdef UKC_WIDGETS

/*** Xukc/Arrow *** this duplicates code that isn't public ***/
#include <X11/Xukc/Arrow.h>

static EnumListRec xukc_arrow_orientation[] = {
	{ XtENorth, NORTH },
	{ XtESouth, SOUTH },
	{ XtEEast, EAST },
	{ XtEWest, WEST },
};

/*** Xukc/Label *** this duplicates code that isn't public ***/
#include <X11/Xukc/LabelP.h>

static EnumListRec xukc_vertical_justify[] = {
	{ XukcJustifyTop, (int)XukcEtop },
	{ XukcJustifyCenter, (int)XukcEcenter },
	{ XukcJustifyBottom, (int)XukcEbottom },
};
#endif /* UKC_WIDGETS */

static XtConvertArgRec pixmap_Args[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.screen),
							sizeof(Screen *)},
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.widget_class),
							sizeof(WidgetClass)},
#ifdef UKC_WIDGETS
    {XtWidgetBaseOffset, (XtPointer)XtOffset(UKCLabelWidget, label.foreground),
							sizeof(Pixel)},
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.background_pixel),
							sizeof(Pixel)}
#endif /* UKC_WIDGETS */
};

#ifdef UKC_WIDGETS

/*** Xukc/RowCol *** this duplicates code that isn't public ***/
#include <X11/Xukc/RowCol.h>

static EnumListRec xukc_layout[] = {
	{ XukcELayoutMaximize, XukcLayoutMaximize },
	{ XukcELayoutMinimize, XukcLayoutMinimize },
	{ XukcELayoutIgnore, XukcLayoutIgnore },
};

static EnumListRec xukc_layout_type[] = {
	{ XukcERequestedColumns, XukcRequestedColumns },
	{ XukcEMaximumColumns, XukcMaximumColumns },
	{ XukcEMaximumUnaligned, XukcMaximumUnaligned },
};

#endif /* UKC_WIDGETS */

/*** Pixel *** conversion arguments ***/

static XtConvertArgRec pixelConvertArgs[] = {
 {XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.screen),
							sizeof(Screen *)},
 {XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.colormap),
							sizeof(Colormap)}
};

/*** Visual *** conversion arguments ***/

static EnumListRec xt_visual[] = {
	{ "staticgray", StaticGray },
	{ "staticcolor", StaticColor },
	{ "truecolor", TrueColor },
	{ "grayscale", GrayScale },
	{ "pseudocolor", PseudoColor },
	{ "directcolor", DirectColor }
};

static XtConvertArgRec xt_visual_Args[] = {
        { XtAddress,    (XtPointer)xt_visual, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xt_visual), sizeof(Cardinal) },
        { XtAddress,    XtRVisual,  sizeof(String) },
};
