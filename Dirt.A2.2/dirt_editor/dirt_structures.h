/* SCCSID: @(#)dirt_structures.h	1.2 (UKC) 5/17/92 */

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

#include <X11/Xukc/Scrollbar.h>

struct app_resources {
	String display;
	String generated_file;
	String spec_file;
	Pixel grav_colour;
	Pixel background;
	Boolean inches;
	Boolean pixels;
	String templates;
	Position x_grid;
	Position y_grid;
	Boolean grid_off;
	Boolean no_snapping;
	Pixel grid_colour;
	Boolean sort_by_type;
}; 

typedef struct _object_list {
/* creation section */
	Boolean is_resource;
	Boolean in_app;
	Boolean	managed;
	Boolean deferred_creation;
	Widget instance;
	String name;
	Cardinal class;
/* vampire section */
	Boolean is_remote;
	Boolean remote_done;
/* comments and help text */
	String comments;
	String help;
/* resources section */
	char resource_flags;	/* flags controlling locking of resources */
	Position x;
	Position y;
	Dimension width;
	Dimension height;
	Cardinal preset_name_list;
	Cardinal preset_value_list;
	Cardinal resource_name_list;
	Cardinal resource_value_list;
	Cardinal constraint_name_list;
	Cardinal constraint_value_list;
	Cardinal sub_resources_list;
/* selection section */
	Cardinal mark_num;
/* list pointers */
	struct _object_list *children;
	struct _object_list *parent;
	struct _object_list *next;
	struct _object_list *prev;
} ObjectListRec;

typedef ObjectListRec *ObjectPtr;

typedef struct resource_definition {
	String	name;
	String	class;
	String	type;
	char	flags;
} ResDefsRec;

typedef ResDefsRec *ResDefsPtr;

typedef struct widget_class_list {
	WidgetClass	*widget_class;
	ObjectClass	*acceptable_objects;
	Cardinal	num_acceptable;
	String		constructor_string;
	Widget		(*constructor)();
	String		header_dir;
	String		header_file;
	String		displayed_name;
	String		class_name;
	String		description;
	ResDefsPtr	uneditable_resources;
	Cardinal	num_uneditable;
} WidgetDefsRec;

typedef WidgetDefsRec *WidgetDefs;

struct callback_data {
	XrmQuark quark;
	String name;
	XtPointer client_data;
};

struct widget_ref {
	Widget wid;
	XrmQuark res_name;
};

typedef enum _side { left, right, top, bottom } Side;

extern WidgetDefsRec widget_classes[];
extern Cardinal num_on_widget_class_list;

/* resource locking flags */
#define X_RES_LOCKED (1 << 0)
#define Y_RES_LOCKED (1 << 1)
#define WIDTH_RES_LOCKED (1 << 2)
#define HEIGHT_RES_LOCKED (1 << 3)

#define X_GRID_DEFAULT 5
#define Y_GRID_DEFAULT 5

#define TOP_SHELL_CLASS topLevelShellWidgetClass

#define IGNORE 1
#define READ_ONLY 2
#define EDIT_ONLY 4

#define NO_OBJECTS ((ObjectClass *)0)
#define ZERO 0
#define NO_CONSTRUCTOR_STRING ((String)NULL)
#define NO_CONSTRUCTOR (NULL)
#define NO_EXCEPTIONS ((ResDefsPtr)NULL)

#define CLASS_ENTRY(i) (*widget_classes[i].widget_class)
#define CLASS_CONSTRUCTOR_STRING(i) (widget_classes[i].constructor_string)
#define CLASS_CONSTRUCTOR(i) (widget_classes[i].constructor)
#define CLASS_ACCEPTABLE_OBJECTS(i) (widget_classes[i].acceptable_objects)
#define CLASS_NUM_ACCEPTABLE(i) (widget_classes[i].num_acceptable)
#define CLASS_DISPLAYED(i) (widget_classes[i].displayed_name)
#define CLASS_NAME(i) (widget_classes[i].class_name)
#define CLASS_DESC(i) (widget_classes[i].description)
#define CLASS_DIR(i) (widget_classes[i].header_dir)
#define CLASS_HEADER(i) (widget_classes[i].header_file)
#define CLASS_NONEDIT_RES(i) (widget_classes[i].uneditable_resources)
#define CLASS_NUM_NONEDIT(i) (widget_classes[i].num_uneditable)

#define NO_PARENT(cl) \
	(Boolean)((cl->parent == NULL) || (cl->parent == GetHeadObject()) \
		|| IsShellClass(GetClass(cl->class)))

#define STRSIZE 100
#define STRLEN(s) (strlen(s) + 1)

/* ids for the 3 resource lists */
#define APPLICATION_RESOURCES 0
#define NEW_WIDGET_RESOURCES 1
#define INSTANCE_RESOURCES 2

#ifndef SYSV
/* BSD4.3 by default */
#define BSD 1
#endif /* SYSV */

/* file suffixes */
#ifdef SYSV
#	define SPEC_SUFFIX ".ds"
#	define TEMPLATE_SUFFIX ".dt"
#	define CREATE_SUFFIX "_ct.c"
#	define LAYOUT_SUFFIX "_lt.h"
#endif /* SYSV */

#ifdef BSD
#	define SPEC_SUFFIX ".dirt_spec"
#	define TEMPLATE_SUFFIX ".dirt_template"
#	define CREATE_SUFFIX "_create.c"
#	define LAYOUT_SUFFIX "_layout.h"
#endif /* BSD */


/* in remote.c */
extern Boolean ObjectSetValues();
extern Boolean ObjectGetValues();
extern Boolean ObjectConversion();

extern Boolean DoConversion();
extern Boolean DoObjectConversion();

/* in dirt.c */
extern void GiveMessage();
extern void GiveWarning();
extern void GiveError();

/* ||| Taken from Xt/IntrinsicI.h ||| */

#undef XtBCopy()
#undef XtBZero()
#undef XtBCmp()

/* UNALIGNED will be defined in the Makefile for BIGENDIAN machines only */

#ifdef UNALIGNED

#define XtBCopy(src, dst, size)                        \
	if (size == sizeof(int))                       \
		*((int *) (dst)) = *((int *) (src));   \
	else if (size == sizeof(char))                 \
		*((char *) (dst)) = *((char *) (src)); \
	else if (size == sizeof(short))                 \
		*((short *) (dst)) = *((short *) (src)); \
	else                                                    \
		bcopy((char *) (src), (char *) (dst), (int) (size));

#define XtBZero(dst, size) \
	if (size == sizeof(int)) \
		*((int *) (dst)) = 0; \
	else \
		bzero((char *) (dst), (int) (size));

#define XtBCmp(b1, b2, size) \
	(size == sizeof(int) ? 				\
	*((int *) (b1)) != *((int *) (b2))		\
	:   bcmp((char *) (b1), (char *) (b2), (int) (size)) )

#else

#define XtBCopy(src, dst, size)         \
	bcopy((char *) (src), (char *) (dst), (int) (size));

#define XtBZero(dst, size) bzero((char *) (dst), (int) (size));

#define XtBCmp(b1, b2, size) bcmp((char *) (b1), (char *) (b2), (int) (size))

#endif
