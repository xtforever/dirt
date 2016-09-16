/* SCCSID: @(#)athena_examples.h	1.1 (UKC) 5/10/92 */

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

/* Header file for the Athena widget set examples */

extern Widget WI_athena_examples, WI_athena_current_class;
extern Widget WI_athena_examples_hide, WI_athena_examples_layout;
extern Widget WI_athena_box, WI_athena_clock, WI_athena_command;
extern Widget WI_athena_dialog_box;
extern Widget WI_athena_editable_text, WI_athena_form, WI_athena_label;
extern Widget WI_athena_grip, WI_athena_mailbox, WI_athena_paned;
extern Widget WI_athena_toggle, WI_athena_logo, WI_athena_scroll_bar;
extern Widget WI_athena_strip_chart, WI_athena_viewport;

/* only here for layout connections */
extern Widget WI_athena_examples_title;
extern Widget WI_athena_examples_box_label, WI_athena_form_label;
extern Widget WI_athena_toggle_label, WI_athena_paned_label;
extern Widget WI_athena_grip_label;
extern Widget WI_athena_popup_a_menu, WI_athena_stripchart_label;
extern Widget WI_athena_mailbox_label, WI_athena_viewport_label;

extern WidgetClass boxWidgetClass, clockWidgetClass, commandWidgetClass;
extern WidgetClass dialogWidgetClass, asciiTextWidgetClass, formWidgetClass;
extern WidgetClass labelWidgetClass, gripWidgetClass, mailboxWidgetClass;
extern WidgetClass panedWidgetClass, toggleWidgetClass, logoWidgetClass;
extern WidgetClass scrollbarWidgetClass, stripChartWidgetClass;
extern WidgetClass viewportWidgetClass;

static struct _examples athena_ex[] = {
	{ &WI_athena_box, &boxWidgetClass },
	{ &WI_athena_clock, &clockWidgetClass },
	{ &WI_athena_command, &commandWidgetClass },
	{ &WI_athena_dialog_box, &dialogWidgetClass },
	{ &WI_athena_editable_text, &asciiTextWidgetClass },
	{ &WI_athena_form, &formWidgetClass },
	{ &WI_athena_label, &labelWidgetClass },
	{ &WI_athena_grip, &gripWidgetClass },
	{ &WI_athena_mailbox, &mailboxWidgetClass },
	{ &WI_athena_paned, &panedWidgetClass },
	{ &WI_athena_toggle, &toggleWidgetClass },
	{ &WI_athena_logo, &logoWidgetClass },
	{ &WI_athena_scroll_bar, &scrollbarWidgetClass },
	{ &WI_athena_strip_chart, &stripChartWidgetClass },
	{ &WI_athena_viewport, &viewportWidgetClass },
};
