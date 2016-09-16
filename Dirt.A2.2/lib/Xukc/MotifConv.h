/* SCCSID: @(#)MotifConv.h	1.1 (UKC) 5/10/92 */

/* 
 * Copyright 1991 Richard Hesketh / rlh2@ukc.ac.uk
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

#ifdef MOTIF_WIDGETS
/* Motif Widget Set */

#include <Xm/Xm.h>
#include "Convert.h"

static EnumListRec xm_navigation_type[] = {
        { "none", (int)XmNONE },
	{ "tab_group", (int)XmTAB_GROUP },
	{ "stick_tab_group", (int)XmSTICKY_TAB_GROUP },
	{ "exclusive_tab_group", (int)XmEXCLUSIVE_TAB_GROUP },
};

static XtConvertArgRec xm_navigation_type_Args[] = {
        { XtAddress,    (XtPointer)xm_navigation_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_navigation_type), sizeof(Cardinal) },
        { XtAddress,    XmRNavigationType,  sizeof(String) },
};

/*****/

static EnumListRec xm_arrow_direction[] = {
        { "arrow_up", (int)XmARROW_UP },
	{ "arrow_down", (int)XmARROW_DOWN },
	{ "arrow_right", (int)XmARROW_RIGHT },
};

static XtConvertArgRec xm_arrow_direction_Args[] = {
        { XtAddress,    (XtPointer)xm_arrow_direction, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_arrow_direction), sizeof(Cardinal) },
        { XtAddress,    XmRArrowDirection,  sizeof(String) },
};

/******/

static EnumListRec xm_unit_type[] = {
	{ "pixels", (int)XmPIXELS },
	{ "100th_millimeters", (int)Xm100TH_MILLIMETERS },
	{ "1000th_inches", (int)Xm1000TH_INCHES },
	{ "100th_points", (int)Xm100TH_POINTS },
	{ "100th_font_units", (int)Xm100TH_FONT_UNITS },
};

static XtConvertArgRec xm_unit_type_Args[] = {
        { XtAddress,    (XtPointer)xm_unit_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_unit_type), sizeof(Cardinal) },
        { XtAddress,    XmRUnitType,  sizeof(String) },
};

/******/

static EnumListRec xm_shadow_type[] = {
	{ "shadow_etched_in", (int)XmSHADOW_ETCHED_IN },
	{ "shadow_etched_out", (int)XmSHADOW_ETCHED_OUT },
	{ "shadow_in", (int)XmSHADOW_IN },
	{ "shadow_out", (int)XmSHADOW_OUT },
};

static XtConvertArgRec xm_shadow_type_Args[] = {
        { XtAddress,    (XtPointer)xm_shadow_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_shadow_type), sizeof(Cardinal) },
        { XtAddress,    XmRShadowType,  sizeof(String) },
};

/******/

static EnumListRec xm_resize_policy[] = {
	{ "resize_none", (int)XmRESIZE_NONE },
	{ "resize_grow", (int)XmRESIZE_GROW },
	{ "resize_any", (int)XmRESIZE_ANY },
};

static XtConvertArgRec xm_resize_policy_Args[] = {
        { XtAddress,    (XtPointer)xm_resize_policy, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_resize_policy), sizeof(Cardinal) },
        { XtAddress,    XmRResizePolicy,  sizeof(String) },
};

/******/

static EnumListRec xm_dialog_style[] = {
	{ "dialog_work_area", (int)XmDIALOG_WORK_AREA },
	{ "dialog_modeless", (int)XmDIALOG_MODELESS },
	{ "dialog_application_modal", (int)XmDIALOG_APPLICATION_MODAL },
	{ "dialog_full_application_modal", (int)XmDIALOG_FULL_APPLICATION_MODAL },
	{ "dialog_primary_application_modal",
				(int)XmDIALOG_PRIMARY_APPLICATION_MODAL },
	{ "dialog_system_modal", (int)XmDIALOG_SYSTEM_MODAL },
};

static XtConvertArgRec xm_dialog_style_Args[] = {
        { XtAddress,    (XtPointer)xm_dialog_style, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_dialog_style), sizeof(Cardinal) },
        { XtAddress,    XmRDialogStyle,  sizeof(String) }
};

/******/

static EnumListRec xm_string_direction[] = {
	{ "string_direction_l_to_r", (int)XmSTRING_DIRECTION_L_TO_R },
	{ "string_direction_r_to_l", (int)XmSTRING_DIRECTION_R_TO_L },
};

static XtConvertArgRec xm_string_direction_Args[] = {
        { XtAddress,    (XtPointer)xm_string_direction, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_string_direction), sizeof(Cardinal) },
        { XtAddress,    XmRStringDirection,  sizeof(String) }
};

/******/

static EnumListRec xm_label_type[] = {
	{ "string", (int)XmSTRING },
	{ "pixmap", (int)XmPIXMAP },
};

static XtConvertArgRec xm_label_type_Args[] = {
        { XtAddress,    (XtPointer)xm_label_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_label_type), sizeof(Cardinal) },
        { XtAddress,    XmRLabelType,  sizeof(String) }
};

/******/

static EnumListRec xm_dialog_type[] = {
	{ "dialog_error", (int)XmDIALOG_ERROR },
	{ "dialog_information", (int)XmDIALOG_INFORMATION },
	{ "dialog_message", (int)XmDIALOG_MESSAGE },
	{ "dialog_question", (int)XmDIALOG_QUESTION },
	{ "dialog_warning", (int)XmDIALOG_WARNING },
	{ "dialog_working", (int)XmDIALOG_WORKING },
	{ "dialog_prompt", (int)XmDIALOG_PROMPT },
	{ "dialog_selection", (int)XmDIALOG_SELECTION },
	{ "dialog_command", (int)XmDIALOG_COMMAND },
};

static XtConvertArgRec xm_dialog_type_Args[] = {
        { XtAddress,    (XtPointer)xm_dialog_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_dialog_type), sizeof(Cardinal) },
        { XtAddress,    XmRDialogType,  sizeof(String) }
};

/******/

static EnumListRec xm_file_type_mask[] = {
	{ "file_directory", (int)XmFILE_DIRECTORY },
	{ "file_regular", (int)XmFILE_REGULAR },
	{ "file_any_type", (int)XmFILE_ANY_TYPE },
};

static XtConvertArgRec xm_file_type_mask_Args[] = {
        { XtAddress,    (XtPointer)xm_file_type_mask, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_file_type_mask), sizeof(Cardinal) },
        { XtAddress,    XmRFileTypeMask,  sizeof(String) }
};

/******/

static EnumListRec xm_alignment[] = {
	{ "alignment_center", (int)XmALIGNMENT_CENTER },
	{ "alignment_beginning", (int)XmALIGNMENT_BEGINNING },
	{ "alignment_end", (int)XmALIGNMENT_END },
};

static XtConvertArgRec xm_alignment_Args[] = {
        { XtAddress,    (XtPointer)xm_alignment, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_alignment), sizeof(Cardinal) },
        { XtAddress,    XmRAlignment,  sizeof(String) }
};

/******/

static EnumListRec xm_selection_policy[] = {
	{ "single_select", (int)XmSINGLE_SELECT },
	{ "multiple_select", (int)XmMULTIPLE_SELECT },
	{ "extended_select", (int)XmEXTENDED_SELECT },
	{ "browse_select", (int)XmBROWSE_SELECT },
};

static XtConvertArgRec xm_selection_policy_Args[] = {
        { XtAddress,    (XtPointer)xm_selection_policy, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_selection_policy), sizeof(Cardinal) },
        { XtAddress,    XmRSelectionPolicy,  sizeof(String) }
};

/******/

static EnumListRec xm_list_size_policy[] = {
	{ "constant", (int)XmCONSTANT },
	{ "variable", (int)XmVARIABLE },
	{ "resize_if_possible", (int)XmRESIZE_IF_POSSIBLE },
};

static XtConvertArgRec xm_list_size_policy_Args[] = {
        { XtAddress,    (XtPointer)xm_list_size_policy, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_list_size_policy), sizeof(Cardinal) },
        { XtAddress,    XmRListSizePolicy,  sizeof(String) }
};

/******/

static EnumListRec xm_scrollbar_display_policy[] = {
	{ "as_needed", (int)XmAS_NEEDED },
	{ "static", (int)XmSTATIC },
};

static XtConvertArgRec xm_scrollbar_display_policy_Args[] = {
        { XtAddress,    (XtPointer)xm_scrollbar_display_policy, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_scrollbar_display_policy), sizeof(Cardinal) },
        { XtAddress,    XmRScrollBarDisplayPolicy,  sizeof(String) }
};

/******/

static EnumListRec xm_scrolling_policy[] = {
	{ "automatic", (int)XmAUTOMATIC },
	{ "application_defined", (int)XmAPPLICATION_DEFINED },
};

static XtConvertArgRec xm_scrolling_policy_Args[] = {
        { XtAddress,    (XtPointer)xm_scrolling_policy, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_scrolling_policy), sizeof(Cardinal) },
        { XtAddress,    XmRScrollingPolicy,  sizeof(String) }
};

/******/

static EnumListRec xm_visual_policy[] = {
	{ "variable", (int)XmVARIABLE },
	{ "constant", (int)XmCONSTANT },
};

static XtConvertArgRec xm_visual_policy_Args[] = {
        { XtAddress,    (XtPointer)xm_visual_policy, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_visual_policy), sizeof(Cardinal) },
        { XtAddress,    XmRVisualPolicy,  sizeof(String) }
};

/******/

static EnumListRec xm_scrollbar_placement[] = {
	{ "top_left", (int)XmTOP_LEFT },
	{ "top_right", (int)XmTOP_RIGHT },
	{ "bottom_left", (int)XmBOTTOM_LEFT },
	{ "bottom_right", (int)XmBOTTOM_RIGHT },
};

static XtConvertArgRec xm_scrollbar_placement_Args[] = {
        { XtAddress,    (XtPointer)xm_scrollbar_placement, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_scrollbar_placement), sizeof(Cardinal) },
        { XtAddress,    XmRScrollBarPlacement,  sizeof(String) }
};

/******/

static EnumListRec xm_command_window_location[] = {
	{ "command_above_workspace", (int)XmCOMMAND_ABOVE_WORKSPACE },
	{ "command_below_workspace", (int)XmCOMMAND_BELOW_WORKSPACE },
};

static XtConvertArgRec xm_command_window_location_Args[] = {
        { XtAddress,    (XtPointer)xm_command_window_location, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_command_window_location), sizeof(Cardinal) },
        { XtAddress,    XmRCommandWindowLocation,  sizeof(String) }
};

/******/

static EnumListRec xm_processing_direction[] = {
	{ "max_on_top", (int)XmMAX_ON_TOP },
	{ "max_on_bottom", (int)XmMAX_ON_BOTTOM },
	{ "max_on_left", (int)XmMAX_ON_LEFT },
	{ "max_on_right", (int)XmMAX_ON_RIGHT },
};

static XtConvertArgRec xm_processing_direction_Args[] = {
        { XtAddress,    (XtPointer)xm_processing_direction, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_processing_direction), sizeof(Cardinal) },
        { XtAddress,    XmRProcessingDirection,  sizeof(String) }
};

/******/

static EnumListRec xm_separator_type[] = {
	{ "no_line", (int)XmNO_LINE },
	{ "single_line", (int)XmSINGLE_LINE },
	{ "double_line", (int)XmDOUBLE_LINE },
	{ "single_dashed_line", (int)XmSINGLE_DASHED_LINE },
	{ "double_dashed_line", (int)XmDOUBLE_DASHED_LINE },
	{ "shadow_etched_in", (int)XmSHADOW_ETCHED_IN },
	{ "shadow_etched_out", (int)XmSHADOW_ETCHED_OUT },
};

static XtConvertArgRec xm_separator_type_Args[] = {
        { XtAddress,    (XtPointer)xm_separator_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_separator_type), sizeof(Cardinal) },
        { XtAddress,    XmRSeparatorType,  sizeof(String) }
};

/******/

static EnumListRec xm_which_button[] = {
	{ "button1", (int)Button1 },
	{ "button2", (int)Button2 },
	{ "button3", (int)Button3 },
	{ "button4", (int)Button4 },
	{ "button5", (int)Button5 },
};

static XtConvertArgRec xm_which_button_Args[] = {
        { XtAddress,    (XtPointer)xm_which_button, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_which_button), sizeof(Cardinal) },
        { XtAddress,    XmRWhichButton,  sizeof(String) }
};

/******/

static EnumListRec xm_packing[] = {
	{ "pack_none", (int)XmPACK_NONE },
	{ "pack_tight", (int)XmPACK_TIGHT },
	{ "pack_column", (int)XmPACK_COLUMN },
};

static XtConvertArgRec xm_packing_Args[] = {
        { XtAddress,    (XtPointer)xm_packing, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_packing), sizeof(Cardinal) },
        { XtAddress,    XmRPacking,  sizeof(String) }
};

/******/

static EnumListRec xm_orientation[] = {
	{ "vertical", (int)XmVERTICAL },
	{ "horizontal", (int)XmHORIZONTAL },
};

static XtConvertArgRec xm_orientation_Args[] = {
        { XtAddress,    (XtPointer)xm_orientation, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_orientation), sizeof(Cardinal) },
        { XtAddress,    XmROrientation,  sizeof(String) }
};

/******/

static EnumListRec xm_attachment[] = {
	{ "attach_none", (int)XmATTACH_NONE },
	{ "attach_form", (int)XmATTACH_FORM },
	{ "attach_opposite_form", (int)XmATTACH_OPPOSITE_FORM },
	{ "attach_widget", (int)XmATTACH_WIDGET },
	{ "attach_opposite_widget", (int)XmATTACH_OPPOSITE_WIDGET },
	{ "attach_position", (int)XmATTACH_POSITION },
	{ "attach_self", (int)XmATTACH_SELF },
};

static XtConvertArgRec xm_attachment_Args[] = {
        { XtAddress,    (XtPointer)xm_attachment, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_attachment), sizeof(Cardinal) },
        { XtAddress,    XmRAttachment,  sizeof(String) }
};

/******/

static EnumListRec xm_indicator_type[] = {
	{ "n_of_many", (int)XmN_OF_MANY },
	{ "one_of_many", (int)XmONE_OF_MANY },
};

static XtConvertArgRec xm_indicator_type_Args[] = {
        { XtAddress,    (XtPointer)xm_indicator_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_indicator_type), sizeof(Cardinal) },
        { XtAddress,    XmRIndicatorType,  sizeof(String) }
};

/******/

static EnumListRec xm_edit_mode[] = {
	{ "single_line_edit", (int)XmSINGLE_LINE_EDIT },
	{ "multi_line_edit", (int)XmMULTI_LINE_EDIT },
};

static XtConvertArgRec xm_edit_mode_Args[] = {
        { XtAddress,    (XtPointer)xm_edit_mode, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_edit_mode), sizeof(Cardinal) },
        { XtAddress,    XmREditMode,  sizeof(String) }
};

/******/

static EnumListRec xm_default_button_type[] = {
	{ "dialog_ok_button", (int)XmDIALOG_OK_BUTTON },
	{ "dialog_cancel_button", (int)XmDIALOG_CANCEL_BUTTON },
	{ "dialog_help_button", (int)XmDIALOG_HELP_BUTTON },
};

static XtConvertArgRec xm_default_button_type_Args[] = {
        { XtAddress,    (XtPointer)xm_default_button_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_default_button_type), sizeof(Cardinal) },
        { XtAddress,    XmRDefaultButtonType,  sizeof(String) }
};

/******/

static EnumListRec xm_row_column_type[] = {
	{ "work_area", (int)XmWORK_AREA },
	{ "menu_bar", (int)XmMENU_BAR },
	{ "menu_popup", (int)XmMENU_POPUP },
	{ "menu_pulldown", (int)XmMENU_PULLDOWN },
	{ "menu_option", (int)XmMENU_OPTION },
};

static XtConvertArgRec xm_row_column_type_Args[] = {
        { XtAddress,    (XtPointer)xm_row_column_type, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_row_column_type), sizeof(Cardinal) },
        { XtAddress,    XmRRowColumnType,  sizeof(String) }
};

/******/

static EnumListRec xm_boolean_dimension[] = {
	{ "true", (int)1 },
	{ "false", (int)0 },
};

static XtConvertArgRec xm_boolean_dimension_Args[] = {
        { XtAddress,    (XtPointer)xm_boolean_dimension, sizeof(EnumList) },
        { XtImmediate,  (XtPointer)XtNumber(xm_boolean_dimension), sizeof(Cardinal) },
        { XtAddress,    XmRBooleanDimension,  sizeof(String) }
};
#endif /* MOTIF_WIDGETS */
