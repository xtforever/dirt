/* SCCSID: @(#)Med3Menu.h	1.1 (UKC) 5/10/92 */

#ifndef _XukcMed3MenuH_h
#define _XukcMed3MenuH_h

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

/**********************************************************************
 *                          UKCMed3MenuWidget                         *
 *                                                                    *
 *    Subclass of coreWidgetClass : creates menus from Med3 files     *
 **********************************************************************/

/* superclasses resources */

#include <X11/Xukc/MenuPane.h>

/* resources for widget classes used by Med3Menu which are accessible via
 * XtSetButtonValues() */

#include <X11/Xukc/Command.h>

/************************  Default Widget Resources  *****************

static XtResource resources[] = {
Name                 Class              RepType          Default Value     
----                 -----              -------          -------------     
XtNmenuFile          MenuFile           String           NULL              
XtNmenuStruct        MenuStruct         MenuStruct       NULL              
XtNmenuWidget        ReadOnly           Widget           NULL              
XtNpopupBitmap       PopupBitmap        Bitmap           None              
XtNnoBorder          NoBorder           Boolean          FALSE             
XtNhighlightThickness Thickness         Dimension        0                 
XtNshrinkMenu        ShrinkMenu         Boolean          FALSE             
*/

/* resources */

#ifndef XtNmenuFile
#define XtNmenuFile "menuFile"
#endif /* XtNmenuFile */

#ifndef XtNmenuWidget
#define XtNmenuWidget "menuWidget"
#endif /* XtNmenuWidget */

#ifndef XtNmenuStruct
#define XtNmenuStruct "menuStruct"
#endif /* XtNmenuStruct */

#ifndef XtNpopupBitmap
#define XtNpopupBitmap "popupBitmap"
#endif /* XtNpopupBitmap */

#ifndef XtNnoBorder
#define XtNnoBorder "noBorder"
#endif /* XtNnoBorder */

#ifndef XtNshrinkMenu
#define XtNshrinkMenu "shrinkMenu"
#endif /* XtNshrinkMenu */

#ifndef XtNuseFile
#define XtNuseFile "useFile"
#endif /* XtNuseFile */


#ifndef XtCMenuFile
#define XtCMenuFile "MenuFile"
#endif /* XtCMenuFile */

#ifndef XtCMenuStruct
#define XtCMenuStruct "MenuStruct"
#endif /* XtCMenuStruct */

#ifndef XtCPopupBitmap
#define XtCPopupBitmap "PopupBitmap"
#endif /* XtCPopupBitmap */

#ifndef XtCNoBorder
#define XtCNoBorder "NoBorder"
#endif /* XtCNoBorder */

#ifndef XtCShrinkMenu
#define XtCShrinkMenu "ShrinkMenu"
#endif /* XtCShrinkMenu */

#ifndef XtCUseFile
#define XtCUseFile "UseFile"
#endif /* XtCUseFile */


#ifndef XtRMenuStruct
#define XtRMenuStruct "MenuStruct"
#endif /* XtRMenuStruct */


/* Class record pointer */

extern WidgetClass	ukcMed3MenuWidgetClass;

/* Widget Type definition */

typedef	struct _ukcMed3MenuRec	*UKCMed3MenuWidget;
typedef struct _ukcMed3MenuClassRec	*UKCMed3MenuWidgetClass;

#define BINDING_VALUE(bind_character, level) (short)(bind_character + (level * 256))

/* functional interface as described in the UKCMed3Menu manual page */
extern Widget XukcFindButton();

extern Boolean XukcAddAcceleratorToMenu();

extern void XukcInstallMenuAccelerators();

extern Boolean XukcBindButton();

extern Boolean XukcUnbindButton();

extern Boolean XukcBindToggle();

extern Boolean XukcUnbindToggle();

extern Boolean XukcMenuSetToggleState(/* menu, button, state, notify */);
/* UKCMed3MenuWidget menu;
   short button;
   Boolean state;
   Boolean notify;	* call the registered callback? *
*/
extern Boolean XukcBindForceDown();

extern Boolean XukcUnbindForceDown();

extern Boolean XukcSetButtonValues(/* menu, button, args, num_args */);

extern Boolean XukcChangeButtonStatus();

#define XukcTurnOffButton(w,b) XukcChangeButtonStatus(w, b, FALSE)
#define XukcTurnOnButton(w,b) XukcChangeButtonStatus(w, b, TRUE)

extern Boolean XukcReplaceMenu();

typedef struct _add_menu {
	String button_name;
	short button_value;
} XukcReplaceMenuStruct;

/*  Structure used in a menu tree structure
 */
typedef struct mnode {
	unsigned short me_flags;
	short me_pos;	/*  The position at which the field is divided */
	char *me_cap;	/*  Caption to be displayed in the field */
	short me_rv; 		/* result to be returned */
	short me_xstart;	/* The window area of this node */
	short me_ystart;
	short me_xend;
	short me_yend;
	short me_xcurs;		/* cursor coordinates for cursor */
	short me_ycurs;		/* relative popup menus */
	short me_colour;
	long me_save;		/* For saving popup backgrounds */
	struct mnode * me_topleft; /* pointer to top or left sub-menu */
	struct mnode * me_botrite; /* pointer to b. or r. sub_menu */
	struct mnode * me_parent;  /* pointer to parent */
	XtPointer *padding;	   /* some padding */
} MENU;

#endif	_XukcMed3MenuH_h
