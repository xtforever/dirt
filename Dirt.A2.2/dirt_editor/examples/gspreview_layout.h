#ifndef WI_gspreview_layout_h
#define WI_gspreview_layout_h
/* $Id: gspreview_layout.h,v 1.2 91/12/17 12:00:15 rlh2 Rel $
 *
 * X Toolkit program layout header file.
 * This file contains all the definitions of the static parts
 * of the user interface.
 */

/*
 * This file was generated by the DIRT interface editor
 *	Author: Richard Hesketh / rlh2@ukc.ac.uk
 *	Version: 1.3 (Wc) of July 1990
 *
 * Generated for rlh2 on Mon Dec 16 16:29:54 1991
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/RowCol.h>
#include <X11/Xukc/Toggle.h>
#include <X11/Xukc/Value.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Shell.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xukc/Viewport.h>
#include <X11/Xukc/Label.h>
#include <X11/Xukc/Command.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>

/* WIDGET/OBJECT class registration list */

struct w_classes {
	WidgetClass *class;
	String class_name;
};

/* WIDGET/OBJECT constructor registration list */

struct w_constructors {
	Widget (*constructor)();
	String constructor_name;
};

struct w_classes WI_gspreview_classes[] = {
	{ &ukcScrListWidgetClass, "ukcScrListWidgetClass" },
	{ &ukcRowColWidgetClass, "ukcRowColWidgetClass" },
	{ &ukcToggleWidgetClass, "ukcToggleWidgetClass" },
	{ &ukcValueWidgetClass, "ukcValueWidgetClass" },
	{ &asciiTextWidgetClass, "asciiTextWidgetClass" },
	{ &simpleWidgetClass, "simpleWidgetClass" },
	{ &ukcViewportWidgetClass, "ukcViewportWidgetClass" },
	{ &ukcLabelWidgetClass, "ukcLabelWidgetClass" },
	{ &ukcCommandWidgetClass, "ukcCommandWidgetClass" },
	{ &boxWidgetClass, "boxWidgetClass" },
	{ &formWidgetClass, "formWidgetClass" },
};

struct w_constructors WI_gspreview_constructors[] = {
	{ XtCreateTransientShell, "XtCreateTransientShell" },
};

/* WIDGET ID. VARIABLES */

Widget WI_gspreview;
Widget  WI_main_form;
Widget   WI_layout_box;
Widget    WI_file_button;
Widget    WI_next_button;
Widget    WI_prev_button;
Widget    WI_goto_button;
Widget    WI_title;
Widget   WI_quit;
Widget   WI_output_view;
Widget    WI_ps_window;
Widget   WI_label_form;
Widget    WI_page_title;
Widget    WI_page_label;
Widget    WI_filename_label;
Widget   WI_options_button;
Widget   WI_file_dialog_shell;
Widget    WI_file_dialog_layout;
Widget     WI_file_title;
Widget     WI_file_dialog_label;
Widget     WI_filename_entry;
Widget     WI_file_error_label;
Widget     WI_file_okay_button;
Widget     WI_file_cancel_button;
Widget   WI_page_dialog_shell;
Widget    WI_page_dialog_layout;
Widget     WI_page_dialog_title;
Widget     WI_page_dialog_label;
Widget     WI_page_value;
Widget     WI_page_okay_button;
Widget     WI_page_cancel_button;
Widget   WI_options_dialog_shell;
Widget    WI_options_form;
Widget     WI_options_title;
Widget     WI_options_page_size_label;
Widget     WI_options_a4_size;
Widget     WI_options_a3_size;
Widget     WI_options_orientation_label;
Widget     WI_options_portrait;
Widget     WI_options_landscape;
Widget     WI_options_normal;
Widget     WI_options_normal_bitmap;
Widget     WI_options_mirror;
Widget     WI_options_mirror_bitmap;
Widget     WI_options_flip;
Widget     WI_options_flip_bitmap;
Widget     WI_options_rotate;
Widget     WI_options_rotate_bitmap;
Widget     WI_options_r_c_box;
Widget      WI_options_apply_button;
Widget      WI_options_reset_button;
Widget      WI_options_dismiss;
Widget   WI_error_dialog_shell;
Widget    WI_error_dialog_layout;
Widget     WI_error_dialog_title;
Widget     WI_error_text_label;
Widget     WI_error_list_label1;
Widget     WI_error_list_label2;
Widget     WI_error_list_label3;
Widget     WI_operand_stack_list;
Widget     WI_execution_stack_list;
Widget     WI_dictionary_stack_list;
Widget     WI_error_r_c_box;
Widget      WI_error_acknowledged_button;
Widget   WI_output_dialog_shell;
Widget    WI_output_dialog_layout;
Widget     WI_output_dialog_title;
Widget     WI_output_viewport;
Widget      WI_output_text;
Widget     WI_output_r_c_box;
Widget      WI_output_acknowledged_button;


/* RUNTIME BINDING STRUCTURE */

struct w_bind_list {
	Widget *w;
	String str;
};

struct w_bind_list WI_gspreview_bind_list[] = {
	{ &WI_main_form, "*mainForm" },
	{ &WI_layout_box, "*layoutBox" },
	{ &WI_file_button, "*fileButton" },
	{ &WI_next_button, "*nextButton" },
	{ &WI_prev_button, "*prevButton" },
	{ &WI_goto_button, "*gotoButton" },
	{ &WI_title, "*title" },
	{ &WI_quit, "*quit" },
	{ &WI_output_view, "*outputView" },
	{ &WI_ps_window, "*psWindow" },
	{ &WI_label_form, "*labelForm" },
	{ &WI_page_title, "*pageTitle" },
	{ &WI_page_label, "*pageLabel" },
	{ &WI_filename_label, "*filenameLabel" },
	{ &WI_options_button, "*optionsButton" },
	{ &WI_file_dialog_shell, "*fileDialogShell" },
	{ &WI_file_dialog_layout, "*fileDialogLayout" },
	{ &WI_file_title, "*fileTitle" },
	{ &WI_file_dialog_label, "*fileDialogLabel" },
	{ &WI_filename_entry, "*filenameEntry" },
	{ &WI_file_error_label, "*fileErrorLabel" },
	{ &WI_file_okay_button, "*fileOkayButton" },
	{ &WI_file_cancel_button, "*fileCancelButton" },
	{ &WI_page_dialog_shell, "*pageDialogShell" },
	{ &WI_page_dialog_layout, "*pageDialogLayout" },
	{ &WI_page_dialog_title, "*pageDialogTitle" },
	{ &WI_page_dialog_label, "*pageDialogLabel" },
	{ &WI_page_value, "*pageValue" },
	{ &WI_page_okay_button, "*pageOkayButton" },
	{ &WI_page_cancel_button, "*pageCancelButton" },
	{ &WI_options_dialog_shell, "*optionsDialogShell" },
	{ &WI_options_form, "*optionsForm" },
	{ &WI_options_title, "*optionsTitle" },
	{ &WI_options_page_size_label, "*optionsPageSizeLabel" },
	{ &WI_options_a4_size, "*optionsA4Size" },
	{ &WI_options_a3_size, "*optionsA3Size" },
	{ &WI_options_orientation_label, "*optionsOrientationLabel" },
	{ &WI_options_portrait, "*optionsPortrait" },
	{ &WI_options_landscape, "*optionsLandscape" },
	{ &WI_options_normal, "*optionsNormal" },
	{ &WI_options_normal_bitmap, "*optionsNormalBitmap" },
	{ &WI_options_mirror, "*optionsMirror" },
	{ &WI_options_mirror_bitmap, "*optionsMirrorBitmap" },
	{ &WI_options_flip, "*optionsFlip" },
	{ &WI_options_flip_bitmap, "*optionsFlipBitmap" },
	{ &WI_options_rotate, "*optionsRotate" },
	{ &WI_options_rotate_bitmap, "*optionsRotateBitmap" },
	{ &WI_options_r_c_box, "*optionsRCBox" },
	{ &WI_options_apply_button, "*optionsApplyButton" },
	{ &WI_options_reset_button, "*optionsResetButton" },
	{ &WI_options_dismiss, "*optionsDismiss" },
	{ &WI_error_dialog_shell, "*errorDialogShell" },
	{ &WI_error_dialog_layout, "*errorDialogLayout" },
	{ &WI_error_dialog_title, "*errorDialogTitle" },
	{ &WI_error_text_label, "*errorTextLabel" },
	{ &WI_error_list_label1, "*errorListLabel1" },
	{ &WI_error_list_label2, "*errorListLabel2" },
	{ &WI_error_list_label3, "*errorListLabel3" },
	{ &WI_operand_stack_list, "*operandStackList" },
	{ &WI_execution_stack_list, "*executionStackList" },
	{ &WI_dictionary_stack_list, "*dictionaryStackList" },
	{ &WI_error_r_c_box, "*errorRCBox" },
	{ &WI_error_acknowledged_button, "*errorAcknowledgedButton" },
	{ &WI_output_dialog_shell, "*outputDialogShell" },
	{ &WI_output_dialog_layout, "*outputDialogLayout" },
	{ &WI_output_dialog_title, "*outputDialogTitle" },
	{ &WI_output_viewport, "*outputViewport" },
	{ &WI_output_text, "*outputText" },
	{ &WI_output_r_c_box, "*outputRCBox" },
	{ &WI_output_acknowledged_button, "*outputAcknowledgedButton" },
};


/* CALLBACK REGISTRATION LIST */

String WI_gspreview_callback_names[] = {
	"quit_gsp",
	"previous_page",
	"next_page",
	"load_file",
	"goto_page",
	"clear_output",
	"clear_load_error",
	"clear_errors",
	"centre_dialog",
};

/* ACTION REGISTRATION LIST */

String WI_gspreview_action_names[] = {
	"reread_file",
	"previous_page",
	"next_page",
	"load_file",
	"gs_quit",
	"goto_page",
	"goto_last",
	"goto_first",
	"do_count",
	"cancel_count",
};


/* LAYOUT CONNECTIONS */
/* END */
#endif /* WI_gspreview_layout_h */
