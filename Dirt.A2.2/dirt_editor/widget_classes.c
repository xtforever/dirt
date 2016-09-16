#ifndef lint
static char sccsid[] = "@(#)widget_classes.c	1.1 (UKC) 5/10/92";
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

/* Intrinsics */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Vendor.h>

#ifdef XAW
/* Athena Widgets */
#ifndef XAW_HEADERS
#define XAW_HEADERS "Xaw"
#endif /* !XAW_HEADERS */

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Logo.h>
#include <X11/Xaw/Mailbox.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
/* New Athena Widgets added at X11R5 */
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Repeater.h>
#include <X11/Xaw/Tree.h>
#endif /* XAW R5 */
#endif /* XAW */

#ifdef UKC_WIDGETS
/* My Widgets */
#ifndef XUKC_HEADERS
#define XUKC_HEADERS "Xukc"
#endif /* !XUKC_HEADERS */

#include <X11/Xukc/Arrow.h>
#include <X11/Xukc/Command.h>
#include <X11/Xukc/Error.h>
#if 0
#include	<X11/Xukc/Glue.h>
#endif
#include <X11/Xukc/Layout.h>
#include <X11/Xukc/Med3Menu.h>
#include <X11/Xukc/MenuPane.h>
#include <X11/Xukc/Mouse.h>
#include <X11/Xukc/Passive.h>
#include <X11/Xukc/RowCol.h>
#include <X11/Xukc/Scaled.h>
#include <X11/Xukc/Scrollbar.h>
#include <X11/Xukc/ScrollList.h>
#include <X11/Xukc/Toggle.h>
#include <X11/Xukc/Value.h>
#include <X11/Xukc/Viewport.h>
#endif /* UKC_WIDGETS */

#ifdef HP_WIDGETS
/* HP Widget Set */
#ifndef XHP_HEADERS
#define XHP_HEADERS "Xw"
#endif /* !XHP_HEADERS */

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
#endif /* HP_WIDGETS */

#ifdef MOTIF_WIDGETS
/* Motif Widget Set */
#ifndef MOTIF_HEADERS
#define MOTIF_HEADERS "Xm"
#endif /* !MOTIF_HEADERS */

#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/ArrowBG.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/DrawnB.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#endif /* MOTIF_WIDGETS */

#ifdef OL_WIDGETS
/* Xt+ OpenLook (tm: AT&T) Widget Set */
#ifndef OL_HEADERS
#define OL_HEADERS "Xol"
#endif /* OL_HEADERS */

#include <Xol/OpenLook.h>

/* really necessary ?? */
#include <Xol/Arrow.h>
#include <Xol/Check.h>
#include <Xol/ListPane.h>
#include <Xol/Mag.h>
#include <Xol/Pushpin.h>
#include <Xol/TextPane.h>
#include <Xol/Help.h>

#include <Xol/AbbrevMenu.h>
#include <Xol/AbbrevStac.h>
#include <Xol/BaseWindow.h>
#include <Xol/BulletinBo.h>
#include <Xol/Button.h>
#include <Xol/ButtonStac.h>
#include <Xol/Caption.h>
#include <Xol/CheckBox.h>
#include <Xol/ControlAre.h>
#include <Xol/Dynamic.h>
#include <Xol/EventObj.h>
#include <Xol/Exclusives.h>
#include <Xol/FCheckBox.h>
#include <Xol/FExclusive.h>
#include <Xol/FNonexclus.h>
#include <Xol/Flat.h>
#include <Xol/FooterPane.h>
#include <Xol/Form.h>
#include <Xol/Manager.h>
#include <Xol/Menu.h>
#include <Xol/MenuButton.h>
#include <Xol/Nonexclusi.h>
#include <Xol/Notice.h>
#include <Xol/OblongButt.h>
#include <Xol/PopupWindo.h>
#include <Xol/Primitive.h>
#include <Xol/RectButton.h>
#include <Xol/Scrollbar.h>
#include <Xol/ScrolledWi.h>
#include <Xol/ScrollingL.h>
#include <Xol/Slider.h>
#include <Xol/StaticText.h>
#include <Xol/Stub.h>
#include <Xol/Text.h>
#include <Xol/TextEdit.h>
#include <Xol/TextField.h>
#endif /* OL_WIDGETS */

#ifdef MISC_WIDGETS
#ifndef MISC_HEADERS
#define MISC_HEADERS "Xmisc"
#endif /* !MISC_HEADERS */

#include <X11/Xmisc/Table.h>
#endif /* MISC_WIDGETS */

#include "dirt_structures.h"

#define XT_HEADERS (String)NULL

/* Include the exception lists for the widget classes .. these are lists
 * of resources that cannot be edited.
 */
#include "exceptions.h"

/* Wc registered constructors.
 */
extern Widget WcCreateApplicationShell();
extern Widget WcCreateOverrideShell();
extern Widget WcCreateShell();
extern Widget WcCreateTopLevelShell();
extern Widget WcCreateTransientShell();
extern Widget WcCreateVendorShell();
extern Widget WcCreateWMShell();

/* {
 *   widgetClassPtr,
 *   acceptable child object classes, number of acceptable classes,
 *   constructor string, constructor function pointer,
 *   include file prefix, include file,
 *   displayed class name, internal class name,
 *   description,
 *   exception resource list, number of exceptions
 * },
 */
WidgetDefsRec widget_classes[] = {

/* widget classes defined in the Xt Intrinsics */

{
   (WidgetClass *)&objectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XT_HEADERS, "Object.h",
   "Object", "objectClass",
   "Root of all objects",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&rectObjClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XT_HEADERS, "RectObj.h",
   "RectObj", "rectObjClass",
   "Root of all rectangular objects",
   rect_object_res, XtNumber(rect_object_res)
},{
   (WidgetClass *)&widgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XT_HEADERS, "Core.h",
   "Core", "coreWidgetClass",
   "Basic \"Core\" widget class",
   core_res, XtNumber(core_res)
},{
   (WidgetClass *)&compositeWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XT_HEADERS, "Composite.h",
   "Composite", "compositeWidgetClass",
   "Basic \"Composite\" widget classes which manages children",
   composite_res, XtNumber(composite_res)
},{
   (WidgetClass *)&constraintWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XT_HEADERS, "Constraint.h",
   "Constraint", "constraintWidgetClass",
   "Sub-class of Composite: allows children to be constrained by certain rules",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&shellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateShell", WcCreateShell,
   XT_HEADERS, "Shell.h",
   "Shell", "shellWidgetClass",
   "Basic popup shell widget class: routes all the communication with the window manager",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&overrideShellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateOverrideShell", WcCreateOverrideShell,
   XT_HEADERS, "Shell.h",
   "WM Override Shell", "overrideShellWidgetClass",
   "Popup shell which overrides window manager control",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&wmShellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateWMShell", WcCreateWMShell,
   XT_HEADERS, "Shell.h",
   "WM Shell", "wmShellWidgetClass", "Popup shell",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&transientShellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateTransientShell", WcCreateTransientShell,
   XT_HEADERS, "Shell.h",
   "Transient Popup Shell", "transientShellWidgetClass",
   "Popup shell under window manager control",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&vendorShellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateVendorShell", WcCreateVendorShell,
   XT_HEADERS, "Vendor.h",
   "Vendor Shell", "vendorShellWidgetClass",
   "Vendor shell",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&topLevelShellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateTopLevelShell", WcCreateTopLevelShell,
   XT_HEADERS, "Shell.h",
   "Top Level Shell", "topLevelShellWidgetClass",
   "Popup shell",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&applicationShellWidgetClass,
   NO_OBJECTS, ZERO,
   "XtCreateApplicationShell", WcCreateApplicationShell,
   XT_HEADERS, "Shell.h",
   "Application Shell", "applicationShellWidgetClass",
   "Popup shell for application (at least one of these in every application)",
   app_shell_res, XtNumber(app_shell_res)
},

/* Athena Widget set */

#ifdef XAW
 {
   (WidgetClass *)&asciiSinkObjectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "AsciiSink.h",
   "Athena Ascii Sink Object", "asciiSinkObjectClass",
   "Athena Ascii sink object for AsciiText Widget class",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&asciiSrcObjectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "AsciiSrc.h",
   "Athena Ascii Source Object", "asciiSrcObjectClass",
   "Athena Ascii source object for AsciiText Widget class - source from a string or disk file",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&asciiTextWidgetClass, 
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "AsciiText.h",
   "Athena AsciiText Edit", "asciiTextWidgetClass", 
   "Athena editable text widget with text taken from a string or disk file",
   xaw_text_res, XtNumber(xaw_text_res)
},{
   (WidgetClass *)&boxWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Box.h",
   "Athena Box", "boxWidgetClass",
   "Athena composite widget that lays out its children in rows and columns",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&clockWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Clock.h",
   "Athena Clock", "clockWidgetClass",
   "Athena clock widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&commandWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Command.h",
   "Athena Command Button", "commandWidgetClass",
   "Athena command button",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&dialogWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Dialog.h",
   "Athena Dialog Box", "dialogWidgetClass",
   "Athena dialog box",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&formWidgetClass,
   NO_OBJECTS, ZERO, NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR, XAW_HEADERS, "Form.h",
   "Athena Form", "formWidgetClass",
   "Athena composite widget that lays out its children via constraints",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&gripWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Grip.h",
   "Athena Grip", "gripWidgetClass",
   "Athena grip widget that provides a gripable area",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&labelWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Label.h",
   "Athena Label", "labelWidgetClass",
   "Athena text label",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&listWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "List.h",
   "Athena List", "listWidgetClass",
   "Athena list widget that provides a list of strings and allows then to be picked",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&logoWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Logo.h",
   "Athena X Logo", "logoWidgetClass",
   "Official X Logo",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&mailboxWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Mailbox.h",
   "Athena Mailbox", "mailboxWidgetClass",
   "Athena mailbox which changes colour when mail arrives",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&menuButtonWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "MenuButton.h",
   "Athena Menu Button", "menuButtonWidgetClass",
   "Athena Menu Button for use in the Simple Menu class",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&panedWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Paned.h",
   "Athena Paned Parent", "panedWidgetClass",
   "Athena composite widget which contrains its children to vertical or horizontal panels",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&scrollbarWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Scrollbar.h",
   "Athena Scroll Bar",
   "scrollbarWidgetClass", "Athena scrollbar or valuator",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&simpleWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Simple.h",
   "Athena Simple", "simpleWidgetClass",
   "Athena widget that provides a settable cursor",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&simpleMenuWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "SimpleMenu.h",
   "Athena Simple Menu", "simpleMenuWidgetClass",
   "Athena widget providing pop up menus",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&smeBSBObjectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "SmeBSB.h",
   "Athena Simple Menu B-S-B Entry Object", "smeBSBObjectClass",
   "Athena object for Bitmap-String-Bitmap entries in a simple menu",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&smeLineObjectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "SmeLine.h",
   "Athena Simple Menu Line Entry Object", "smeLineObjectClass",
   "Athena object for Line style entries in a simple menu",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&stripChartWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "StripChart.h",
   "Athena Strip Chart", "stripChartWidgetClass",
   "Athena strip chart graph",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&textSinkObjectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "TextSink.h",
   "Athena Text Sink Object", "textSinkObjectClass",
   "Athena text sink object:  this object is used by the text widget to provide a superclass to objects that display text in a text widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&textSrcObjectClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "TextSrc.h",
   "Athena Text Source Object", "textSrcObjectClass",
   "Athena text source object:  this object is used by the text widget to provide a superclass to objects that produce/store text for a text widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&textWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Text.h",
   "Athena Text Widget", "textWidgetClass",
   "Athena text widget: requires a source and sink",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&toggleWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Toggle.h",
   "Athena Toggle Button", "toggleWidgetClass",
   "Athena toggle button, radio grouping facility",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&viewportWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Viewport.h",
   "Athena Viewport", "viewportWidgetClass",
   "Athena scrollable viewing window",
   NO_EXCEPTIONS, ZERO
},
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
/* New Athena Widgets added at X11R5 */
{
   (WidgetClass *)&pannerWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Panner.h",
   "Athena Panner", "pannerWidgetClass",
   "Athena panner widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&portholeWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Porthole.h",
   "Athena Porthole", "portholeWidgetClass",
   "Athena porthole widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&repeaterWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Repeater.h",
   "Athena Repeater", "repeaterWidgetClass",
   "Athena repeater widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&treeWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XAW_HEADERS, "Tree.h",
   "Athena Tree", "treeWidgetClass",
   "Athena tree layout widget",
   NO_EXCEPTIONS, ZERO
},
#endif /* XAW R5 */
#endif /* XAW */

/* UKC Widget Set */

#ifdef UKC_WIDGETS
  {
   (WidgetClass *)&ukcArrowWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Arrow.h",
   "UKC Arrow", "ukcArrowWidgetClass",
   "UKC arrow widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcCommandWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Command.h",
   "UKC Command Button", "ukcCommandWidgetClass",
   "UKC command button",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcErrorWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Error.h",
   "UKC Error Handler", "ukcErrorWidgetClass",
   "Catches and displays Xlib and/or Toolkit warnings",
   NO_EXCEPTIONS, ZERO
},
#if 0
  {
   (WidgetClass *)&ukcGlueWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Glue.h",
   "UKC Glue", "ukcGlueWidgetClass",
   "UKC composite parent that uses TeX's idea of glue that shrinks and stretches",
   NO_EXCEPTIONS, ZERO
},
#endif /* 0 */
  {
   (WidgetClass *)&ukcLabelWidgetClass, 
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Label.h",
   "UKC Label", "ukcLabelWidgetClass",
   "UKC text and bitmap label",
   xukc_label_res, XtNumber(xukc_label_res)
},{
   (WidgetClass *)&ukcLayoutWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Layout.h",
   "UKC Layout", "ukcLayoutWidgetClass",
   "UKC composite widget which constrains its children using a connection list",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcMed3MenuWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Med3Menu.h",
   "UKC Med3 Menu", "ukcMed3MenuWidgetClass",
   "UKC Med3 Menu widget: creates heirarchical menus from a Med3 menu editor description file",
   xukc_med3_menu_res, XtNumber(xukc_med3_menu_res)
},{
   (WidgetClass *)&ukcMenuPaneWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "MenuPane.h",
   "UKC Menu Pane", "ukcMenuPaneWidgetClass",
   "UKC composite widget used for creating static panels",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcMouseWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Mouse.h",
   "UKC Mouse Hole", "ukcMouseWidgetClass",
   "UKC Mouse Hole",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcPassiveWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Passive.h",
   "UKC Passive", "ukcPassiveWidgetClass",
   "UKC passive widget that provides a drawing area for graphics",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcRowColWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "RowCol.h",
   "UKC Row Column", "ukcRowColWidgetClass",
   "UKC Row and Column constraint widget",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcScaledWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Scaled.h",
   "UKC Scaled Parent", "ukcScaledWidgetClass",
   "UKC constraint parent that scales its children and allows any geomertry",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcScrollbarWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Scrollbar.h",
   "UKC Scroll Bar", "ukcScrollbarWidgetClass",
   "UKC scrollbar/valuator that also allows marks to be placed next to the thumb bar",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&ukcScrListWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "ScrollList.h",
   "UKC Scrollable List", "ukcScrListWidgetClass",
   "UKC Scrollable list inside a viewport parent",
   xukc_scr_list_res, XtNumber(xukc_scr_list_res)
},{
   (WidgetClass *)&ukcToggleWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Toggle.h",
   "UKC Toggle Button", "ukcToggleWidgetClass",
   "UKC Toggle button",
   xukc_toggle_res, XtNumber(xukc_toggle_res)
},{
   (WidgetClass *)&ukcValueWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Value.h",
   "UKC Value Entry", "ukcValueWidgetClass",
   "UKC editable value widget: provides an editable number field along with increment and decrement arrows",
   xukc_value_res, XtNumber(xukc_value_res)
},{
   (WidgetClass *)&ukcViewportWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XUKC_HEADERS, "Viewport.h",
   "UKC Viewport", "ukcViewportWidgetClass",
   "UKC scrollable window which uses UKC scrollbars",
   xukc_viewport_res, XtNumber(xukc_viewport_res)
},
#endif /* UKC_WIDGETS */

/* HP Widget Set */

#ifdef HP_WIDGETS
  {
   (WidgetClass *)&XwarrowWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Arrow.h",
   "HP Arrow", "XwarrowWidgetClass",
   "HP arrow",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwbulletinBoardWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "BBoard.h",
   "HP Bulletin Board", "XwbulletinBoardWidgetClass", 
   "HP composite widget that allows children any geometry",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwcascadeWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Cascade.h",
   "HP Cascade", "XwcascadeWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwformWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Form.h",
   "HP Form", "XwformWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwimageEditWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "ImageEdit.h",
   "HP Editable Image", "XwimageEditWidgetClass",
   "HP editable raster/bitmap image",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwlistWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "List.h",
   "HP List", "XwlistWidgetClass",
   "HP scrolling list of strings",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwlistrowColWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "ListRC.h",
   "HP List Row/Column", "XwlistrowColWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwmenuButtonWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "MenuBtn.h",
   "HP Menu Button", "XwmenuButtonWidgetClass",
   "HP menu button",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwmenuSepWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "MenuSep.h",
   "HP Menu Separator", "XwmenuSepWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwrowColWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "RCManager.h",
   "HP Row/Column", "XwrowColWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwstaticRasterWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "SRaster.h",
   "HP Static Raster", "XwstaticRasterWidgetClass",
   "HP static raster/bitmap image",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwstaticTextWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "SText.h",
   "HP Static Text", "XwstaticTextWidgetClass",
   "HP static text label",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwscrolledWindowWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "SWindow.h",
   "HP Scrolled Window", "XwscrolledWindowWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwsashWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Sash.h",
   "HP Sash", "XwsashWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwscrollBarWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "ScrollBar.h",
   "HP Scroll Bar", "XwscrollBarWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwtextEditWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "TextEdit.h",
   "HP Text Edit", "XwtextEditWidgetClass",
   "HP editable text",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwtitleBarWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "TitleBar.h",
   "HP Title Bar", "XwtitleBarWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwtoggleWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Toggle.h",
   "HP Toggle Button", "XwtoggleWidgetClass",
   "HP Toggle button",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwvPanedWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "VPW.h",
   "HP Vertically Paned", "XwvPanedWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwvaluatorWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Valuator.h",
   "HP Valuator", "XwvaluatorWidgetClass",
   "HP scrollbar/valuator",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwworkSpaceWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "WorkSpace.h",
   "HP Work Space", "XwworkSpaceWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwpopupMgrWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "PopupMgr.h",
   "HP Popup Manager", "XwpopupMgrWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{
   (WidgetClass *)&XwpanelWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "Panel.h",
   "HP Panel", "XwpanelWidgetClass",
   "",
   NO_EXCEPTIONS, ZERO
},{ 
   (WidgetClass *)&XwpushButtonWidgetClass,
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   XHP_HEADERS, "PButton.h",
   "HP Push Button", "XwpushButtonWidgetClass",
   "HP push/command button",
   NO_EXCEPTIONS, ZERO
},
#endif /* HP_WIDGETS */

#ifdef MOTIF_WIDGETS
/* Motif Widget Set */
  {
   (WidgetClass *)&xmArrowButtonWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateArrowButton", XmCreateArrowButton,
   MOTIF_HEADERS, "ArrowB.h", 
   "Motif Arrow Button", "xmArrowButtonWidgetClass",
   "Motif Arrow Button",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmArrowButtonGadgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateArrowButtonGadget", XmCreateArrowButtonGadget,
   MOTIF_HEADERS, "ArrowBG.h", 
   "Motif Arrow Button GADGET", "xmArrowButtonGadgetClass",
   "Motif Arrow Button GADGET",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmBulletinBoardWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateBulletinBoard", XmCreateBulletinBoard,
   MOTIF_HEADERS, "BulletinB.h", 
   "Motif Bulletin Board", "xmBulletinBoardWidgetClass",
   "Motif Bulletin Board",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmCascadeButtonWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateCascadeButton", XmCreateCascadeButton,
   MOTIF_HEADERS, "CascadeB.h", 
   "Motif Cascade Button", "xmCascadeButtonWidgetClass",
   "Motif Cascade Button",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmCascadeButtonGadgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateCascadeButtonGadget", XmCreateCascadeButtonGadget,
   MOTIF_HEADERS, "CascadeBG.h", 
   "Motif Cascade Button GADGET", "xmCascadeButtonGadgetClass",
   "Motif Cascade Button GADGET",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmCommandWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateCommand", XmCreateCommand,
   MOTIF_HEADERS, "Command.h", 
   "Motif Command Button", "xmCommandWidgetClass",
   "Motif Command Button",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmDialogShellWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateDialogShell", XmCreateDialogShell,
   MOTIF_HEADERS, "DialogS.h", 
   "Motif Dialog Shell", "xmDialogShellWidgetClass",
   "Motif Dialog Shell",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmDrawingAreaWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateDrawingArea", XmCreateDrawingArea,
   MOTIF_HEADERS, "DrawingA.h", 
   "Motif Drawing Area", "xmDrawingAreaWidgetClass",
   "Motif Drawing Area",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmDrawnButtonWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateDrawnButton", XmCreateDrawnButton,
   MOTIF_HEADERS, "DrawnB.h", 
   "Motif Drawn Button", "xmDrawnButtonWidgetClass",
   "Motif Drawn Button",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmFileSelectionBoxWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateFileSelectionBox", XmCreateFileSelectionBox,
   MOTIF_HEADERS, "FileSB.h", 
   "Motif File Selection Box", "xmFileSelectionBoxWidgetClass",
   "Motif File Selection Box",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmFormWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateForm", XmCreateForm,
   MOTIF_HEADERS, "Form.h", 
   "Motif Form Constraint Parent", "xmFormWidgetClass",
   "Motif Form Constraint Parent",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmFrameWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateFrame", XmCreateFrame,
   MOTIF_HEADERS, "Form.h", 
   "Motif Frame", "xmFrameWidgetClass",
   "Motif Frame",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmLabelWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateLabel", XmCreateLabel,
   MOTIF_HEADERS, "Label.h", 
   "Motif Label", "xmLabelWidgetClass",
   "Motif Label",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmLabelGadgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateLabelGadget", XmCreateLabelGadget,
   MOTIF_HEADERS, "LabelG.h", 
   "Motif Label GADGET", "xmLabelGadgetClass",
   "Motif Label GADGET",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmListWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateList", XmCreateList,
   MOTIF_HEADERS, "List.h", 
   "Motif List", "xmListWidgetClass",
   "Motif List",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmMainWindowWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateMainWindow", XmCreateMainWindow,
   MOTIF_HEADERS, "MainW.h", 
   "Motif Main Window", "xmMainWindowWidgetClass",
   "Motif Main Window",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmMenuShellWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateMenuShell", XmCreateMenuShell,
   MOTIF_HEADERS, "MenuShell.h", 
   "Motif Menu Shell", "xmMenuShellWidgetClass",
   "Motif Menu Shell",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmMessageBoxWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateMessageBox", XmCreateMessageBox,
   MOTIF_HEADERS, "MessageB.h", 
   "Motif Message Box", "xmMessageBoxWidgetClass",
   "Motif Message Box",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmPanedWindowWidgetClass, 
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   MOTIF_HEADERS, "PanedW.h", 
   "Motif Paned Window", "xmPanedWindowWidgetClass",
   "Motif Paned Window",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmPushButtonWidgetClass, 
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   MOTIF_HEADERS, "PushB.h", 
   "Motif Push Button", "xmPushButtonWidgetClass",
   "Motif Push Button",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmPushButtonGadgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreatePushButtonGadget", XmCreatePushButtonGadget,
   MOTIF_HEADERS, "PushBG.h", 
   "Motif Push Button GADGET", "xmPushButtonGadgetClass",
   "Motif Push Button GADGET",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmRowColumnWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateRowColumn", XmCreateRowColumn,
   MOTIF_HEADERS, "RowColumn.h", 
   "Motif Row Column", "xmRowColumnWidgetClass",
   "Motif Row Column Constraint Parent",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmScaleWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateScale", XmCreateScale,
   MOTIF_HEADERS, "Scale.h", 
   "Motif Scale", "xmScaleWidgetClass",
   "Motif Scale",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmScrollBarWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateScrollBar", XmCreateScrollBar,
   MOTIF_HEADERS, "ScrollBar.h", 
   "Motif Scroll Bar", "xmScrollBarWidgetClass",
   "Motif Scroll Bar",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmScrolledWindowWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateScrolledWindow", XmCreateScrolledWindow,
   MOTIF_HEADERS, "ScrolledW.h", 
   "Motif Scrolled Window", "xmScrolledWindowWidgetClass",
   "Motif Scrolled Window Parent",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmSelectionBoxWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateSelectionBox", XmCreateSelectionBox,
   MOTIF_HEADERS, "SelectioB.h", 
   "Motif Selection Box", "xmSelectionBoxWidgetClass",
   "Motif Selection Box",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmSeparatorGadgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateSeparatorGadget", XmCreateSeparatorGadget,
   MOTIF_HEADERS, "SeparatoG.h", 
   "Motif Separator GADGET", "xmSeparatorGadgetClass",
   "Motif Separator GADGET",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmSeparatorWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateSeparator", XmCreateSeparator,
   MOTIF_HEADERS, "Separator.h", 
   "Motif Separator", "xmSeparatorWidgetClass",
   "Motif Separator",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmTextWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateText", XmCreateText,
   MOTIF_HEADERS, "Text.h", 
   "Motif Text Widget", "xmTextWidgetClass",
   "Motif Text Widget",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmTextFieldWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateTextField", XmCreateTextField,
   MOTIF_HEADERS, "TextF.h", 
   "Motif Text Field Widget", "xmTextFieldWidgetClass",
   "Motif Text Field Widget",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmToggleButtonWidgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateToggleButton", XmCreateToggleButton,
   MOTIF_HEADERS, "ToggleB.h", 
   "Motif Toggle Button", "xmToggleButtonWidgetClass",
   "Motif Toggle Button",
   NO_EXCEPTIONS, ZERO 
},{
   (WidgetClass *)&xmToggleButtonGadgetClass, 
   NO_OBJECTS, ZERO,
   "XmCreateToggleButtonGadget", XmCreateToggleButtonGadget,
   MOTIF_HEADERS, "ToggleBG.h", 
   "Motif Toggle Button GADGET", "xmToggleButtonGadgetClass",
   "Motif Toggle Button GADGET",
   NO_EXCEPTIONS, ZERO 
},
#endif /* MOTIF_WIDGETS */

#ifdef MISC_WIDGETS
/* Miscellaneous Widgets */
  {
   (WidgetClass *)&tableWidgetClass, 
   NO_OBJECTS, ZERO,
   NO_CONSTRUCTOR_STRING, NO_CONSTRUCTOR,
   MISC_HEADERS, "Table.h", 
   "Table Layout", "tableWidgetClass",
   "A composite widget which lays out its children in a table style",
   NO_EXCEPTIONS, ZERO 
},
#endif /* MISC_WIDGETS */
};

Cardinal num_on_widget_class_list = XtNumber(widget_classes);
