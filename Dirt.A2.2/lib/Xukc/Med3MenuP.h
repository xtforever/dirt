/* SCCSID: @(#)Med3MenuP.h	1.1 (UKC) 5/10/92 */

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

/***********************************************************
 *      Med3Menu Widget : Subclass of coreWidgetClass      *
 *        (c) 1988 Richard Hesketh / rlh2@ukc.ac.uk        *
 *     Computing Lab., University of Kent at Canterbury    *
 ***********************************************************/

#ifndef	_XukcMed3MenuPrivate_h
#define _XukcMed3MenuPrivate_h

/* my superclasses definitions and resources */
#include <X11/Xukc/MenuPaneP.h>

#include <X11/Xukc/Med3Menu.h>
#include <X11/Xukc/CommandP.h>

typedef struct _ukcMed3MenuClassPart {
	short	mumble; /* no new fields */
} UKCMed3MenuClassPart;

/* Full Class Record declaration */

typedef struct	_ukcMed3MenuClassRec {
	CoreClassPart		core_class;
	CompositeClassPart      composite_class;
	ConstraintClassPart     constraint_class;
	UKCMenuPaneClassPart    menuPane_class;
	UKCMed3MenuClassPart	med3Menu_class;
} UKCMed3MenuClassRec;

/* Class record variable */

extern UKCMed3MenuClassRec	ukcMed3MenuClassRec;

struct _ukcMed3MenuPart {
	/* public resources */
	String	menu_file;	/* name of menu description file */
	MENU *	menu_struct;	/* internal menu structure */
	Pixmap	popup_pixmap;	/* bitmap showing a popup menu exists */
	Boolean	no_border;
	Boolean shrink_menu;
	Dimension highlight;	/* thickness of highlight border (if any) */
	Boolean use_file;	/* if true then use menu file before struct */

	/* private resources */
	Buttons	button_list;	/* list of buttons and binding characters */
	PopupData shell_data;	/* popup shell data */
/* ||| public resource for backwards-compatibility only ||| */
	Widget	menu_widget;	/* created root of the menu widget */
};
	
typedef struct _ukcMed3MenuPart UKCMed3MenuPart;

typedef struct _ukcMed3MenuRec {
	CorePart	core;
	CompositePart   composite;
	ConstraintPart  constraint;
	UKCMenuPanePart menuPane;
	UKCMed3MenuPart	med3Menu;
} UKCMed3MenuRec;

#endif	_XukcMed3MenuPrivate_h
