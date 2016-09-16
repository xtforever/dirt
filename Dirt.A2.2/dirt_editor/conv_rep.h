/* SCCSID: @(#)conv_rep.h	1.1 (UKC) 5/10/92 */

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

/* Representation types */

typedef struct _rep_array {
	XrmQuark quark;
	String	type;
	EnumList enum_array;
	Cardinal num_enums;
	Widget (*display_proc)();
} RepArray;

typedef RepArray *RepArrayPtr;


static XtProc cannot_convert();
static XtProc colour_convert();
static XtProc trans_convert();

static Widget display_callbacks();
static Widget display_string();
static Widget display_signed_value();
static Widget display_unsigned_value();
static Widget display_boolean();
static Widget display_enumerated();
static Widget display_translations();
static Widget display_list();
static Widget display_widget();
static Widget display_widget_list();
static Widget display_font();
static Widget display_colour();
static Widget display_read_only();

/* !!! this has been defined in FormP.h instead of Form.h !!! */
#ifndef XtREdgeType
#	define XtREdgeType	"EdgeType"
#endif !XtREdgeType

#ifdef UKC_WIDGETS
#	include <X11/Xukc/Value.h>
#	include <X11/Xukc/Layout.h>
#	include <X11/Xukc/RowCol.h>
#endif /* UKC_WIDGETS */

#ifdef MOTIF_WIDGETS
#	include <Xm/Xm.h>
#endif /* MOTIF_WIDGETS */

static RepArray rep_defaults[] = {
/* intrinsic supported types */
	{ (XrmQuark)0, XtRCallback, (EnumList)NULL, 0, display_callbacks },
	{ (XrmQuark)0, XtRCardinal, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XtRDimension, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XtRPosition, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XtRAtom, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRBool, (EnumList)NULL, 0, display_boolean },
	{ (XrmQuark)0, XtRBoolean, (EnumList)NULL, 0, display_boolean },
	{ (XrmQuark)0, XtRInt, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XtRLongBoolean, (EnumList)NULL, 0, display_boolean },
	{ (XrmQuark)0, XtRPixel, (EnumList)NULL, 0, display_colour },
	{ (XrmQuark)0, XtRShort, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XtRGeometry, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRAcceleratorTable, (EnumList)NULL, 0, display_translations },
	{ (XrmQuark)0, XtRBitmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRColor, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRFile, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRFont, (EnumList)NULL, 0, display_font },
	{ (XrmQuark)0, XtRFontStruct, (EnumList)NULL, 0, display_font },
	{ (XrmQuark)0, XtRInitialState, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRStringTable, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRUnsignedChar, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XtRTranslationTable, (EnumList)NULL, 0, display_translations },
	{ (XrmQuark)0, XtRVisual, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRWidget, (EnumList)NULL, 0, display_widget },
	{ (XrmQuark)0, XtRWidgetList, (EnumList)NULL, 0, display_widget_list },

#ifdef XMU
	{ (XrmQuark)0, XtRCursor, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRBackingStore, xmu_backing_store,
			XtNumber(xmu_backing_store), display_enumerated },
	{ (XrmQuark)0, XtRString, (EnumList)NULL, 0, display_string },
#endif /* XMU */

#ifdef XAW
	{ (XrmQuark)0, XtROrientation, xaw_orientation,
			XtNumber(xaw_orientation), display_enumerated },
	{ (XrmQuark)0, XtREdgeType, xaw_form_edges,
			XtNumber(xaw_form_edges), display_enumerated },
/* Label/Xmu */
	{ (XrmQuark)0, XtRJustify, xt_justify,
			XtNumber(xt_justify), display_enumerated },
/* Command/Xmu */
	{ (XrmQuark)0, XtRShapeStyle, xaw_shape,
			XtNumber(xaw_shape), display_enumerated },
/* Scroll */
	{ (XrmQuark)0, XtRFloat, (EnumList)NULL, 0, display_string },
/* Text */
	{ (XrmQuark)0, XtREditMode, xaw_edit_modes,
			XtNumber(xaw_edit_modes), display_enumerated },
	{ (XrmQuark)0, XtRScrollMode, xaw_text_scroll,
			XtNumber(xaw_text_scroll), display_enumerated },
	{ (XrmQuark)0, XtRWrapMode, xaw_text_wrap,
			XtNumber(xaw_text_wrap), display_enumerated },
	{ (XrmQuark)0, XtRResizeMode, xaw_text_resize,
			XtNumber(xaw_text_resize), display_enumerated },
	{ (XrmQuark)0, XtRAsciiType, xaw_ascii_type,
			XtNumber(xaw_ascii_type), display_enumerated },
#endif /* XAW */

/* UKC Widget set resource types */
#ifdef UKC_WIDGETS
	{ (XrmQuark)0, XtRArrowOrientation, xukc_arrow_orientation,
			XtNumber(xukc_arrow_orientation), display_enumerated },
	{ (XrmQuark)0, XtRArrowsPosition, xukc_arrow_orientation,
			XtNumber(xukc_arrow_orientation), display_enumerated },
	{ (XrmQuark)0, XtRGeomStatus, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XtRVerticalJustify, xukc_vertical_justify,
			XtNumber(xukc_vertical_justify), display_enumerated },
	{ (XrmQuark)0, XtRLayout, xukc_layout, XtNumber(xukc_layout),
			display_enumerated },
	{ (XrmQuark)0, XtRLayoutType, xukc_layout_type,
			XtNumber(xukc_layout_type), display_enumerated },
#endif /* UKC_WIDGETS */

/* Motif Widget Set resource types .. dozens of them! */
#ifdef MOTIF_WIDGETS
	{ (XrmQuark)0, XmRArrowDirection, xm_arrow_direction,
			XtNumber(xm_arrow_direction), display_enumerated },
	{ (XrmQuark)0, XmRAlignment, xm_alignment,
			XtNumber(xm_alignment), display_enumerated },
	{ (XrmQuark)0, XmRAttachment, xm_attachment,
			XtNumber(xm_attachment), display_enumerated },
	{ (XrmQuark)0, XmRBooleanDimension, xm_boolean_dimension,
			XtNumber(xm_boolean_dimension), display_enumerated },
	{ (XrmQuark)0, XmRDefaultButtonType, xm_default_button_type,
			XtNumber(xm_default_button_type), display_enumerated },
	{ (XrmQuark)0, XmRCharSetTable, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRCommandWindowLocation, xm_command_window_location,
			XtNumber(xm_command_window_location),
			display_enumerated },
	{ (XrmQuark)0, XmRCompoundText, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRDefaultButtonType, xm_default_button_type,
			XtNumber(xm_default_button_type), display_enumerated },
	{ (XrmQuark)0, XmRDialogStyle, xm_dialog_style,
			XtNumber(xm_dialog_style), display_enumerated },
	{ (XrmQuark)0, XmRDialogType, xm_dialog_type,
			XtNumber(xm_dialog_type), display_enumerated },
	{ (XrmQuark)0, XmREditMode, xm_edit_mode,
			XtNumber(xm_edit_mode), display_enumerated },
	{ (XrmQuark)0, XmRFileTypeMask, xm_file_type_mask,
			XtNumber(xm_file_type_mask), display_enumerated },
	{ (XrmQuark)0, XmRFontList, (EnumList)NULL, 0, display_list },
	{ (XrmQuark)0, XmRGadgetPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRHorizontalDimension, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XmRHorizontalInt, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XmRHorizontalPosition, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XmRIndicatorType, xm_indicator_type, 
			XtNumber(xm_indicator_type), display_enumerated },
	{ (XrmQuark)0, XmRKeySym, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRKeySymTable, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRLabelType, xm_label_type, 
			XtNumber(xm_label_type), display_enumerated },
	{ (XrmQuark)0, XmRListSizePolicy, xm_list_size_policy,
			XtNumber(xm_list_size_policy), display_enumerated },
	{ (XrmQuark)0, XmRManBottomShadowPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRManHighlightPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRManTopShadowPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRMenuWidget, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRMnemonic, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRNavigationType, xm_navigation_type,
			XtNumber(xm_navigation_type), display_enumerated },
	{ (XrmQuark)0, XmROrientation, xm_orientation,
			XtNumber(xm_orientation), display_enumerated },
	{ (XrmQuark)0, XmRPacking, xm_packing,
			XtNumber(xm_packing), display_enumerated },
	{ (XrmQuark)0, XmRPrimBottomShadowPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRPrimHighlightPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRPrimTopShadowPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRProcessingDirection, xm_processing_direction,
			XtNumber(xm_processing_direction), display_enumerated },
	{ (XrmQuark)0, XmRResizePolicy, xm_resize_policy,
			XtNumber(xm_resize_policy), display_enumerated },
	{ (XrmQuark)0, XmRRowColumnType, xm_row_column_type,
			XtNumber(xm_row_column_type), display_enumerated },
	{ (XrmQuark)0, XmRScrollBarDisplayPolicy, xm_scrollbar_display_policy,
			XtNumber(xm_scrollbar_display_policy),
			display_enumerated },
	{ (XrmQuark)0, XmRScrollBarPlacement, xm_scrollbar_placement,
			XtNumber(xm_scrollbar_placement), display_enumerated },
	{ (XrmQuark)0, XmRScrollingPolicy, xm_scrolling_policy,
			XtNumber(xm_scrolling_policy), display_enumerated },
	{ (XrmQuark)0, XmRSelectionPolicy, xm_selection_policy,
			XtNumber(xm_selection_policy), display_enumerated },
	{ (XrmQuark)0, XmRSeparatorType, xm_separator_type,
			XtNumber(xm_separator_type), display_enumerated },
	{ (XrmQuark)0, XmRShadowType, xm_shadow_type,
			XtNumber(xm_shadow_type), display_enumerated },
	{ (XrmQuark)0, XmRShellHorizDim, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XmRShellHorizPos, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XmRShellVertDim, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XmRShellVertPos, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XmRStringCharSet, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRStringDirection, xm_string_direction,
			XtNumber(xm_string_direction), display_enumerated },
	{ (XrmQuark)0, XmRTextPosition, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRTraversalType, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XmRUnitType, xm_unit_type,
			XtNumber(xm_unit_type), display_enumerated },
	{ (XrmQuark)0, XmRVerticalDimension, (EnumList)NULL, 0, display_unsigned_value },
	{ (XrmQuark)0, XmRVerticalInt, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XmRVerticalPosition, (EnumList)NULL, 0, display_signed_value },
	{ (XrmQuark)0, XmRVisualPolicy, xm_visual_policy,
			XtNumber(xm_visual_policy), display_enumerated },
	{ (XrmQuark)0, XmRWhichButton, xm_which_button,
			XtNumber(xm_which_button), display_enumerated },
	{ (XrmQuark)0, XmRXmBackgroundPixmap, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRXmString, (EnumList)NULL, 0, display_string },
	{ (XrmQuark)0, XmRXmStringTable, (EnumList)NULL, 0, display_list },
#endif /* MOTIF_WIDGETS */

/* HP Widget set resource types */
#ifdef HP_WIDGETS
	{ (XrmQuark)0, XtRAlignment, xhp_alignment,
			XtNumber(xhp_alignment), display_enumerated },
	{ (XrmQuark)0, XtRArrowDirection, xhp_arrow_direction,
			XtNumber(xhp_arrow_direction), display_enumerated },
	{ (XrmQuark)0, XtRDestroyMode, xhp_destroy_mode,
			XtNumber(xhp_destroy_mode), display_enumerated },
	{ (XrmQuark)0, XtRElementHighlight, xhp_element_highlight,
			XtNumber(xhp_element_highlight), display_enumerated },
	{ (XrmQuark)0, XtRGravity, xhp_gravity, XtNumber(xhp_gravity), display_enumerated },
	{ (XrmQuark)0, XtRGrow, xhp_grow, XtNumber(xhp_grow), display_enumerated },
	{ (XrmQuark)0, XtRHighlightStyle, xhp_highlight_style, XtNumber(xhp_highlight_style), display_enumerated },
#if 0 /* ||| */
	{ (XrmQuark)0, XtRImage, xhp_image, XtNumber(xhp_image), display_enumerated },
#endif 0
	{ (XrmQuark)0, XtRLabelLocation, xhp_label_location, XtNumber(xhp_label_location), display_enumerated },
	{ (XrmQuark)0, XtRLabelType, xhp_label_type, XtNumber(xhp_label_type), display_enumerated },
	{ (XrmQuark)0, XtRLayout, xhp_layout, XtNumber(xhp_layout), display_enumerated },
	{ (XrmQuark)0, XtRLayoutType, xhp_layout_type, XtNumber(xhp_layout_type), display_enumerated },
#if 0 /* ||| */
	{ (XrmQuark)0, XtRMode, xhp_mode, XtNumber(xhp_mode), display_enumerated },
#endif 0
	{ (XrmQuark)0, XtRRCMode, xhp_rc_mode, XtNumber(xhp_rc_mode), display_enumerated },
	{ (XrmQuark)0, XtRScroll, xhp_scroll, XtNumber(xhp_scroll), display_enumerated },
	{ (XrmQuark)0, XtRSelectionBias, xhp_selection_bias, XtNumber(xhp_selection_bias), display_enumerated },
	{ (XrmQuark)0, XtRSelectionMethod, xhp_selection_method, XtNumber(xhp_selection_method), display_enumerated },
	{ (XrmQuark)0, XtRSelectionStyle, xhp_selection_style, XtNumber(xhp_selection_style), display_enumerated },
	{ (XrmQuark)0, XtRSeparatorType, xhp_separator_type, XtNumber(xhp_separator_type), display_enumerated },
	{ (XrmQuark)0, XtRSlideOrientation, xhp_slide_orientation, XtNumber(xhp_slide_orientation), display_enumerated },
	{ (XrmQuark)0, XtRSourceType, xhp_source_type, XtNumber(xhp_source_type), display_enumerated },
	{ (XrmQuark)0, XtRTileType, xhp_tile_type, XtNumber(xhp_tile_type), display_enumerated },
	{ (XrmQuark)0, XtRTitlePositionType, xhp_title_position_type, XtNumber(xhp_title_position_type), display_enumerated },
	{ (XrmQuark)0, XtRTitleType, xhp_title_type, XtNumber(xhp_title_type), display_enumerated },
	{ (XrmQuark)0, XtRTraversalType, xhp_traversal_type, XtNumber(xhp_traversal_type), display_enumerated },
	{ (XrmQuark)0, XtRWrap, xhp_wrap, XtNumber(xhp_wrap), display_enumerated },
	{ (XrmQuark)0, XtRWrapBreak, xhp_wrap_break, XtNumber(xhp_wrap_break), display_enumerated },
	{ (XrmQuark)0, XtRWrapForm, xhp_wrap_form, XtNumber(xhp_wrap_form), display_enumerated },
#endif /* HP_WIDGETS */

#if 0 /* resource types that make no sense for direct design */
/*Xmu*/	{ (XrmQuark)0, XtRCallProc, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRFunction, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRPointer, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRImmediate, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRWindow, (EnumList)NULL, 0, display_read_only },
	{ (XrmQuark)0, XtRDisplay, (EnumList)NULL, 0, display_read_only },
#endif /* 0 */
};
