/* BUGS: need to work a good method for persistent popups that are children
 * of dynamic popups.  Grabs get confused easily.
 */

#ifndef lint
/* from:  $Xukc: Med3Menu.c,v 1.13 91/12/17 10:01:38 rlh2 Rel */
static char sccsid[] = "@(#)Med3Menu.c	1.2 (UKC) 5/10/92";
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

/**********************************************************************
 *								      *
 *         Menu Meta-class Widget - subclass of coreWidgetClass       *
 *								      *
 **********************************************************************/


/* TOOLKIT */

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <X11/Xukc/memory.h>

#define MAXNUM 32767

typedef struct _popup_data *PopupData; /* opaque type */
typedef struct _buttons *Buttons; /* opaque type */

#include "Med3MenuP.h"

extern Widget XukcGetShellManagedChild();
extern Dimension XukcGetWidth();
extern Dimension XukcGetHeight();

/* data structure for popup shells, used by action routines */

typedef struct _popup_data {
	struct _popup_data *parents_data; /* this parent's parent data */
	struct _popup_data *last_up_data; /* sub_menu data record pointer */
	Widget	last_up;		/* sub_menu currently up */
	Widget	last_setpopup;		/* button who popped up sub_menu */
	Widget	last_chosen;		/* button last chosen in sub_menu */
	Widget	shell;			/* this parent's shell */
	Position shell_x;		/* this parent's original position */
	Position shell_y;
	Boolean	inhibit_popdown;	/* <Btn1Up> does not popdown menu? */
	Boolean torn_off;		/* this is a torn off menu */
} PopupDRec;


/* linked list of buttons used for binding application procedures */

struct toggledata {
	String on;
	String off;
	Boolean state;
	Boolean *address;
	XtCallbackProc proc;
	XtPointer data;
};

typedef struct _buttons {
	UKCMed3MenuWidget menu_widget;	/* the menu widget for this button */
	int	button_type;		/* reserved for future use */
	short	button_value;		/* button return value */
	Widget	button_widget;		/* widget that implements the button */
	PopupData shell_data;		/* this buttons menu shell */
	struct toggledata *toggle_data;	/* toggle data if a toggle button */
	struct _buttons *next;
	struct _buttons *prev;
} ButtonRec;

/* flag testing macros for Med3 menu tree */
#define TF(flag) (xtmenu->me_flags & flag)
#define TFB(flag) ((Boolean)(TF(flag) ? TRUE : FALSE))

/* parent types */
#define PARENT_MASK 3
#define NO_PARENT 1
#define STATIC_POPUP 2
#define DYNAMIC_POPUP 3

/* button types */
#define BUTTON_MASK (NORMAL_BUTTON|TOGGLE_BUTTON|FORCE_DOWN_BUTTON)
#define BUTTON_TYPE_NOT_SET 0
#define NORMAL_BUTTON 4
#define TOGGLE_BUTTON 8
#define FORCE_DOWN_BUTTON 16

static PopupData create_popup_shell_data();

/* global variables */
static int current_num = 0;
static Widget startwidget = NULL;
static PopupData startdata = NULL;
static Buttons Mbuttons = NULL;
static Pixmap popup_bm = None;

/* methods */
static void Class_Initialize();
static void Initialize();
static Boolean SetValues();
static XtGeometryResult Query_Geometry();

/* forward definition of actions */
static void set_popup();
static void popdown();
static void last_set();
static void tear_off();
static void move_tear_off();
static void drop_tear_off();
#ifdef DEBUG
	static void debug_stop();
#endif

/* other forward definitions */
static void CreateMenu();
static void CreateMenuFromFile();

/* Translation Tables */

/* accelerator mask */
static String acceleratorTranslations = "%s: set() notify() unset()";

/* Static Menus ... */

/* for buttons of static menus which pop up fixed position menus */
static String fp_sButtonTranslations =
	"<Btn1Down>:	unhighlight() set() notify() med3SetPopup(%d %d) XtMenuPopup(%s) \n\
	 <Btn1Up>:	reset() \n\
	 <EnterWindow>: highlight() \n\
	 <LeaveWindow>: unhighlight() \n\
	 <Btn2Down>: label_edit(on)\n\
	 <Btn2Motion>: position_bar()\n\
	 :<Key>Delete: delete_char()\n\
	 :<Key>BackSpace: delete_char()\n\
	 :<Key>Return: label_edit(off) label_update()\n\
	 :<Key>Linefeed: label_edit(off) label_update()\n\
	 :<Key>Escape: undo_edit()\n\
	 <Key>: insert_char()\n\
";

/* for buttons of static menus which pop up cursor relative menus */
static String cr_sButtonTranslations =
	"<Btn1Down>:	unhighlight() set() notify() med3SetPopup(%d %d TRUE) XtMenuPopup(%s) \n\
	 <Btn1Up>:	reset() \n\
	 <EnterWindow>: highlight() \n\
	 <LeaveWindow>: unhighlight()\n\
	 <Btn2Down>: label_edit(on)\n\
	 <Btn2Motion>: position_bar()\n\
	 :<Key>Delete: delete_char()\n\
	 :<Key>BackSpace: delete_char()\n\
	 :<Key>Return: label_edit(off) label_update()\n\
	 :<Key>Linefeed: label_edit(off) label_update()\n\
	 :<Key>Escape: undo_edit()\n\
	 <Key>: insert_char()\n\
";

/* Dynamic Menus ... */

/* for buttons of dynamic menus which do not pop up menus */
static String dButtonTranslations =
#if 0
	"~Shift<EnterWindow>: med3Popdown(%d) set() med3LastSet(%d) \n\
	 Shift Button1<LeaveWindow>: med3TearOff(%d) \n\
	 Shift<Btn1Up>: med3DropTearOff(%d) \n\
	 Shift<Btn1Motion>: med3MoveTearOff(%d) \n\
"
#endif
	"<EnterWindow>: med3Popdown(%d) set() med3LastSet(%d) \n\
	 <LeaveWindow>:	unset() \n\
	 <Btn1Up>:	notify() \n\
	 <Btn2Down>: label_edit(on)\n\
	 <Btn2Motion>: position_bar()\n\
	 :<Key>Delete: delete_char()\n\
	 :<Key>BackSpace: delete_char()\n\
	 :<Key>Return: label_edit(off) label_update()\n\
	 :<Key>Linefeed: label_edit(off) label_update()\n\
	 :<Key>Escape: undo_edit()\n\
	 <Key>: insert_char()\n\
";

/* for buttons of dynamic menus which popup fixed position menus */
static String fp_dButtonTranslations =
#if 0
	"~Shift Button1<EnterWindow>: set() notify() med3LastSet(%d) med3SetPopup(%d %d) XtMenuPopup(%s) \n\
	 Shift Button1<LeaveWindow>: med3TearOff(%d) \n\
	 Shift<Btn1Up>: med3DropTearOff(%d) \n\
	 Shift<Btn1Motion>: med3MoveTearOff(%d) \n\
"
#endif
	"Button1<EnterWindow>: set() notify() med3LastSet(%d) med3SetPopup(%d %d) XtMenuPopup(%s) \n\
	 <LeaveWindow>: unset() \n\
	 <Btn2Down>: label_edit(on)\n\
	 <Btn2Motion>: position_bar()\n\
	 :<Key>Delete: delete_char()\n\
	 :<Key>BackSpace: delete_char()\n\
	 :<Key>Return: label_edit(off) label_update()\n\
	 :<Key>Linefeed: label_edit(off) label_update()\n\
	 :<Key>Escape: undo_edit()\n\
	 <Key>: insert_char()\n\
";

/* for buttons of dynamic menus which pop up cursor relative menus */
static String cr_dButtonTranslations =
#if 0
	"~Shift Button1<EnterWindow>: set() notify() med3LastSet(%d) med3SetPopup(%d %d TRUE) XtMenuPopup(%s) \n\
	 Shift Button1<LeaveWindow>: med3TearOff(%d) \n\
	 Shift<Btn1Up>: med3DropTearOff(%d) \n\
	 Shift<Btn1Motion>: med3MoveTearOff(%d) \n\
"
#endif
	"Button1<EnterWindow>: set() notify() med3LastSet(%d) med3SetPopup(%d %d TRUE) XtMenuPopup(%s) \n\
	 <LeaveWindow>: unset() \n\
	 <Btn2Down>: label_edit(on)\n\
	 <Btn2Motion>: position_bar()\n\
	 :<Key>Delete: delete_char()\n\
	 :<Key>BackSpace: delete_char()\n\
	 :<Key>Return: label_edit(off) label_update()\n\
	 :<Key>Linefeed: label_edit(off) label_update()\n\
	 :<Key>Escape: undo_edit()\n\
	 <Key>: insert_char()\n\
";

static String shellTranslations =
	"Shift<Btn1Up>:	med3DropTearOff(%d)\n\
	 Shift Button1<LeaveWindow>: med3TearOff(%d) \n\
	 Shift<Btn1Motion>: med3MoveTearOff(%d) \n\
	 <Btn1Up>:	med3Popdown(%d)\n\
";

/* Actions Table */

static XtActionsRec menuActions[] = {
	{ "med3SetPopup",	set_popup },
	{ "med3Popdown",	popdown },
	{ "med3LastSet",	last_set },
	{ "med3TearOff",	tear_off },
	{ "med3MoveTearOff",	move_tear_off },
	{ "med3DropTearOff",	drop_tear_off },
#ifdef DEBUG
	{ "med3Debug",	debug_stop },
#endif DEBUG
};


static XtResource resources[] = {
#define offset(field) XtOffset(UKCMed3MenuWidget, med3Menu.field)
	{ XtNmenuFile, XtCMenuFile, XtRString, sizeof(String),
	  offset(menu_file), XtRString, (XtPointer)NULL },
	{ XtNmenuStruct, XtCMenuStruct, XtRMenuStruct, sizeof(MENU *),
	  offset(menu_struct), XtRString, (XtPointer)NULL },
	{ XtNuseFile, XtCUseFile, XtRBoolean, sizeof(Boolean),
	  offset(use_file), XtRImmediate, (XtPointer)FALSE },
	{ XtNpopupBitmap, XtCPopupBitmap, XtRBitmap, sizeof(Pixmap),
	  offset(popup_pixmap), XtRImmediate, (XtPointer)None },
	{ XtNnoBorder, XtCNoBorder, XtRBoolean, sizeof(Boolean),
	  offset(no_border), XtRImmediate, (XtPointer)FALSE },
	{ XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
	  offset(highlight), XtRImmediate, (XtPointer)0 },

/* this currently does not work due to MenuPane */
	{ XtNshrinkMenu, XtCShrinkMenu, XtRBoolean, sizeof(Boolean),
	  offset(shrink_menu), XtRImmediate, (XtPointer)FALSE },

/* ||| for backwards-compatibility only ||| */
	{ XtNmenuWidget, XtCReadOnly, XtRWidget, sizeof(Widget),
	  offset(menu_widget), XtRString, (XtPointer)NULL },

/* use these here to cause suitable defaults to be set */
	{ XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	  XtOffset(UKCMed3MenuWidget, core.width), XtRImmediate, (XtPointer)0},
	{ XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	  XtOffset(UKCMed3MenuWidget, core.height), XtRImmediate, (XtPointer)0},
};

/* Full Class Record Constant */

UKCMed3MenuClassRec ukcMed3MenuClassRec = {
  { /* core_class fields  */	
    /* superclass	  */	(WidgetClass) &ukcMenuPaneClassRec,
    /* class_name	  */	"UKCMed3Menu",
    /* widget_size	  */	sizeof(UKCMed3MenuRec),
    /* class_initialize   */    Class_Initialize,
    /* class_part_initialize */ NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,
    /* realize		  */	XtInheritRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	NULL,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	XtInheritResize,
    /* expose		  */	XtInheritExpose,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	XtInheritAcceptFocus,
    /* version		  */	XtVersion,
    /* callback_private	  */	NULL,
    /* tm_table		  */	NULL,
    /* query_geometry	  */	Query_Geometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension */		NULL
  },{
  /* composite_class fields */
  /* geometry_handler   */	XtInheritGeometryManager,
  /* change_managed     */	XtInheritChangeManaged,
  /* insert_child       */ 	XtInheritInsertChild,
  /* delete_child       */ 	XtInheritDeleteChild,
  /* extension          */ 	NULL
  },{
  /* constraint_class fields */
  /* resources          */    	NULL,
  /* num_resources      */    	0,
  /* constraint_size    */	sizeof(MPConstraintRecord),
  /* initialize         */	NULL,
  /* destroy            */	NULL,
  /* set_values         */	NULL,
  /* extension          */	NULL
  },{ /* UKCMenuPane class fields */
    /* None */			NULL
  },{ /* UKCMed3Menu class fields */
    /* None */			NULL
  }
};

WidgetClass ukcMed3MenuWidgetClass = (WidgetClass)&ukcMed3MenuClassRec;

/*---^^^^^^^-----------End of X Toolkit Specific Definitions-----------*/

/*-----------------------------vvvvvvvvvvv---------------------------------*/

/* This bit ... Copyright 1988 UKC Software Tools Centre.
 *
 * This code is copyrighted with the same conditions given in the
 * copyright at the beginning of this file.
 *
 * Hacked by rlh2 from jdb/mtr code for med3 library July '88
 */

#include <stdio.h>
#include <ctype.h>

#define MAXMEN 40	/* the maximum number of open menus */

/*  menerr error values
 */
#define MENOK	0
#define MTABFL  1       /* already MAXMEN open menus */
#define MNOTMEN 2       /* menu descriptor is not an open menu */
#define MBADMD  3       /* menu descriptor out of range */
#define MBADFIL 4       /* can't read menu from menu file */
#define MNOFIL  5       /* can't open menu file */
#define MBADWD  6       /* bad window file descriptor */
#define MNOTDIS 7       /* the menu is not currently displayed */
#define MBADARG 8       /* bad function arguments */
#define MAERROR 9       /* message area error */

#define MAXNODES 1000   /* Max number of nodes for compiling menus */

#define ANY_CHAR (-2)
#define LASTNODE (-2)


/* Copyright 1988 UKC Software Tools Centre.
 */

/* flags for me_flags
 */
#define ME_FONT		0x3	/* mask for font */
#define ME_VER		0x4	/* node is divided vertically */
#define ME_HOR		0x8	/* node divided horizontally */
#define ME_DIV		0xc	/* node is divided */
#define ME_NSEL		0x10	/* This node can't be selected */
#define ME_POPUP	0x20	/* use in pop-up mode */
#define ME_FREC		0x40	/* me_cap can be free()'d */
#define ME_FREN		0x80	/* the node itself can be free()'d */
#define ME_OPEN		0x100	/* the node is open */
#define ME_ISUB		0x200	/* has an indivisible submenu */
#define ME_FREE		0x400	/* free standing node */
#define ME_BGLINE	0x800	/* draw lines in background colour */
#define ME_NOSCALE	0x1000	/* don't scale submenu */
#define ME_CREL		0x2000	/* cursor relative submenu */
#define ME_REDRAW	0x4000  /* flag to force menu redrawing */
#define ME_LEFT		0x8000	/* left justify caption rather than centre it */
/* added option */
#define ME_RESIZE	0x10000 /* resize the popup */


/* These structures aren't actually used .. we keep them here to be
 * compatible with the description files generated by the med3 editor */

/*  define these to make the source look more like jdb's version
 */

typedef struct fontst FONT;

/*  structure defining an opened menu
 */
struct omenst {
	MENU * om_root;		/* root node of the menu tree */
	MENU * om_last;		/* last node displayed (NULL if not displayed) */
	short om_md;		/* menu descriptor of the menu */
	short om_fback[3];	/* feedback colours used */
	FONT *om_font[4];	/* fonts for captions */
};


struct omenst _menu_[MAXMEN];
int menerr;

static MENU *Nodevec[MAXNODES]; /* Table of node addresses */

#define MAXMERR 128
static struct merrst {
	char mr_message[MAXMERR];
	int  mr_line;
} merr;      /* Error information */


char *malloc();

/*--------------------End of Med3 Specific definitions-------------*/

/*---vvvvvvv-------------Med3 Specific Routines------------------------*/

static MENU *getmenu();
static MENU *getsrcmenu();
static fix_parent_links();
static skip_header();
static int getmnode();
static skipnull();
static getstring();
static backslash();
static do_struct_header();
static skipwhite();
static getword();
static checkword();
static int getnodenum();
static getnum();
static void meprintf();
static mygetc();
static void myungetc();
static void freemen();

static
rplink(mp,parent)
MENU *mp, *parent;
{
	if (mp == NULL)
		return;
	rplink(mp->me_topleft,mp);
	rplink(mp->me_botrite,mp);
	mp->me_parent = parent;
	mp->me_flags &= ~(ME_FREC | ME_FREN);
}


/*  Insert the menu in the _menu_ array.  A menu identifier is returned which
 *  is used to identify the menu in all subsequent function calls.
 *  -1 is returned if unsuccessful.
 */
static int
Minsert(menu)
MENU * menu;
{
	register i;

	for (i = 0; i < MAXMEN; i++)
		if (_menu_[i].om_root == NULL)
			break;
	if (i == MAXMEN) {
		menerr = MTABFL;
		return(-1);
	}

	rplink(menu,(MENU *)NULL);

	_menu_[i].om_root = menu;
	_menu_[i].om_last = NULL;
	_menu_[i].om_md = i;
	_menu_[i].om_font[0] =
	_menu_[i].om_font[1] =
	_menu_[i].om_font[2] = 
	_menu_[i].om_font[3] =  NULL;
	_menu_[i].om_fback[0] = NULL;
	_menu_[i].om_fback[1] = NULL;
	_menu_[i].om_fback[2] = NULL;
	menerr = MENOK;
	return(i);
}


/*  Read in the menu from file 'name'.  A menu identifier is returned which
 *  is used to identify the menu in all subsequent function calls.
 *  -1 is returned if unsuccessful.
 */
static int
Mopen(name)
char * name;
{
	register i;

	for (i = 0; i < MAXMEN; i++)
		if (_menu_[i].om_root == NULL)
			break;
	if (i == MAXMEN) {
		menerr = MTABFL;
		return(-1);
	}

	if ((_menu_[i].om_root = getmenu(name, (char *)NULL)) == NULL)
		return(-1);

	_menu_[i].om_md = i;
	_menu_[i].om_last = NULL;
	_menu_[i].om_font[0] =
	_menu_[i].om_font[1] =
	_menu_[i].om_font[2] =
	_menu_[i].om_font[3] =  NULL;
	_menu_[i].om_fback[0] = NULL;
	_menu_[i].om_fback[1] = NULL;
	_menu_[i].om_fback[2] = NULL;
	menerr = MENOK;
	return(i);
}


static int
fake_a_menu(new_pane, num_buttons, vert, size, other_dimension)
XukcReplaceMenuStruct **new_pane;
Cardinal num_buttons;
Boolean vert;
Dimension size, other_dimension;
{
	register i, j;
	MENU *mnode, *mhead, *mlast;
	int current_size;

	for (i = 0; i < MAXMEN; i++)
		if (_menu_[i].om_root == NULL)
			break;
	if (i == MAXMEN) {
		menerr = MTABFL;
		return(-1);
	}

	if ((mnode = mhead = (MENU *)malloc(sizeof(MENU))) == NULL)
		abort();

	mhead->me_flags = (vert ? ME_VER : ME_HOR);
	mhead->me_flags |= ME_RESIZE;
	mhead->me_pos = 0;
	mhead->me_cap = NULL;
	mhead->me_rv = NULL;
	if (vert) {
		mhead->me_ystart = 0;
		mhead->me_yend = other_dimension * num_buttons;
		mhead->me_xstart = 0;
		mhead->me_xend = size;
	} else {
		mhead->me_xstart = 0;
		mhead->me_xend = other_dimension * num_buttons;
		mhead->me_ystart = 0;
		mhead->me_yend = size;
	}
	mhead->me_xcurs = 0; /* cursor coordinates for cursor */
	mhead->me_ycurs = 0; /* relative popup menus */
	mhead->me_colour = NULL;
	mhead->me_save = FALSE;	/* For saving popup backgrounds */
	mhead->me_topleft = NULL; /* pointer to top or left sub-menu */
	mhead->me_botrite = NULL;  /* pointer to b. or r. sub_menu */
	mnode->me_parent = NULL;  /* pointer to parent */

	mlast = mhead;
	for (current_size = j = 0; j < num_buttons; j++) { 
		if ((mnode = (MENU *)malloc(sizeof(MENU))) == NULL)
			abort();

		mlast->me_botrite = mnode;
		mnode->me_flags = ME_FREC;
			/*  The position at which the field is divided */
		mnode->me_pos = 0;
				/*  Caption to be displayed in the field */
		mnode->me_cap = XtNewString(new_pane[j]->button_name);
				/* result to be returned */
		mnode->me_rv = new_pane[j]->button_value; 
		/* The window area of this node */
		if (vert) {
			mnode->me_ystart = current_size;
			current_size += other_dimension;
			mnode->me_yend = current_size;
			mnode->me_xstart = 0;
			mnode->me_xend = size;
		} else {
			mnode->me_xstart = current_size;
			current_size += other_dimension;
			mnode->me_xend = current_size;
			mnode->me_ystart = 0;
			mnode->me_yend = size;
		}
		mnode->me_xcurs = 0; /* cursor coordinates for cursor */
		mnode->me_ycurs = 0; /* relative popup menus */
		mnode->me_colour = NULL;
		mnode->me_save = FALSE;	/* For saving popup backgrounds */
		mnode->me_topleft = NULL; /* pointer to top or left sub-menu */
		mnode->me_botrite = NULL;  /* pointer to b. or r. sub_menu */
		mnode->me_parent = mlast;  /* pointer to parent */

		mlast = mnode;

	}

	mlast->me_botrite = NULL;
	_menu_[i].om_root = mhead;
	_menu_[i].om_md = i;
	_menu_[i].om_last = NULL;
	_menu_[i].om_font[0] =
	_menu_[i].om_font[1] =
	_menu_[i].om_font[2] =
	_menu_[i].om_font[3] =  NULL;
	_menu_[i].om_fback[0] = NULL;
	_menu_[i].om_fback[1] = NULL;
	_menu_[i].om_fback[2] = NULL;
	menerr = MENOK;
	return(i);
}


/*  Read a menu from file fname and return a pointer to it. In case of failure
 *  NULL is returned. If the menu is in the old format it is automatically
 *  converted. If the menu is in the new format and mname is non NULL then
 *  it is used as a destination for the menu variable name.
 */
static MENU *
getmenu(fname,mname)
char *fname, *mname;
{
	int fd;
	MENU *menu;
	MENU *getsrcmenu();
	long lseek();

	if ((fd = open(fname,0)) < 0) {
		menerr = MNOFIL;
		return(NULL);
	}
	merr.mr_line = 1;
	menu = getsrcmenu(fd,mname);
	close(fd);
	return(menu);
}

/*  The following routines, which compile a C source code menu, are due
 *  to Mark Russell.
 */

/*  Read in a source code menu from the file open with file descriptor fd.
 *  If mname is not NULL it will be used as a destination for the menu
 *  name.
 */
static MENU *
getsrcmenu(fd,mname)
int fd;
char *mname;
{
	FILE *fp;
	MENU *mnode;
	int nodenum;
	
	if ((fp = fdopen(fd,"r")) == NULL) {
		return(NULL);
	}
	if (skip_header(fp) == -1) {
		meprintf("bad menu file header");
		menerr = MBADFIL;
		fclose(fp);
		return(NULL);
	}
	for (;;) {
		if ((mnode = (MENU *)malloc(sizeof(MENU))) == NULL)
			abort();
		if ((nodenum = getmnode(fp,mnode,mname)) == LASTNODE)
			break;
		if (nodenum >= MAXNODES) {
			meprintf("Too many nodes");
			menerr = MBADFIL;
			nodenum = -1;
		}
		if (nodenum == -1) {
			for (nodenum = 0; nodenum < MAXNODES; nodenum++) {
				if (Nodevec[nodenum] != NULL)
					free((char *)Nodevec[nodenum]);
				Nodevec[nodenum] = NULL;
			}
			return(NULL);
		}
		Nodevec[nodenum] = mnode;
	}
	fix_parent_links(mnode,(MENU *)NULL);
	for (nodenum = 0; nodenum < MAXNODES; nodenum++)
		Nodevec[nodenum] = NULL;
	return(mnode);
}

static
fix_parent_links(mp,parent)
MENU *mp, *parent;
{
	if (mp == NULL)
		return;
	fix_parent_links(mp->me_topleft,mp);
	fix_parent_links(mp->me_botrite,mp);
	mp->me_parent = parent;
}

/*  skip header of menu file. We depend on the first real line
 *  starting with the 's' of 'static'
 */
static	
skip_header(fp)
FILE *fp;
{
	int ch, lastch;
	
	lastch = '\n';
	while ((ch = mygetc(fp)) != EOF) {
		if ((ch == 's' || ch == 'M') && lastch == '\n') {
			myungetc(ch,fp);
			return(0);
		}
		lastch = ch;
	}
	return(-1);
}

static int
getmnode(fp,mnode,mname)
FILE *fp;
MENU *mnode;
char *mname;
{
	short flags, nodenum;
	
	if ((nodenum = do_struct_header(fp,mname)) == -1||
	    getnum(fp, &flags,',')			||
	    getnum(fp, &mnode->me_pos,',')		||
	    getstring(fp, &mnode->me_cap,',')		||
	    getnum(fp, &mnode->me_rv,',')		||
	    getnum(fp, &mnode->me_xstart,',')		||
	    getnum(fp, &mnode->me_ystart,',')		||
	    getnum(fp, &mnode->me_xend,',')		||
	    getnum(fp, &mnode->me_yend,',')		||
	    getnum(fp, &mnode->me_xcurs,',')		||
	    getnum(fp, &mnode->me_ycurs,',')		||
	    getnum(fp, &mnode->me_colour,',')		||
	    skipnull(fp,',')				||
	    getnodenum(fp, &mnode->me_topleft,',')	||
	    getnodenum(fp, &mnode->me_botrite,',')	||
	    skipnull(fp,',')				||
	    skipnull(fp,',')				||
	    skipwhite(fp,'}')				||
	    skipwhite(fp,';'))
		return(-1);
	mnode->me_flags = flags;
	mnode->me_save = NULL;
	mnode->me_parent = NULL;
	return(nodenum);
}

/* ARGSUSED */
static
skipnull(fp,expected)
FILE *fp;
int expected;
{
	return(checkword(fp,"NULL") != 0 ? -1 : skipwhite(fp,','));
}

static
getstring(fp,p_cptr,expected)
FILE *fp;
char **p_cptr;
int expected;
{
	char buf[256], *cptr;
	int ch;
	char *strcpy();
	
	if ((ch = skipwhite(fp,ANY_CHAR)) == 'N') {
		*p_cptr = NULL;
		ungetc('N',fp);
		return(skipnull(fp,expected));
	}
	else  if (ch == '0') {
		*p_cptr = NULL;
		return(skipwhite(fp,expected));
	}
	else if (ch != '"') {
		meprintf("expected '\"', got %c",ch);
		menerr = MBADFIL;
		return(-1);
	}
	cptr = buf;
	while ((ch = mygetc(fp)) != EOF && ch != '"') {
		if (ch < 32 || ch > 127) {
			/*  illegal character
			 */
			meprintf("illegal character %o (octal)",ch);
			menerr = MBADFIL;
			return(-1);
		}
		if (cptr >= buf + sizeof(buf) - 1) {
			meprintf("string too long");
			menerr = MBADFIL;
			return(-1);
		}
		if (ch == '\\')
			if ((ch = backslash(fp)) == -1)
				return(-1);
		*cptr++ = ch;
	}
	if (ch == EOF) {
		meprintf("unexpected end of file");
		menerr = MBADFIL;
		return(-1);
	}
	*cptr = '\0';
	if ((*p_cptr = malloc((unsigned)(strlen(buf) + 1))) == NULL)
		abort();
	(void) strcpy(*p_cptr,buf);
	return(skipwhite(fp,expected));
}

/*  process C escape sequence
 */
static
backslash(fp)
FILE *fp;
{
	char *pos;
	int ch, res, ndigs;
	
	if ((ch = mygetc(fp)) == EOF) {
		meprintf("unexpected EOF in string in menu file");
		menerr = MBADFIL;
		return(-1);
	}
	if ((pos = index("n\nt\tb\br\r",ch)) != NULL)
		return(pos[1]);
	else if (ch < '0' || ch > '7')
		return(ch);
	res = ndigs = 0;
	while (ndigs++ < 3 && (ch = mygetc(fp)) != EOF && ch>='0' && ch<='7')
		res = res * 8 + ch - '0';
	if (ch == EOF) {
		meprintf("unexpected EOF in string in menu file");
		menerr = MBADFIL;
		return(-1);
	}
	return(res);
}

static
do_struct_header(fp,mname)
FILE *fp;
char *mname;
{
	char wbuf[100];
	short nodenum;
	
	if (getword(fp,wbuf) != 0)
		return(-1);
	if (strcmp(wbuf,"static") == 0) {
		if (checkword(fp,"MENU") != 0 ||
		    checkword(fp,"MM") != 0 ||
		    getnum(fp,&nodenum,'=') == -1 ||
		    skipwhite(fp,'{') != 0)
			return(-1);
		return(nodenum);
	}
	else {
		if (strcmp(wbuf,"MENU") != 0 ||
		    getword(fp,mname) != 0 ||
		    skipwhite(fp,'=') != 0 ||
		    skipwhite(fp,'{') != 0)
			return(-1);
		return(LASTNODE);
	}
}

static
skipwhite(fp,expected)
FILE *fp;
int expected;
{
	int ch;

	while ((ch = mygetc(fp)) == ' ' || ch == '\t' || ch == '\n')
		;
	if (ch == EOF) {
		meprintf("unexpected EOF in menu file");
		menerr = MBADFIL;
		return(-1);
	}
	if (expected != ANY_CHAR && ch != expected) {
		meprintf("expected '%c', got '%c' in menu file",expected,ch);
		menerr = MBADFIL;
		return(-1);
	}
	return((expected == ANY_CHAR) ? ch : 0);
}

static
getword(fp,buf)
FILE *fp;
char *buf;
{
	int ch;
	
	if ((ch = skipwhite(fp,ANY_CHAR)) == -1 || !(isascii(ch)&&isalpha(ch)))
		return(-1);
	while (isascii(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) {
		if (buf != NULL)
			*buf++ = ch;
		ch = mygetc(fp);
	}
	if (buf != NULL)
		*buf = '\0';
	return(0);
}

static
checkword(fp,word)
FILE *fp;
char *word;
{
	int ch;
	
	if (skipwhite(fp,*word++) != 0)
		return(-1);
	while (*word != '\0' && (ch = mygetc(fp)) == *word++)
		;
	if (*word != '\0') {
		if (ch == EOF) {
			meprintf("unexpected EOF");
			menerr = MBADFIL;
		} else {
			meprintf("\"%s\" expected", word);
			menerr = MBADFIL;
		}
		return(-1);
	}
	return(0);
}

static int 
getnodenum(fp,p_mnode,expected)
FILE *fp;
MENU **p_mnode;
int expected;
{
	int ch;
	short nodenum;
	
	if ((ch = skipwhite(fp,ANY_CHAR)) == '&') {
		if (skipwhite(fp,'M') != 0 ||
		    mygetc(fp) != 'M' ||
		    getnum(fp,&nodenum,expected) == -1)
			return(NULL);
		if (nodenum < 0 || nodenum >= MAXNODES ||
					Nodevec[nodenum] == NULL){
			meprintf("bad node number in menu file");
			menerr = MBADFIL;
			return(-1);
		}
		*p_mnode = Nodevec[nodenum];
		return(0);
	}
	else if (ch == 'N') {
		if (checkword(fp,"ULL") != 0)
			return(-1);
	}
	else if (ch != '0') {
		meprintf("bad menu pointer field in menu file");
		menerr = MBADFIL;
		return(-1);
	}
	*p_mnode = NULL;
	return(skipwhite(fp,expected));
}

static
getnum(fp, p_res, followch)
FILE *fp;
short *p_res;
int followch;
{
	int ch, base, res, minus;
	
	if ((ch = skipwhite(fp,ANY_CHAR)) == -1)
		return(-1);
	minus = 0;
	if (ch == '0') {
		if ((ch = mygetc(fp)) == 'x')
			base = 16;
		else {
			base = 8;
			myungetc(ch,fp);
		}
	}
	else if (isdigit(ch) || ch == '-') {
		base = 10;
		if (ch == '-')
			minus = 1;
		else
			myungetc(ch,fp);
	}
	else {
		meprintf("number expected in menu file");
		menerr = MBADFIL;
		return(-1);
	}
	res = 0;
	for (;;) {
		if ((ch = mygetc(fp)) == EOF) {
			meprintf("unexpected EOF in menu file");
			menerr = MBADFIL;
			return(-1);
		}
		if (isascii(ch) && islower(ch))
			ch = toupper(ch);
		if (isdigit(ch) || (base == 16 && ch >= 'A' && ch <= 'F'))
			res = res * base + ch - ((ch > '9') ? 'A'-10 : '0');
		else {
			*p_res = minus ? -res : res;
			myungetc(ch,fp);
			return(skipwhite(fp,followch));
		}
	}
}

/*  Code for reporting back on errors in the menu file.
 */

/* VARARGS */
static void
meprintf(fmt,arg1,arg2,arg3,arg4,arg5,arg6)
char *fmt;
{
	(void)sprintf(merr.mr_message,fmt,arg1,arg2,arg3,arg4,arg5,arg6);
}

static
mygetc(fp)
FILE *fp;
{
	int c;

	if ((c = getc(fp)) == '\n')
		merr.mr_line++;
	return(c);
}

static void
myungetc(c,fp)
int c;
FILE *fp;
{
	ungetc(c,fp);
	if (c == '\n')
		merr.mr_line--;
}


/* destroy the memory allocated for this menu */
static void
Mremove(md)
int md;
{
	freemen(_menu_[md].om_root);
	_menu_[md].om_root = NULL;
}

/*  Free the memory occupied by the menu.
 */
static void
freemen(menu)
MENU * menu;
{
	if (menu == NULL)
		return;
	freemen(menu->me_topleft);
	freemen(menu->me_botrite);
	if ((menu->me_cap != NULL) && (menu->me_flags & ME_FREC))
		free(menu->me_cap);
	if (menu->me_flags & ME_FREN)
		free((char *)menu);
}
/*---^^^^^---------end of Software Tools Copyrighted stuff---------------*/

/*---vvvvv------------X Toolkit specific class routines------------------*/

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

/* ARGSUSED */
static void
Class_Initialize(myclass)
WidgetClass myclass;
{
	current_num = 0;
}
	

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
	UKCMed3MenuWidget nw = (UKCMed3MenuWidget)new;

	XtAppAddActions(XtWidgetToApplicationContext(new),
			(XtActionList)menuActions, XtNumber(menuActions));

	popup_bm = nw->med3Menu.popup_pixmap; /* global variable */

	if (nw->med3Menu.use_file) {
		if (nw->med3Menu.menu_file != NULL) {
			String file = XtNewString(nw->med3Menu.menu_file);

			nw->med3Menu.menu_file = file;
			CreateMenuFromFile(nw, nw->med3Menu.menu_file);
		} else if (nw->med3Menu.menu_struct != NULL)
			CreateMenu(nw, nw->med3Menu.menu_struct);
	} else if (nw->med3Menu.menu_struct != NULL)
		CreateMenu(nw, nw->med3Menu.menu_struct);
	else if (nw->med3Menu.menu_file != NULL) {
		String file = XtNewString(nw->med3Menu.menu_file);

		nw->med3Menu.menu_file = file;
		CreateMenuFromFile(nw, nw->med3Menu.menu_file);
	}

	if (nw->composite.num_children == 0)
		XtVaCreateManagedWidget("message", ukcLabelWidgetClass,
				new, XtNlabel, "<No Menu Set>", NULL);

	/* ||| for backwards compatiblility only ||| */
	nw->med3Menu.menu_widget = new;
}


/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
	UKCMed3MenuWidget wc = (UKCMed3MenuWidget)current;
	UKCMed3MenuWidget wn = (UKCMed3MenuWidget)new;

	if ((wc->med3Menu.menu_file != wn->med3Menu.menu_file) ||
	    (wc->med3Menu.menu_struct != wn->med3Menu.menu_struct) ||
	    (wc->med3Menu.use_file != wn->med3Menu.use_file)) {
		if (wn->med3Menu.use_file) {
			if (wc->med3Menu.menu_file != wn->med3Menu.menu_file &&
			    wn->med3Menu.menu_file != NULL) {
				String file =
					XtNewString(wn->med3Menu.menu_file);
				XtFree(wc->med3Menu.menu_file);
				wn->med3Menu.menu_file = file;
				CreateMenuFromFile(wn, wn->med3Menu.menu_file);
			} else if (wc->med3Menu.menu_struct !=
				   wn->med3Menu.menu_struct
			    && wn->med3Menu.menu_struct != NULL)
				CreateMenu(wn, wn->med3Menu.menu_struct);
		} else if (wc->med3Menu.menu_struct != wn->med3Menu.menu_struct
			    && wn->med3Menu.menu_struct != NULL)
				CreateMenu(wn, wn->med3Menu.menu_struct);
		else if (wc->med3Menu.menu_file != wn->med3Menu.menu_file &&
			    wn->med3Menu.menu_file != NULL) {
			String file = XtNewString(wn->med3Menu.menu_file);

			XtFree(wc->med3Menu.menu_file);
			wn->med3Menu.menu_file = file;
			CreateMenuFromFile(wn, wn->med3Menu.menu_file);
		}
	}

#if 0 /* ||| NYIP ||| */
	if (wc->med3Menu.no_border != wn->med3Menu.no_border)
		XtWarning("XtNnoBorder can only be set when a UKCMed3MenuWidget is first created");
#endif /* ||| NYIP ||| */
	return (FALSE);
}


static XtGeometryResult
Query_Geometry(widget, constraint, preferred)
Widget widget;
XtWidgetGeometry *constraint, *preferred;
{
	UKCMed3MenuWidget w = (UKCMed3MenuWidget)widget;

	if (w->composite.num_children == 0)
		return (XtGeometryYes);

	preferred->request_mode = CWWidth | CWHeight;
	preferred->width = w->core.width;
	preferred->height = w->core.height;

	if (((constraint->request_mode & CWHeight) &&
	     (constraint->height != preferred->height)) ||
	    ((constraint->request_mode & CWWidth) &&
	     (constraint->width != preferred->width)))
		return (XtGeometryAlmost);
	else
		return (XtGeometryYes);
}
		

/* Menu Action Routines */
#ifdef DEBUG
static void
Invoke(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	printf("Button \"%s\" callback invoked\n", w->core.name);
}
#endif DEBUG


/* clear a UKCCommandWidget Button */
static void
clear_button(w)
Widget w;
{
	UKCCommandWidget cw;
	WidgetClass cwc;

	cw = (UKCCommandWidget)w;
	cwc = XtClass(w);

	/* ||| KLUDGE ||| */
	/* this clears the highlighting and is specific
	 * to the UKCCommand button widget class */
	if (cw->command.set) {
		cw->command.set = FALSE;
		cw->label.normal_GC = cw->command.normal_GC;
		(*cwc->core_class.expose)(w, (XEvent *)NULL, (Region)NULL);
	}
}


/* ARGSUSED */
static void
PopDown(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	PopupData pop = (PopupData)client_data;

	if (pop == NULL) {
		XtWarning("No data passed to PopDown()");
		return;
	}

	/* recursively pop down any menus above this one */
	if (pop->last_up_data != NULL) {
		if (pop->last_up_data->inhibit_popdown)
			return;

		PopDown(pop->last_setpopup, (XtPointer)pop->last_up_data,
							(XtPointer)NULL);
	}

	if (pop->last_chosen != NULL) {
		clear_button(pop->last_chosen);
		pop->last_chosen = NULL;
	}

	if (pop->last_up != NULL) {
		XtPopdown(pop->last_up);

		/* clear the highlighting on the button which popped this
		 * menu up. */
		clear_button(pop->last_setpopup);

		pop->last_up = NULL;
		pop->last_setpopup = NULL;
		pop->last_up_data = NULL;
	}
}


/* ARGSUSED */
static void
ForceDown(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	PopupData pop = (PopupData)client_data;
	PopupData last_up = pop->last_up_data;
	Boolean old_inhibit = FALSE;

	if (last_up != NULL) {
		old_inhibit = last_up->inhibit_popdown;
		last_up->inhibit_popdown = FALSE;
		PopDown(pop->last_setpopup, (XtPointer)pop, (XtPointer)NULL);
		last_up->inhibit_popdown = old_inhibit;
	}
}


/* ARGSUSED */
static void
popdown(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int fred;
	PopupData pud;

	(void)sscanf(params[0], "%d", &fred);
	pud = (PopupData)fred;

	PopDown(w, (XtPointer)pud, (XtPointer)NULL);
}


/* ARGSUSED */
static void
Clean_Down(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	PopupData popupdata = (PopupData)client_data;

	XtFree((char *)popupdata);
#ifdef DEBUG
printf("Clean_Down(%s)\n", w->core.name);
#endif DEBUG
}


/* ARGSUSED */
static void
tear_off(w, event, params, num_params)
Widget  w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int fred;
	PopupData pud;

	(void)sscanf(params[0], "%d", &fred);		/* popup data */
	pud = (PopupData)fred;
	if (pud->torn_off)
		return;
	pud->torn_off = TRUE;
printf("TEAR OFF\n");
}


/* ARGSUSED */
static void
move_tear_off(w, event, params, num_params)
Widget  w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int fred;
	PopupData pud;

	(void)sscanf(params[0], "%d", &fred);		/* popup data */
	pud = (PopupData)fred;

	if (!pud->torn_off)
		return;
printf("MOVED TEAR OFF\n");
}


/* ARGSUSED */
static void
drop_tear_off(w, event, params, num_params)
Widget  w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int fred;
	PopupData pud;

	(void)sscanf(params[0], "%d", &fred);		/* popup data */
	pud = (PopupData)fred;

	if (!pud->torn_off)
		return;
	pud->torn_off = FALSE;
printf("DROPPED TEAR OFF\n");
}


/* ARGSUSED */
static void
last_set(w, event, params, num_params)
Widget  w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int fred;

	(void)sscanf(params[0], "%d", &fred);		/* popup data */
	((PopupData)fred)->last_chosen = w; 
}


/* ARGSUSED */
static void set_popup(w, event, params, num_params)
Widget  w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	Position x, y;
	Widget popupshell;
	int fred, john;
	Boolean is_relative;
	PopupData pud, pusd;

	/* set_popup(pud pusd [TRUE])
	 */
	
	if ((*num_params != 2) && (*num_params != 3)) {
		XtWarning("Bad parameter count in set_popup()");
		return;
	}

	(void)sscanf(params[0], "%d", &fred);		/* popup data */
	pud = (PopupData)fred; 

	if (pud == NULL) {
		XtWarning("Bad popup data passed to set_popup()");
		return;
	}

	(void)sscanf(params[1], "%d", &john);		/* popup shell data */
	pusd = (PopupData)john;

	if (pusd == NULL) {
		XtWarning("Bad popup shell data passed to set_popup()");
		return;
	}

	popupshell = pusd->shell;

	/* shell already up! */
	if (pud->last_up == popupshell)
		return;

	if (*num_params == 3)
		is_relative = TRUE;
	else
		is_relative = FALSE;

	if (pud->last_up != NULL)
		PopDown(pud->last_setpopup, (XtPointer)pud, (XtPointer)NULL);

#if 1 /* ||| KLUDGE:
       * |||    It should be done this way but because the intrinsics do
       * |||    not "correctly" support reparenting window managers .. we
       * |||    must make a server request instead. ||| */
	XtTranslateCoords(w, is_relative ? event->xbutton.x + pusd->shell_x
					 : pusd->shell_x,
			     is_relative ? event->xbutton.y + pusd->shell_y
					 : pusd->shell_y,
			     &x, &y);
#else
{
	Window dummy;

	XTranslateCoordinates(XtDisplay(w), XtScreen(w)->root, XtWindow(w),
				is_relative ? event->xbutton.x + pusd->shell_x
					    : pusd->shell_x,
				is_relative ? event->xbutton.y + pusd->shell_y
					    : pusd->shell_y,
				&x, &y, &dummy);
}
#endif /* ||| KLUDGE ||| */

	XtVaSetValues(popupshell, XtNx, x, XtNy, y, NULL);

	pud->last_up = popupshell;
	pud->last_setpopup = w;
	pud->last_up_data = pusd;

	pusd->last_setpopup = NULL;
	pusd->last_up = NULL;
	pusd->last_up_data = NULL;
}


#ifdef DEBUG
/* ARGSUSED */
static void
debug_stop(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	printf("DEBUGGER ON\n");
}
#endif DEBUG

/**************************** Private Routines *****************************/

/****** Misc Routines ******/

/*  return sys_errlist[errno] if in range
 */
static String
reason()
{
	extern char *sys_errlist[];
	extern int errno, sys_nerr;

	return( (errno > 0 && errno < sys_nerr) ? sys_errlist[errno]
							: "unknown reason");
}


static void
replace_translations(w, new_trans)
Widget w;
String new_trans;
{
	Arg args[1];

	XtSetArg(args[0], XtNtranslations, XtParseTranslationTable(new_trans));
	XtSetValues(w, (ArgList)args, 1);
}


#ifdef DEBUG
print_flags(xtmenu)
MENU *xtmenu;
{
	if (xtmenu == NULL)
		return;

	printf ("Flags Set :-\n");
	if TF(ME_VER)
		printf("\tVertical menu\n");
	if TF(ME_HOR)
		printf("\tHorizontal menu\n");
	if TF(ME_DIV)
		printf("\tDivided menu\n");
	if TF(ME_NSEL)
		printf("\tCan't select this node\n");
	if TF(ME_POPUP)
		printf("\tPopup flag\n");
	if TF(ME_FREC)
		printf("\tCaption can be freed\n");
	if TF(ME_FREN)
		printf("\tNode can be freed\n");
	if TF(ME_OPEN)
		printf("\tNode is open\n");
	if TF(ME_ISUB)
		printf("\tNode has an indivisible sub-menu\n");
	if TF(ME_FREE)
		printf("\tFree Standing Node\n");
	if TF(ME_BGLINE)
		printf("\tDraw lines in Background colour\n");
	if TF(ME_NOSCALE)
		printf("\tDon't Scale submenu\n");
	if TF(ME_CREL) {
		printf("\tCursor relative sub-menu\n");
		printf ("Position , x = %d , y = %d , width = %d , height = %d\n", xtmenu->me_xstart, xtmenu->me_ystart, xtmenu->me_xend-xtmenu->me_xstart, xtmenu->me_yend - xtmenu->me_ystart);
	}
	if TF(ME_REDRAW)
		printf("\tRequires redrawing\n");
	if TF(ME_LEFT)
		printf("\tLeft justify caption\n");

	if (TF(ME_FREE) || TF(ME_POPUP))
		printf ("Position , x = %d , y = %d , width = %d , height = %d\n", xtmenu->me_xstart, xtmenu->me_ystart, xtmenu->me_xend-xtmenu->me_xstart, xtmenu->me_yend - xtmenu->me_ystart);
	if (xtmenu->me_cap != NULL)
		printf("Caption = %s\n", xtmenu->me_cap);

	putchar('\n');
}
#endif DEBUG


static int
next_num()
{
	if (current_num == MAXNUM)
		current_num = 1;
	else
		current_num++;
	return (current_num);
}

/****** Button routines ******/

/* callback routine called when a button widget is destroyed. */
/* ARGSUSED */
static void
destroy_button(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Buttons butt = (Buttons)client_data;

	if (butt == NULL) {
		XtWarning("destroy_button() called with NULL button pointer");
		return;
	}

#ifdef DEBUG
printf("Destroy_Button(%s)\n", butt->button_widget->core.name);
#endif DEBUG
	/* remove this dying button from the menu's button list */
	if (butt->next != NULL)
		butt->next->prev = butt->prev;
	if (butt->prev != NULL)
		butt->prev->next = butt->next;
	else	/* head of list so update menu's pointer to list */
		butt->menu_widget->med3Menu.button_list = butt->next;
	XtFree((char *)butt);	
}


/* allocate some space for button data and add a callback to the button's
 * XtNdestroy callback list */
static Buttons
create_button(menuparent, w, pmd, value)
UKCMed3MenuWidget menuparent;
Widget w;
PopupData pmd;
short value;
{
	Buttons temp;

	temp = XtNew(ButtonRec);

	temp->menu_widget = menuparent;
	temp->button_widget = w;
	temp->button_value = value;
	temp->shell_data = pmd;
	temp->next = Mbuttons; /* global variable */
	temp->prev = NULL;
	temp->button_type = NO_PARENT;
	temp->toggle_data = NULL;
	if (Mbuttons != NULL)
		Mbuttons->prev = temp; /*global variable */

	XtAddCallback(w, XtNdestroyCallback, destroy_button, (XtPointer)temp);
	return (Mbuttons = temp);
}

	
/* recursively find button data by return value and optionally by the widget
 * button it is assigned to */
static Buttons
find_button(buttons, value, w)
Buttons buttons;
short value;
Widget w;
{
	if (buttons == NULL)
		return ((Buttons)NULL);

	if (w != NULL) {
		if ((buttons->button_widget == w) &&
		    (buttons->button_value == value))
			return (buttons);
	} else if (buttons->button_value == value)
			return (buttons);

	return (find_button(buttons->next, value, w));
}


/* remove an application callback from a menu button */
static void
unbind_button(button, bind_proc, proc_data)
Buttons button;
XtCallbackProc bind_proc;
XtPointer proc_data;
{
	if (button == NULL)
		XtWarning("Cannot unbind procedure from non existent button");
	else
		XtRemoveCallback(button->button_widget, XtNcallback,
				 bind_proc, proc_data);
}


/* bind an application callback to a menu button */
static void
bind_button(button, bind_proc, proc_data)
Buttons button;
XtCallbackProc bind_proc;
XtPointer proc_data;
{
	if (button == NULL)
		XtWarning("Cannot bind procedure to non existent button");
	else
		XtAddCallback(button->button_widget, XtNcallback,
				bind_proc, proc_data);
}
		
/****** Toggle Button Routines ******/

/* ARGSUSED */
static XtCallbackProc
free_toggle(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	struct toggledata *tdata = (struct toggledata *)client_data;
#ifdef DEBUG
printf("Free_Toggle(%s/%s)\n", tdata->on, tdata->off);
#endif DEBUG
	if (tdata->on != NULL)
		XtFree((char *)tdata->on);
	if (tdata->off != NULL)
		XtFree((char *)tdata->off);
	XtFree((char *)tdata);
}


/* ARGSUSED */
static XtCallbackProc
toggle_label(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	struct toggledata *tdata = (struct toggledata *)client_data;

	tdata->state = !tdata->state;
	XtVaSetValues(w, XtNlabel, tdata->state ? tdata->on : tdata->off, NULL);

	if (tdata->address != NULL)
		(*tdata->address) = tdata->state;
	if (tdata->proc != NULL)
		(*tdata->proc)(w, tdata->data, (XtPointer)tdata->state);
}


static void
set_toggle_state(button, new_state, notify)
Buttons button;
Boolean new_state;
Boolean notify;
{
	button->toggle_data->state = !new_state;

	if (!notify) {
		XtCallbackProc old_proc = button->toggle_data->proc;
		/* don't call the callback procedure */
		button->toggle_data->proc = NULL;
		toggle_label(button->button_widget,
			(XtPointer)button->toggle_data, (XtPointer)NULL);
		button->toggle_data->proc = old_proc;
	} else
		toggle_label(button->button_widget,
			(XtPointer)button->toggle_data, (XtPointer)NULL);
}


static void
bind_toggle(button, on_string, off_string, address, bind_proc, proc_data)
Buttons button;
String on_string, off_string;
Boolean *address;
XtCallbackProc bind_proc;
XtPointer proc_data;
{
	struct toggledata *tdata;
	Arg args[1];

	if (button == NULL) {
		XtWarning("Cannot bind procedure to non existent button");
		return;
	}

	tdata = XtNew(struct toggledata);
	if (on_string != NULL)
		tdata->on = XtNewString(on_string);
	else
		tdata->on = XtNewString("");
	if (off_string != NULL)
		tdata->off = XtNewString(off_string);
	else
		tdata->off = XtNewString("");
	tdata->state = ((address == NULL) ? FALSE : *address);
	tdata->proc = bind_proc;
	tdata->data = proc_data;
	tdata->address = address;
	button->toggle_data = tdata;	/* used by XukcUnbindToggle() */
	button->button_type |= TOGGLE_BUTTON;
	XtAddCallback(button->button_widget, XtNcallback,
					(XtCallbackProc)toggle_label,
					(XtPointer)tdata);
	XtAddCallback(button->button_widget, XtNdestroyCallback,
					(XtCallbackProc)free_toggle,
					(XtPointer)tdata);
	if (address != NULL)
		XtSetArg(args[0], XtNlabel, *address ? tdata->on : tdata->off);
	else
		XtSetArg(args[0], XtNlabel, tdata->off);
	XtSetValues(button->button_widget, (ArgList)args, 1);
}
		

static void
unbind_toggle(button)
Buttons button;
{
	XtRemoveCallback(button->button_widget, XtNcallback, 
				(XtCallbackProc)toggle_label,
				(XtPointer)button->toggle_data);
	XtRemoveCallback(button->button_widget, XtNdestroyCallback, 
				(XtCallbackProc)free_toggle,
				(XtPointer)button->toggle_data);
	free_toggle((Widget)NULL, (XtPointer)button->toggle_data,
						(XtPointer)NULL);
	button->toggle_data = NULL;
	button->button_type &= ~TOGGLE_BUTTON;
}
		

/****** Menu Creation Routines ******/

static PopupData
create_popup_shell_data()
{
	PopupData pmd;

	pmd = XtNew(PopupDRec);
	pmd->parents_data = NULL;
	pmd->last_up = NULL;
	pmd->last_setpopup = NULL;
	pmd->last_up_data = NULL;
	pmd->last_chosen = NULL;
	pmd->shell = NULL;
	pmd->shell_x = 0;
	pmd->shell_y = 0;
	pmd->inhibit_popdown = FALSE;

	return (pmd);
}


/* MAIN ROUTINE : Traverses the tree created by the Med3 routines and
 * generates a widget hierarchy of UKCMenuPanes containing UKCCommand Widgets
 * that implement the menu.  Popup submenus have their own shells.
 */
static Widget
create_menu(parent_menu, xtmenu, parent, parent_type, pmd, horiz_menu,
				connection, fixed_size, root_x, root_y)
UKCMed3MenuWidget parent_menu;
MENU *xtmenu;
Widget parent;
int parent_type;
PopupData pmd;
Boolean horiz_menu;
Widget connection;
Boolean fixed_size;
Position root_x, root_y;
{
	Arg args[20];		/* only 14 can be used but lets be safe! */
	Buttons button;
	Cardinal i;
	Widget this_w, old_conn;
#define NAME_SIZE 1000
	char tmpc[NAME_SIZE];

	old_conn = connection;

	if (xtmenu == NULL)
		return (old_conn);

#ifdef DEBUG
	print_flags(xtmenu);
#endif DEBUG

	/* free standing node */
	if TF(ME_FREE) {
		i = 0;
		if (parent_type == NO_PARENT) { /* root of the menu */
			if (parent_menu->core.width == 0) {
				XtSetArg(args[i], XtNwidth,
					xtmenu->me_xend - xtmenu->me_xstart);
				i++;
			}
			if (parent_menu->core.height == 0) {
				XtSetArg(args[i], XtNheight,
					xtmenu->me_yend - xtmenu->me_ystart);
				i++;
			}
		} else {
			XtSetArg(args[i], XtNx, -1); i++;
			XtSetArg(args[i], XtNy, -1); i++;
			XtSetArg(args[i], XtNwidth,
					xtmenu->me_xend - xtmenu->me_xstart);
			i++;
			XtSetArg(args[i], XtNheight,
					xtmenu->me_yend - xtmenu->me_ystart);
			i++;
			(void)sprintf(tmpc, "med3MenuLayout%d", next_num());
		}

		if (parent_menu->med3Menu.no_border) {
			XtSetArg(args[i], XtNborderWidth, 0); i++;
		}
		if (horiz_menu != TFB(ME_HOR))
			horiz_menu = TFB(ME_HOR);

		XtSetArg(args[i], XtNvertPane, !horiz_menu); i++;
		XtSetArg(args[i], XtNaskChild,
			 parent_menu->med3Menu.shrink_menu); i++;
		XtSetArg(args[i], XtNresize, !fixed_size); i++;

		if (parent_type == NO_PARENT) {
			parent = (Widget)parent_menu;
			XtSetValues(parent, (ArgList)args, i);
		} else 
			parent = XtCreateWidget(tmpc, ukcMenuPaneWidgetClass,
						parent, (ArgList)args, i);
		connection = NULL;

		/* if this is a root node then create a shell data record */
		switch (parent_type) {
			case NO_PARENT :
				pmd = create_popup_shell_data();
				pmd->shell_x = root_x;
				pmd->shell_y = root_y;
				XtAddCallback(parent, XtNdestroyCallback,
						Clean_Down, (XtPointer)pmd);
				startdata = pmd; /* global variable */
				startwidget = parent; /* global variable */
				break;
			case DYNAMIC_POPUP :
			case STATIC_POPUP :
				pmd->shell_x = xtmenu->me_xstart - root_x;
				pmd->shell_y = xtmenu->me_ystart - root_y;
				XtManageChild(parent);
				break;
			default:
				XtError("Bad internal data in CreateMenu{FromFile}()");
				break; /* NOTREACHED */
		}

		root_x = xtmenu->me_xstart;
		root_y = xtmenu->me_ystart;
	}

	if TF(ME_DIV) {
		if ((horiz_menu != TFB(ME_HOR)) && !(TF(ME_FREE))) {
			while ((connection != NULL) &&
			       (XtParent(connection) != parent))
				connection = XtParent(connection);

			i = 0;
			XtSetArg(args[i], XtNx, xtmenu->me_xstart - root_x - 1);
			i++;
			XtSetArg(args[i], XtNy, xtmenu->me_ystart - root_y - 1);
			i++;
			XtSetArg(args[i], XtNwidth,
				xtmenu->me_xend - xtmenu->me_xstart); i++;
			XtSetArg(args[i], XtNheight,
				xtmenu->me_yend - xtmenu->me_ystart); i++;
			(void)sprintf(tmpc, "menu_layout%d", next_num());
			if (parent_menu->med3Menu.no_border) {
				XtSetArg(args[i], XtNborderWidth, 0); i++;
			}
			XtSetArg(args[i], XtNvertPane, !TFB(ME_HOR)); i++;
			XtSetArg(args[i], XtNaskChild,
				 parent_menu->med3Menu.shrink_menu); i++;
			XtSetArg(args[i], XtNresize, !fixed_size); i++;
			parent = XtCreateManagedWidget(tmpc,
						ukcMenuPaneWidgetClass,
						parent, (ArgList)args, i);
			connection = NULL;
			root_x = xtmenu->me_xstart;
			root_y = xtmenu->me_ystart;
		}
		horiz_menu = TFB(ME_HOR);
	}

	/* button nodes */
	if (xtmenu->me_cap != NULL) {
		int width, height;
		char buttonName[NAME_SIZE];
		char buttonLabel[NAME_SIZE];

		(void)strncpy(buttonLabel, xtmenu->me_cap, NAME_SIZE-1);

		(void)sprintf(buttonName, "%sMed3Button", buttonLabel);
		for (i = 0; i < NAME_SIZE && buttonName[i] != '\0'; i++)
			if (isspace(buttonName[i]))
				buttonName[i] = '_';
		XmuCopyISOLatin1Lowered(buttonName, buttonName);

		i = 0;
		width = xtmenu->me_xend - xtmenu->me_xstart;
		height = xtmenu->me_yend - xtmenu->me_ystart;

		if (parent_menu->med3Menu.no_border || TF(ME_BGLINE)) {
			XtSetArg(args[i], XtNborderWidth, 0); i++;
		} else {
			if (horiz_menu)
				width--;
			else
				height--;
		}

#ifdef DEBUG
printf("%s : width = %d , height = %d\n", buttonLabel, width, height);
#endif DEBUG
		XtSetArg(args[i], XtNlabel, buttonLabel); i++;
		XtSetArg(args[i], XtNx, xtmenu->me_xstart - root_x - 1); i++;
		XtSetArg(args[i], XtNy, xtmenu->me_ystart - root_y - 1); i++;
		XtSetArg(args[i], XtNwidth, width); i++;
		XtSetArg(args[i], XtNheight, height); i++;
		XtSetArg(args[i], XtNresize, parent_menu->med3Menu.shrink_menu);
		i++;
		XtSetArg(args[i], XtNhighlightThickness, 0); i++;
		switch (parent_type) {
			case NO_PARENT:
			case STATIC_POPUP:
				args[i-1].value = parent_menu->med3Menu.highlight;
				break;
			case DYNAMIC_POPUP:
				(void)sprintf(tmpc, dButtonTranslations,
					      pmd, pmd->parents_data);
#if 0
					      pmd, pmd, pmd);
#endif
				XtSetArg(args[i], XtNtranslations,
					XtParseTranslationTable(tmpc)); i++;
				break;
			default:
				break;
		}

		if TF(ME_LEFT) {
			XtSetArg(args[i], XtNjustify, XtJustifyLeft);
			i++;
		}

		this_w = XtCreateManagedWidget(buttonName,
				ukcCommandWidgetClass,
				parent, (ArgList)args, i);
#ifdef DEBUG
		XtAddCallback(this_w, XtNcallback, Invoke, (XtPointer)NULL);
#endif /* DEBUG */
		connection = this_w;

		/* create the button-binding record for this menu button */
		button = create_button(parent_menu, this_w, pmd, xtmenu->me_rv);

		/* For static-popups add a default callback routine that
		 * causes the popup to be pushed down after it has been
		 * pressed. Binding a force down causes the menu to change
		 * from this default style to one that stays up until a
		 * particular button gets pressed and then is pushed down.
		 */
		if (parent_type == STATIC_POPUP)
			bind_button(button, ForceDown,
				(XtPointer)button->shell_data->parents_data);
	}

	/* pop up nodes */
	if TF(ME_ISUB) {
		int old_parent = parent_type;
		PopupData old_pmd = pmd;
		char fred[300];
		char *erik;

		if (popup_bm != None)
			XtVaSetValues(this_w, XtNrightBitmap, popup_bm, NULL);

		/* translate spaces to underlines */
		(void)sprintf(fred,"%sMed3PopUpMenu", this_w->core.name);
		for (erik = fred; *erik != '\0'; erik++)
			if (*erik == ' ')
				*erik = '_';

		parent_type = TF(ME_POPUP) ? DYNAMIC_POPUP : STATIC_POPUP;

		i = 0;
		if (parent_type == DYNAMIC_POPUP) {
			(void)sprintf(tmpc, shellTranslations,
						old_pmd, old_pmd, old_pmd,
								old_pmd);
			XtSetArg(args[i], XtNtranslations,
					  XtParseTranslationTable(tmpc));
			i++;
		}
		XtSetArg(args[i], XtNallowShellResize, !fixed_size); i++;
#if 1 /* ||| which is right for ICCCM compliance? ||| */
		parent = XtCreatePopupShell(fred, parent_type == STATIC_POPUP ?
				transientShellWidgetClass :
				overrideShellWidgetClass,
				parent, (ArgList)args, i);
#else
		parent = XtCreatePopupShell(fred, overrideShellWidgetClass,
				parent, (ArgList)args, i);
#endif /* ||| */

		pmd = create_popup_shell_data();
		pmd->parents_data = old_pmd;
		pmd->shell = parent;

		XtAddCallback(parent, XtNdestroyCallback, Clean_Down, (XtPointer)pmd);
		button->button_type = parent_type;
		
		*tmpc = '\0';
		switch (old_parent) {
			case NO_PARENT:
			case STATIC_POPUP:
				if TF(ME_CREL)
					(void)sprintf(tmpc,
							cr_sButtonTranslations,
							old_pmd, pmd,
							parent->core.name);
				else
					(void)sprintf(tmpc,
							fp_sButtonTranslations,
							old_pmd, pmd,
							parent->core.name);
				break;
			case DYNAMIC_POPUP :
				if TF(ME_CREL)
					(void)sprintf(tmpc,
						cr_dButtonTranslations,
						old_pmd, pmd->parents_data,
						pmd,
						parent->core.name);
#if 0 /* ||| */
						pmd, pmd, pmd);
#endif
				else
					(void)sprintf(tmpc,
						fp_dButtonTranslations,
						old_pmd, pmd->parents_data,
						pmd,
						parent->core.name);
#if 0 /* ||| */
						pmd, pmd, pmd);
#endif
				if (!TF(ME_POPUP))
					pmd->inhibit_popdown = TRUE;
				break;
			default:
				break;
		}

		if (*tmpc != '\0')
			replace_translations(this_w, tmpc);

		root_x = TF(ME_CREL) ? xtmenu->me_xend : xtmenu->me_xstart;
		root_y = TF(ME_CREL) ? xtmenu->me_yend : xtmenu->me_ystart;

		fixed_size = TF(ME_NOSCALE);
	}

	/* traverse left branch of menu tree */
	connection = create_menu(parent_menu, xtmenu->me_topleft,
				 parent, parent_type,
				 pmd, horiz_menu, connection,
				 fixed_size, root_x, root_y);

	/* traverse right branch of menu tree */
	connection = create_menu(parent_menu, xtmenu->me_botrite,
				 parent, parent_type,
				 pmd, horiz_menu, connection,
				 fixed_size, root_x, root_y);

	return (TF(ME_FREE) ? old_conn : connection);
}


static void
do_create(parent, id)
UKCMed3MenuWidget parent;
int id;
{
	Cardinal i;

	/* initialize global variables */
	Mbuttons = NULL;
	startwidget = NULL;
	startdata = NULL;

	if (XtIsRealized((Widget)parent)) {
		for (i = 0; i < parent->composite.num_children; i++)
			if (XtIsManaged(parent->composite.children[i]))
			     XtDestroyWidget(parent->composite.children[i]);

		for (i = 0; i < parent->core.num_popups; i++)
			XtDestroyWidget(parent->core.popup_list[i]);
	} else {
		while ((i = parent->composite.num_children) > 0)
			if (XtIsManaged(parent->composite.children[i-1]))
			     XtDestroyWidget(parent->composite.children[i-1]);

		while ((i = parent->core.num_popups) > 0)
			XtDestroyWidget(parent->core.popup_list[i-1]);
	}

	(void)create_menu(parent, _menu_[id].om_root,
			  (Widget)NULL, NO_PARENT,
			  (PopupData)NULL, FALSE,
			  (Widget)NULL, TRUE, 0, 0);
	Mremove(id);

	parent->med3Menu.button_list = Mbuttons;
	parent->med3Menu.shell_data = startdata;
}


static void
CreateMenuFromFile(parent, filename)
UKCMed3MenuWidget parent;
String filename;
{
	int id;

	id = Mopen(filename);

	if (id == -1) {
		String params[2];
		Cardinal num_params = 2;

		if (!parent->med3Menu.use_file)
			return;

		params[0] = filename;
		params[1] = reason();
		XtAppWarningMsg(XtWidgetToApplicationContext((Widget)parent),
				"fileIO", "xukcMedMenuCreateFromFile",
				"XukcToolkitError",
				"Cannot open Med3Menu file \"%s\" : (%s)",
				params, &num_params);
	} else
		do_create(parent, id);
}


static void
CreateMenu(parent, menu_ptr)
UKCMed3MenuWidget parent;
MENU *menu_ptr;
{
	int id;

	id = Minsert(menu_ptr);

	if (id == -1) {
		Cardinal num_params = 0;

		XtAppWarningMsg(XtWidgetToApplicationContext((Widget)parent),
				"badData", "xukcMedMenuCreate",
				"XukcToolkitError",
				"Cannot create menu from structure",
				(String *)NULL, &num_params);
	} else
		do_create(parent, id);
}

/* Public Routines */

/* Find the widget button in a given menu that has a certain med3 button
 * value associated with it.
 */
Widget
XukcFindButton(this_menu, button_value)
UKCMed3MenuWidget this_menu;
short button_value;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcFindButton");
		return (NULL);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot find menu button : no list given");
		return (NULL);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);

	if (butt != NULL)
		return(butt->button_widget);
	else
		return(NULL);
}


/* add an accelerator to a button.  existing accelerators on a button are
 * augmented, so that different keystrokes can invoke the same menu button.
 */
Boolean
XukcAddAcceleratorToMenu(this_menu, button_value, invoke_event, display_string)
UKCMed3MenuWidget this_menu;
short button_value;
String invoke_event;
String display_string;	/* may be NULL */
{
	Buttons butt;
	char new_name[500];
	char new_acc[500];
	String name;
	Arg args[1];

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcAddAcceleratorToMenu");
		return (FALSE);
	}

	if (this_menu == NULL
	    || this_menu->med3Menu.button_list == NULL
	    || invoke_event == NULL) {
		XtWarning("Cannot add accelerator to button .. data missing");
		return (FALSE);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL )
		return (FALSE);

	if (display_string != NULL) {
		XtSetArg(args[0], XtNlabel, &name);
		XtGetValues(butt->button_widget, (ArgList)args, 1);
		(void)sprintf(new_name, "%s [%s]", name, display_string);
		XtSetArg(args[0], XtNlabel, new_name);
		XtSetValues(butt->button_widget, (ArgList)args, 1);
	}

	(void)sprintf(new_acc, acceleratorTranslations, invoke_event);
	XtSetArg(args[0], XtNaccelerators, XtParseAcceleratorTable(new_acc));
	XtSetValues(butt->button_widget, (ArgList)args, 1);

	return (TRUE);
}


void
XukcInstallMenuAccelerators(this_menu, dest_widget)
UKCMed3MenuWidget this_menu;
Widget dest_widget;
{
	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass))
		XtWarning("Non UKCMed3MenuWidget passed to XukcInstallMenuAccelerators");
	XtInstallAllAccelerators(dest_widget, (Widget)this_menu);
}


/* bind an application procedure to a button's callback list along with
 * some data.
 */
Boolean
XukcBindButton(this_menu, button_value, button_proc, client_data)
UKCMed3MenuWidget this_menu;
short button_value;
XtCallbackProc button_proc;
XtPointer client_data;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcBindButton");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot bind procedure to button .. no data given");
		return (FALSE);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	bind_button(butt, button_proc, client_data);
	butt->button_type |= NORMAL_BUTTON;
	return (TRUE);
}


/* Unbind a procedure from a menu button */
Boolean
XukcUnbindButton(this_menu, button_name, button_proc, client_data)
UKCMed3MenuWidget	this_menu;
String button_name;
XtCallbackProc button_proc;
XtPointer client_data;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcUnbindButton");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot unbind procedure from button .. no data given");
		return (FALSE);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_name,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	if (butt->button_type & NORMAL_BUTTON == BUTTON_TYPE_NOT_SET) {
		XtWarning("No binding on button to be unbound");
		return (FALSE);
	}

	unbind_button(butt, button_proc, client_data);
	return (TRUE);
}


Boolean
XukcMenuSetToggleState(this_menu, button_value, new_state, notify)
UKCMed3MenuWidget this_menu;
short button_value;
Boolean new_state;
Boolean notify;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcBindToggle");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot change toggle state .. no data given");
		return (FALSE);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	if (butt->button_type & TOGGLE_BUTTON == BUTTON_TYPE_NOT_SET) {
		XtWarning("Button is not a toggle.");
		return (FALSE);
	}

	set_toggle_state(butt, new_state, notify);
	return (TRUE);
}


/* Change a button into a toggling button with two state strings and
 * a callback that is called with the new buttons boolean state. */
Boolean
XukcBindToggle(this_menu, button_value, on_string, off_string, address,
						button_proc, client_data)
UKCMed3MenuWidget this_menu;
short button_value;
String on_string, off_string;
Boolean *address;
XtCallbackProc button_proc;
XtPointer client_data;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcBindToggle");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot bind procedure to button .. no data given");
		return (FALSE);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	if (butt->button_type & TOGGLE_BUTTON) {
		XtWarning("Button already has a toggle binding.");
		return (FALSE);
	}

	bind_toggle(butt, on_string, off_string, address, button_proc,
								client_data);
	return (TRUE);
}


Boolean
XukcUnbindToggle(this_menu, button_value)
UKCMed3MenuWidget this_menu;
short button_value;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcUnbindToggle");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot unbind procedure to button .. no data given");
		return (FALSE);
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	if (butt->button_type & TOGGLE_BUTTON == BUTTON_TYPE_NOT_SET) {
		XtWarning("Button is not a toggle.");
		return (FALSE);
	}

	unbind_toggle(butt);
	return (TRUE);
}


/* add a force down binding to a button of a static menu */
Boolean
XukcBindForceDown(this_menu, button_value)
UKCMed3MenuWidget this_menu;
short button_value;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcBindForceDown");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot bind force down to button .. no data given");
		return (FALSE);
	}
		
	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);

	if (butt == NULL)
		return (FALSE);

	if (butt->shell_data->shell == NULL) {
		XtWarning("Cannot force down a single level menu");
		return (FALSE);
	}

	if (butt->button_type & FORCE_DOWN_BUTTON) {
		XtWarning("Force down already on button");
		return (FALSE);
	}

#if 0 /* ||| merge this in sometime ||| */
	bind_button(butt, ForceDown, (XtPointer)butt->shell_data->parents_data);
#else
{
	Buttons tmpbtn;
	Boolean already_stripped = FALSE;

	tmpbtn = butt->prev;
	for (; !already_stripped && tmpbtn != NULL; tmpbtn = tmpbtn->prev) {
		if (butt->shell_data == tmpbtn->shell_data) {
			if (butt->button_type & FORCE_DOWN_BUTTON)
				already_stripped = TRUE;
			else
				unbind_button(tmpbtn, ForceDown,
				     (XtPointer)tmpbtn->shell_data->parents_data);
		}
	}
	tmpbtn = butt->next;
	for (; !already_stripped && tmpbtn != NULL; tmpbtn = tmpbtn->next) {
		if (butt->shell_data == tmpbtn->shell_data) {
			if (butt->button_type & FORCE_DOWN_BUTTON)
				already_stripped = TRUE;
			else
				unbind_button(tmpbtn, ForceDown,
				     (XtPointer)tmpbtn->shell_data->parents_data);
		}
	}
}
#endif 0
	butt->button_type |= FORCE_DOWN_BUTTON;
	return (TRUE);
}


/* remove a static menu force down binding from a button */
Boolean
XukcUnbindForceDown(this_menu, button_value)
UKCMed3MenuWidget this_menu;
short button_value;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcUnbindForceDown");
		return (FALSE);
	}

	if (this_menu == NULL || this_menu->med3Menu.button_list == NULL) {
		XtWarning("Cannot unbind a force down from button .. no data given");
		return (FALSE);
	}
		
	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	if (butt->button_type & FORCE_DOWN_BUTTON == BUTTON_TYPE_NOT_SET) {
		XtWarning("No Force down bound to this button");
		return (FALSE);
	}

	unbind_button(butt, ForceDown,
			(XtPointer)butt->shell_data->parents_data);
	butt->button_type &= ~FORCE_DOWN_BUTTON;
	return (TRUE);
}


/* do an XtSetValues on a menu button */
Boolean
XukcSetButtonValues(this_menu, button_value, args, num_args)
UKCMed3MenuWidget this_menu;
short button_value;
ArgList args;
Cardinal num_args;
{
	Buttons butt;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcSetButtonValues");
		return False;
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);

	XtSetValues(butt->button_widget, args, num_args);
	return (TRUE);
}


/* change the sensitivity of a button */
Boolean
XukcChangeButtonStatus(this_menu, button_value, status)
UKCMed3MenuWidget this_menu;
short button_value;
Boolean status;
{
	Arg args[1];

	XtSetArg(args[0], XtNsensitive, status);
	return(XukcSetButtonValues(this_menu, button_value, (ArgList)args, 1));
}


/* change an exisiting popup menu to a dynamically created one */
Boolean
XukcReplaceMenu(this_menu, button_value, new_pane, num_buttons, vertical)
UKCMed3MenuWidget this_menu;
short button_value;
XukcReplaceMenuStruct **new_pane;
Cardinal num_buttons;
Boolean vertical;
{
	Widget tmp;
	Buttons butt;
	Cardinal i;
	Dimension size, other_dimension;
	PopupData pmd;
	int id;

	if (!XtIsSubclass((Widget)this_menu, ukcMed3MenuWidgetClass)) {
		XtWarning("Non UKCMed3MenuWidget passed to XukcReplaceMenu()");
		return False;
	}

	butt = find_button(this_menu->med3Menu.button_list, button_value,
								(Widget)NULL);
	if (butt == NULL)
		return (FALSE);
	if (butt->menu_widget != this_menu)
		XtError("Corruption of UKCMed3MenuWidget detected in XukcReplaceMenu()");

	/* create a temporary widget and use it to find the longest/tallest
	 * string button name */
	tmp = XtVaCreateWidget("xUKCTmp", ukcCommandWidgetClass,
			XtParent(this_menu), XtNresize, TRUE, NULL);
	for (i = 0, size = 0; i < num_buttons; i++) {
		XtVaSetValues(tmp, XtNlabel, new_pane[i]->button_name, NULL);
		if (size < (vertical ? XukcGetWidth(tmp) : XukcGetHeight(tmp)))
			size = vertical ? XukcGetWidth(tmp) :
					  XukcGetHeight(tmp);
	}
	other_dimension = vertical ? XukcGetHeight(tmp) : XukcGetWidth(tmp);
	XtDestroyWidget(tmp);

	/* destroy the old popup menu but keep its associated shell */
	pmd = butt->shell_data;

	if (pmd->shell == NULL) {
		XtWarning("Button given to XukcReplaceMenu() is not in a popup menu");
		return False;
	}

	tmp = XukcGetShellManagedChild(pmd->shell);
	XtDestroyWidget(tmp);

	Mbuttons = this_menu->med3Menu.button_list; /* GLOBAL VARIABLE */
	id = fake_a_menu(new_pane, num_buttons, vertical, size, other_dimension);
	/* ||| this should be done automatically??? ||| */
	/* adjust the size of the menu's shell to contain the new menu */
	other_dimension *= num_buttons;
	XtVaSetValues(pmd->shell,
			XtNwidth, (vertical ? size : other_dimension),
			XtNheight, (vertical ? other_dimension : size),
			XtNallowShellResize, TRUE,
			NULL);
	XtVaSetValues(pmd->shell, XtNallowShellResize, FALSE, NULL);

	(void)create_menu(this_menu, _menu_[id].om_root,
			  pmd->shell, DYNAMIC_POPUP, pmd,
			  vertical, (Widget)NULL, FALSE, 0, 0);
	this_menu->med3Menu.button_list = Mbuttons; /* GLOBAL VARIABLE */
	Mremove(id);
	return (TRUE);
}
