/* SCCSID: @(#)Dirt.h	1.1 (UKC) 5/10/92 */

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

static String app_defaults[] = {
#ifdef MOTIF_WIDGETS
	/* Another bug in Motif which requires all shells to have an
	 * a non-NULL iconName resource */
        "*iconName: Dirt",
#endif /* MOTIF_WIDGETS */

	/* force these popups to come up at a calculated place */
	"Dirt*createShell.x: -9999",
	"Dirt*commentShell.x: -9999",
	"Dirt*newShell.x: -9999",
	"Dirt*instanceShell.x: -9999",
	"Dirt*actionStubsShell.x: -9999",
	"Dirt*callbackStubsShell.x: -9999",
	"Dirt*fileDialogShell.x: 9999",
	"Dirt*loadDialogShell.x: 9999",

	"Dirt*resourcesTable.columns: 2",

	"Dirt*resourcesTable.resourceName.borderWidth: 0",
	"Dirt*resourcesTable.resourceName.x: 1",
	"Dirt*resourcesTable.resourceName.height: 27",
	"Dirt*resourcesTable.resourceName.justify: right",

	"Dirt*resourcesTable.stringEdit*editType: edit",
	"Dirt*resourcesTable.stringEdit.borderWidth: 1",
	"Dirt*resourcesTable.stringEdit.borderColor: XtDefaultBackground",
	"Dirt*resourcesTable.stringEdit.width: 200",
	"Dirt*resourcesTable.stringEdit.height: 24",
	"Dirt*resourcesTable.stringEdit.resize: both",

	"Dirt*resourcesTable.callbackEdit*editType: edit",
	"Dirt*resourcesTable.callbackEdit.borderWidth: 1",
	"Dirt*resourcesTable.callbackEdit.borderColor: XtDefaultBackground",
	"Dirt*resourcesTable.callbackEdit.width: 200",
	"Dirt*resourcesTable.callbackEdit.height: 24",
	"Dirt*resourcesTable.callbackEdit.resize: both",

	"Dirt*resourcesTable.booleanToggle.onBitmap: tick",
	"Dirt*resourcesTable.booleanToggle.offBitmap: cross",
	"Dirt*resourcesTable.booleanToggle.label:",
	"Dirt*resourcesTable.booleanToggle.borderWidth: 0",

	"Dirt*resourcesTable.enumeratedBox.borderWidth: 0",
	"Dirt*resourcesTable.enumeratedBox.hSpace: 0",
	"Dirt*resourcesTable.enumeratedBox.layoutType: requestedColumns",
	"Dirt*resourcesTable.enumeratedBox.columns: 1",

	"Dirt*resourcesTable*enumeratedToggle.onBitmap: on_circle",
	"Dirt*resourcesTable*enumeratedToggle.offBitmap: off_circle",
	"Dirt*resourcesTable*enumeratedToggle.borderWidth: 0",

	"Dirt*resourcesTable.readOnly.borderWidth: 0",
	"Dirt*resourcesTable.readOnly.justify: left",

	"Dirt*resourcesTable.editableSignedValue.max: 9999",
	"Dirt*resourcesTable.editableSignedValue.min: -9999",

	"Dirt*resourcesTable.editableUnsignedValue.max: 9999",
	"Dirt*resourcesTable.editableUnsignedValue.min: 0",

	"Dirt*resourcesTable.transButton.label: Press To Edit",
	"Dirt*resourcesTable.transButton.borderWidth: 2",

	"Dirt*resourcesTable.widgetButton.borderWidth: 2",
	"Dirt*resourcesTable.widgetButton.width: 150",
	"Dirt*resourcesTable.widgetButton.justify: left",
	"Dirt*resourcesTable.widgetButton.editable: true",
	"Dirt*resourcesTable.widgetButton.overflowRight: false",
	"Dirt*resourcesTable.widgetButton.resize: true",
	"Dirt*resourcesTable.widgetButton.translations: #augment\\n\
		<Btn3Up>:  dirtACTPickObject()",

	"Dirt*resourcesTable.widgetListBox.widgetButton.borderWidth: 2",
	"Dirt*resourcesTable.widgetListBox.widgetButton.resize: true",

	"Dirt*resourcesTable.fontButton.borderWidth: 2",
	"Dirt*resourcesTable.fontButton.overflowRight: false",
	"Dirt*resourcesTable.fontButton.editable: true",
	"Dirt*resourcesTable.fontButton.resize: true",
	"Dirt*resourcesTable.fontButton.justify: left",

	"Dirt*resourcesTable.colourButton.borderWidth: 2",
	"Dirt*resourcesTable.colourButton.overflowRight: false",
	"Dirt*resourcesTable.colourButton.editable: true",
	"Dirt*resourcesTable.colourButton.justify: left",

	"Dirt*translationsLayout*List.defaultColumns: 1",
	"Dirt*translationsLayout.List.verticalList: true",
	"Dirt*translationsLayout.List.forceColumns: true",
	"Dirt*translationsLayout.List.translations: \
		<Btn1Down>:    Set() \\n\
		<Btn1Up>:      Notify() Unset() \\n\
		Button1<LeaveWindow>:  Unset() \\n\
		Button1<EnterWindow>:  Set()",
};


static String gen_defaults[] = {
#ifdef MOTIF_WIDGETS
	/* Another "feature" of Motif which requires all shells to have an
	 * a non-NULL iconName resource */
        "*iconName: ",
#endif /* MOTIF_WIDGETS */

	/* widget specific defaults */

	"*_dirtTopLeftGrip.cursor: top_left_corner",
	"*_dirtTopLeftGrip.borderWidth: 1",
	"*_dirtTopMiddleGrip.cursor: top_side",
	"*_dirtTopMiddleGrip.borderWidth: 1",
	"*_dirtTopRightGrip.cursor: top_right_corner",
	"*_dirtTopRightGrip.borderWidth: 1",

	"*_dirtMiddleLeftGrip.cursor: left_side",
	"*_dirtMiddleLeftGrip.borderWidth: 1",
	"*_dirtMiddleGrip.cursor: tcross",
	"*_dirtMiddleGrip.borderWidth: 1",
	"*_dirtMiddleRightGrip.cursor: right_side",
	"*_dirtMiddleRightGrip.borderWidth: 1",

	"*_dirtBottomLeftGrip.cursor: bottom_left_corner",
	"*_dirtBottomLeftGrip.borderWidth: 1",
	"*_dirtBottomMiddleGrip.cursor: bottom_side",
	"*_dirtBottomMiddleGrip.borderWidth: 1",
	"*_dirtBottomRightGrip.cursor: bottom_right_corner",
	"*_dirtBottomRightGrip.borderWidth: 1",
};
