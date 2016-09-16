#ifndef lint
static char sccsid[] = "@(#)load.c	1.4 (UKC) 5/18/92";
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

/* load.c - parses and loads the UI specification file.  To start the parse
 *          call ProcessFile(fd) with the open file descriptor.  TRUE is
 *          returned if NO_ERROR's were found.   The syntax and semantics
 *          of the UI Spec. file is given in a separate document.
 */
#define LOG_SUB 1
#undef LOG_SUB

#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xukc/text.h>
#include <X11/Xukc/Layout.h>
#include "dirt_structures.h"

struct _missing_resources {
	String name;
	String value;
	ObjectPtr object;
	Cardinal line_number;
};

/* size of temporary string buffer */
#define BUF_SIZE 10000

/* error states returned from Find functions */
#define NO_ERROR		(1<<0)
#define CAUGHT_EOF		(1<<1)
#define CAUGHT_ERROR		(1<<2)
#define CONTINUE_AFTER_ERROR	(1<<3)

extern Widget WI_dirt;
extern Widget WI_create_name_entry;
extern Widget WI_create_class_entry;
extern Widget WI_busy_window;
extern char app_class[];
extern struct app_resources editor_resources;

/* externally defined routines */
extern ObjectPtr SearchRecordsForComplexName();
extern ObjectPtr CreateObjectRecord();
extern ObjectPtr GetHeadObject();
extern ObjectPtr GetStartOfList();
extern ObjectPtr CreateTemplateHead();
extern void ChangeStartObjectRealization();
extern void AddCreatedObject();
extern void AddDestroyCallback();
extern void DestroyObjectRecord();
extern Cardinal GetClassFromName();
extern void AddAppEventHandlers();
extern void AddEventHandlers();
extern void MergeInternalDatabase();
extern WidgetClass GetClass();
extern Boolean IsShellClass();
extern Boolean IsCompositeClass();
extern XtResourceList GetAResource();
extern void CreateResourceTable();
extern void StoreConnection();
extern void RegisterName();
extern void ClearRegisteredNames();
extern void DisplayRegisteredNames();
extern XtAppContext GetUIContext();
extern void DrainUIEventQueue();
extern Boolean ConversionExists();
extern Cardinal FindClass();
extern void RealizeAllChildren();
extern void CreateAndRealize();
extern void AddHashedName();
extern void LogResourceSetting();
extern void RememberWidgetIdUse();

/* defined in Xukc library */
extern String XukcGrowString();
extern Boolean XukcParseMeasurementString();
extern String XukcCreateMeasurementString();
extern void XukcCopyToArgVal();
extern Cardinal XukcAddObjectToList();

static Cardinal line_number = 1;  /* current line number for error purposes */
static int current_char = -1;	/* current input charatcer (look-ahead) */
static char big_buffer[BUF_SIZE]; /* a buffer! */
static Arg args[1000];		/* arguments for the currently read in widget */
static Boolean free_value[1000];/* args[i].value can be freed? */
static Boolean is_widget_type[1000]; /* args[i].value is a widget id */
static Cardinal num_args = 0;	/* number of arguments in args[] */
static Arg presets[500];	/* arguments to set before creating widget */
static Boolean free_preset[500];/* preset[i].value can be freed? */
static Cardinal num_presets = 0;/* number of arguments in preset[] */
static struct _missing_resources unset_resources[1000];
static Cardinal num_unset = 0;
#ifdef LOG_SUB /* ||| */
static Cardinal sub_resources_list = 0;
static Cardinal sub_values_list = 0;
#endif /* ||| */
static Cardinal error_return = NO_ERROR;
static ObjectPtr new_start = NULL;
static ObjectListRec new_head;
static String comments = NULL;

/* temporary holding variables whilst reading in widget description */
static String tmpstr;		/* temporary holding variable */
static String class;		/* widget class */
static String res_name;		/* name of a resource to set */
static String res_value;	/* value of a resource to set */
static ObjectPtr new_object;	/* the newly created widget */
static String layout_name;
static String w1_name;
static String w2_name;
static String direct_name;

/* new widget definition completion flags */
static int widget_definition_state = NULL;
#define NAME (1 << 0)
#define CLASS (1 << 1)
#define PARENT (1 << 2)
#define MANAGED (1 << 3)
#define DEFERRED (1 << 4)
#define COMPLETED (NAME | CLASS | PARENT)
#define GOT_APP (NAME | CLASS)
#define IN_COMMAND (1 << 5)
#define IN_LINE (1 << 6)
#define APP_DEFINED (1 << 7)
#define CHILD_DEFINED (1 << 8)
#define TEMPLATE_DEFINED (1 << 9)
#define EOF_ERROR (IN_COMMAND | IN_LINE)
#define CHECK_FOR_APP_DETAILS() { \
	if (!CHECK_STATE(APP_DEFINED)) \
		return (parse_error(fd, "Application details missing", TRUE)); \
}
#define CHECK_FOR_MISSING_APP_NAME() { \
	if (!CHECK_STATE(NAME)) \
		return (parse_error(fd, "Application name missing", TRUE)); \
}
#define CHECK_FOR_MISSING_NAME() { \
	if (!CHECK_STATE(NAME)) \
		return (parse_error(fd, "Widget name missing", TRUE)); \
}
#define CHECK_FOR_REDEFINITION(b, m) { \
	if (CHECK_STATE(b)) { \
		char red_tmp[200]; \
		(void)sprintf(red_tmp, "redefinition of %s", m); \
		return (parse_error(fd, red_tmp, FALSE)); \
	} \
}
#define WIDGET_DEFINED() (CHECK_STATE(COMPLETED) == COMPLETED)

#define SET_STATE(b) widget_definition_state |= (b)
#define RESET_STATE(b) widget_definition_state &= ~(b)
#define CHECK_STATE(b) (widget_definition_state & (b))

#define SEPARATOR ':'

/* available commands .. given as "%A contents_of_line\n" */
static String commands = "%ACRHFNcpmoszrdhfLYZT";
#define COMMENT 0
#define APP_NAME 1
#define APP_CLASS 2
#define APP_RES 3
#define APP_HELP 4
#define APP_COMPLETED 5
#define WIDGET_NAME 6
#define WIDGET_CLASS 7
#define WIDGET_PARENT 8
#define WIDGET_MANAGED 9
#define WIDGET_DEFERRED 10
#define WIDGET_SETTING 11
#define WIDGET_CONSTRAINT 12
#define WIDGET_RES 13
#define WIDGET_RDB 14
#define WIDGET_HELP 15
#define WIDGET_COMPLETED 16
#define LAYOUT_CONN 17
#define ACTION_NAME 18
#define CALLBACK_NAME 19
#define TEMPLATE_NAME 20

#define NEXTCHAR(fd) fgetc(fd)
#define FOUND_EOF (current_char == EOF)

#define PROCESS_ERROR(command) { \
	int ret_err = (command); \
	if (ret_err != NO_ERROR) \
		return (ret_err); \
}

#define RELEASE_STORAGE(v) { XtFree((char *)v); v = NULL; }

static Cardinal FindNewline();


static void
release_argument_storage(a, b, c)
ArgList a;
Cardinal b;
Boolean *c;
{
	Cardinal i;

	for (i = 0; i < b; i++) {
		XtFree((char *)a[i].name);
		if (c[i])
			XtFree((char *)a[i].value);
	}
}


static Cardinal
parse_error(fd, message, fatal)
FILE *fd;
String message;
Boolean fatal;
{
	GiveWarning(WI_dirt, "badSpecification", "specificationLoad",
			"Error during load (line %d): %s", 2,
			line_number, message);
	if (!fatal)
		return (FindNewline(fd, TRUE));
	else
		return (CAUGHT_ERROR);
}


static Cardinal 
process_eof_state(fd)
FILE *fd;
{
	if (CHECK_STATE(EOF_ERROR))
		return (parse_error(fd, "caught End-Of-File in middle of line",
								TRUE));
	if (!CHECK_STATE(COMPLETED))
		return (CAUGHT_EOF);

	return (parse_error(fd, "incomplete widget definition at EOF", TRUE));
}


static void
call_create_widget()
{
	int i;

	new_object->is_remote = FALSE;
	new_object->in_app = new_object->parent->in_app;
	if (comments == NULL)
		new_object->comments = XtNewString("");
	else
		new_object->comments = comments;
	CreateAndRealize(new_object, new_object->parent,
			 presets, num_presets, FALSE);
	AddEventHandlers(new_object);
	AddCreatedObject(new_object);
	AddDestroyCallback(new_object);
	AddHashedName(new_object);
	for (i = 0; i < num_presets; i++) {
		if (is_widget_type[i])
			RememberWidgetIdUse(new_object->instance,
					    presets[i].name,
					    (Widget)presets[i].value);
		is_widget_type[0] = FALSE;
	}
	release_argument_storage(presets, num_presets, free_preset);
	num_presets = 0;
	comments = NULL;
}


static void
call_create_application()
{
	/* unrealize any exiting windows so we can add realize the new
	 * ones in one go.
	 */
	ChangeStartObjectRealization(FALSE);

	new_object->instance = XtAppCreateShell(new_object->name, class,
				TOP_SHELL_CLASS,
				XtDisplay(GetHeadObject()->instance),
				(ArgList)presets, num_presets);
	new_object->is_remote = FALSE;
	new_object->in_app = TRUE;
	if (comments == NULL)
		new_object->comments = XtNewString("");
	else
		new_object->comments = comments;
	CreateResourceTable(APPLICATION_RESOURCES, new_object, 0, FALSE);
	AddAppEventHandlers(new_object);
	AddDestroyCallback(new_object);
	AddHashedName(new_object);
	XukcNewTextWidgetString(WI_create_name_entry, new_object->name);
	XukcNewTextWidgetString(WI_create_class_entry, class);
	(void)strcpy(app_class, class); /* ||| is this still necessary? ||| */
	RELEASE_STORAGE(class);
	release_argument_storage(presets, num_presets, free_preset);
	new_start = new_object;

	/* need a pseudo head of tree for searching */
	new_head.children = new_start;

	num_presets = 0;
	new_object = NULL;
	comments = NULL;
}


static void
call_create_template(name)
String name;
{
	/* create an object record that contains the template_building_area
	 * instance id.  Use this as a root for the real objects to be
	 * read in.
	 */
	new_start = CreateTemplateHead(name);

	if (comments == NULL)
		new_start->comments = XtNewString("");
	else
		new_start->comments = comments;

	/* need a pseudo head of tree for searching */
	new_head.children = new_start;

	num_presets = 0;
	new_object = NULL;
	comments = NULL;
}


static void
save_comment(str)
String str;
{
	/* strip leading whitespace */
	while (*str == ' ')
		str++;

	comments = XukcGrowString(comments, (Cardinal)strlen(str) + 2);
	(void)strcat(comments, str);
	(void)strcat(comments, "\n");
}


static String
cvt_to_newlines(str)
String str;
{
	String outstr;
	Cardinal i;
	Boolean found_escape;

	outstr = XukcGrowString(NULL, (Cardinal)strlen(str) + 1);

	for (i = 0, found_escape = FALSE; *str; str++) {
		outstr[i] = *str;
		if (found_escape) {
			found_escape = FALSE;
			if (outstr[i] == 'n') /* found \n */
				outstr[i-1] = '\n';
			else
				i++;
		} else {
			if (outstr[i] == '\\')
				found_escape = TRUE;
			i++;
		}
	}
	outstr[i] = '\0';
	return (outstr);
}


static String
change_measurement_resources(res, value)
XtResourceList res;
String value;
{
	int pixels;

	if (strcmp(res->resource_type, XtRPosition) != 0 &&
	    strcmp(res->resource_type, XtRDimension) != 0) 
		return (NULL); /* not the type we are looking for */

	/* convert the value into an absolute pixel measurement for this
	 * display. Then produce a string containing either the number of
	 * pixels or the size of this measurement in millimeters or inches.
	 * This new string is returned.
	 */
	if (!XukcParseMeasurementString(
	    XtScreen(GetHeadObject()->instance), value, &pixels,
	    strcmp(res->resource_type, XtRDimension)))
		/* an error in the resource, ignore it, the error will
		 * be caught and displayed later. */
		return (NULL);

	if (editor_resources.pixels) {
		/* convert all measurements into pixels */
		char pixel_str[100];

		(void)sprintf(pixel_str, "%d", pixels);
		value = XtNewString(pixel_str);
	} else {
		/* convert all measurements into millimeters or inches */
		String mm_str;

		mm_str = XukcCreateMeasurementString(
				XtScreen(GetHeadObject()->instance), pixels,
				!editor_resources.inches);

		value = XtNewString(mm_str);
	}
	return (value);
}


static Boolean
strip_locked_resources(obj, res, value)
ObjectPtr obj;
XtResourceList res;
String value;
{
	/* remove X, Y, WIDTH and HEIGHT resources and store these as
	 * our "locked" resources.  These can be edited using the little
	 * locks next to their value entries in the Edit dialog.
	 */
	String ret;
	int val;

	if ((ret = change_measurement_resources(res, value)) == NULL)
		return FALSE;

	val = atoi(ret);
	XtFree(ret);

	is_widget_type[num_presets] = FALSE;

	if (strcmp(res->resource_name, XtNx) == 0) {
		obj->resource_flags |= X_RES_LOCKED;
		obj->x = (Position)val;
		XtSetArg(presets[num_presets], XtNewString(XtNx), obj->x);
		free_preset[num_presets] = FALSE;
		num_presets++;
		return TRUE;
	} else if (strcmp(res->resource_name, XtNy) == 0) {
		obj->resource_flags |= Y_RES_LOCKED;
		obj->y = (Position)val;
		XtSetArg(presets[num_presets], XtNewString(XtNy), obj->y);
		free_preset[num_presets] = FALSE;
		num_presets++;
		return TRUE;
	} else if (strcmp(res->resource_name, XtNwidth) == 0) {
		obj->resource_flags |= WIDTH_RES_LOCKED;
		obj->width = (Dimension)val;
		XtSetArg(presets[num_presets], XtNewString(XtNwidth), obj->width);
		free_preset[num_presets] = FALSE;
		num_presets++;
		return TRUE;
	} else if (strcmp(res->resource_name, XtNheight) == 0) {
		obj->resource_flags |= HEIGHT_RES_LOCKED;
		obj->height = (Dimension)val;
		XtSetArg(presets[num_presets], XtNewString(XtNheight), obj->height);
		free_preset[num_presets] = FALSE;
		num_presets++;
		return TRUE;
	}
	/* this is not the Dimension/Position we are looking for */
	return FALSE;
}


static Cardinal
set_a_resource(fd, cl, name, value, is_preset)
FILE *fd;
ObjectPtr cl;
String name, value;
Boolean is_preset;
{
	XrmValue from, to;
	int resource_number;
	XtResourceList list;
	Boolean is_constraint;
	String new_value;
	
	list = GetAResource(cl, name, isupper(name[0]), &resource_number,
							&is_constraint);
	if (resource_number < 0) {
		XtFree((char *)list);
		return (parse_error(fd, "unable to set resource: no such name",
									FALSE));
	}

	if (!ConversionExists(list[resource_number].resource_type)) {
		XtFree((char *)list);
		return (parse_error(fd, "unable to set resource: unknown type",
									FALSE));
	}

	if (strip_locked_resources(cl, list + resource_number, value))
		return (NO_ERROR);
		
	/* update all Position and Dimension type resources to Pixels or
	 * millimeters/inches, depending upon the options giving to dirt
	 */
	new_value = NULL;
	new_value = change_measurement_resources(list + resource_number, value);

	if (new_value != NULL) {
		from.addr = cvt_to_newlines(new_value);
		XtFree(new_value);
	} else
		from.addr = cvt_to_newlines(value);
	value = from.addr;
	from.size = STRLEN(from.addr);
	to.size = 0;
	to.addr = NULL;

	if (is_preset) {
		is_widget_type[num_presets] = FALSE;

		if (strcmp(list[resource_number].resource_type, XtRString) != 0) {
#if 0
			if (*value == '\0' &&
			    (strcmp(list[resource_number].resource_type,
			    XtRTranslationTable) != 0)) {
				/* illegal blank value */
				XtFree((char *)list);
				return (parse_error(fd, "Missing resource value", FALSE));
			}
#endif
				
			if (DoConversion(cl->parent->instance,
					XtRString, &from,
					list[resource_number].resource_type,
					&to)) {
				presets[num_presets].name =
				    XtNewString(list[resource_number].resource_name);
				XukcCopyToArgVal(to.addr,
					&(presets[num_presets].value),
					list[resource_number].resource_size);
				free_preset[num_presets] = FALSE;

				/* Check for Widget type resources and
				 * save their use.
				 */ 
				if (strcmp(list[resource_number].resource_type,
				    XtRWidget) == 0)
					is_widget_type[num_presets] = TRUE;

				num_presets++;
			} else {
				XtFree((char *)list);
				/* try the conversion again later */
				return (CONTINUE_AFTER_ERROR);
			}
		} else {
			free_preset[num_presets] = TRUE;
			XtSetArg(presets[num_presets], XtNewString(name), XtNewString(value));
			num_presets++;
		}
	} else {
		if (strcmp(list[resource_number].resource_type, XtRString) != 0) {
#if 0
			if (*value == '\0' &&
			    (strcmp(list[resource_number].resource_type,
			    XtRTranslationTable) != 0)) {
				/* illegal blank value */
				XtFree((char *)list);
				return (parse_error(fd, "Missing resource value", FALSE));
			}
#endif

			/* kludge: Callbacks are special and should not
			 *         simply be overwritten but should be merged
			 *         in using XtAddCallback()
			 */
			if (DoConversion(cl->instance, XtRString, &from,
					list[resource_number].resource_type,
					&to)) {
			    if (strcmp(list[resource_number].resource_type,
				XtRCallback) == 0) {
				XtCallbackList cb = *(XtCallbackList *)to.addr;

				for (; cb && cb->callback != NULL; cb++)
				    XtAddCallback(cl->instance,
					list[resource_number].resource_name,
					cb->callback, cb->closure);
			    } else {
				args[num_args].name = XtNewString(
					list[resource_number].resource_name);
				XukcCopyToArgVal(to.addr,
					&(args[num_args].value),
					list[resource_number].resource_size);
				free_value[num_args] = FALSE;
				num_args++;

				/* Check for Widget type resources and
				 * save their use.
				 */ 
				if (strcmp(list[resource_number].resource_type,
				    XtRWidget) == 0)
					RememberWidgetIdUse(cl->instance,
					    list[resource_number].resource_name,
					    *(Widget *)to.addr);
			    }
			} else {
				XtFree((char *)list);
				/* try the conversion again later */
				return (CONTINUE_AFTER_ERROR);
			}
		} else {
			free_value[num_args] = TRUE;
			XtSetArg(args[num_args], XtNewString(name), XtNewString(value));
			num_args++;
		}
	}
	XtFree((char *)list);
	LogResourceSetting(cl, name, value, is_constraint);
	return (NO_ERROR);
}


static Cardinal
try_unset_resources_again(fd)
FILE *fd;
{
	Cardinal i, e;

	for (i = 0; i < num_unset; i++) {
		num_args = 0;
		line_number = unset_resources[i].line_number;
		e = set_a_resource(fd, unset_resources[i].object,
					unset_resources[i].name,
					unset_resources[i].value,
					FALSE);
		if (e != NO_ERROR)
			return (parse_error(fd, "unable to set resource (conversion failed)", TRUE));
		RELEASE_STORAGE(args[num_args].name);
		if (free_value[num_args])
			RELEASE_STORAGE(args[num_args].value);
	}

	return (CAUGHT_EOF);
}


static void
SkipWhiteSpace(fd)
FILE *fd;
{
	while (current_char == ' ' || current_char == '\t')
		current_char = NEXTCHAR(fd);
}


static Cardinal
FindNewline(fd, skip)
FILE *fd;
Boolean skip;
{
	if (skip)
		while (current_char != EOF && current_char != '\n')
			current_char = NEXTCHAR(fd);
	else
		SkipWhiteSpace(fd);

	if (current_char != '\n') {
		if (current_char == EOF)
			return (process_eof_state(fd));
		else
			return (parse_error(fd, "found character instead of newline", FALSE));
	}
	line_number++;
	current_char = NEXTCHAR(fd);
	return (skip ? CONTINUE_AFTER_ERROR : NO_ERROR);
}


static Cardinal
FindAndDiscardSeparator(fd, sep)
FILE *fd;
char sep;
{
	SkipWhiteSpace(fd);
	if (current_char != sep)
		return (parse_error(fd, "Missing separator", FALSE));
	current_char = NEXTCHAR(fd);
	SkipWhiteSpace(fd);
	return (NO_ERROR);
}


/* find an identifier string: [a-zA-Z0-9_] 
 * RETURNS: error code and string in str_ret which has been malloc'ed */
static Cardinal
GetIdentifierString(fd, str_ret)
FILE *fd;
String *str_ret;
{
	String tmp = big_buffer;

	while (current_char != EOF && (isalpha(current_char)
		|| isdigit(current_char) || current_char == '_')) {
		*tmp = current_char;
		current_char = NEXTCHAR(fd);
		tmp++;
	};
	*tmp = '\0';
	*str_ret = XtNewString(big_buffer);

	if (current_char == EOF)
		return (CAUGHT_EOF);

	if (tmp == big_buffer)
		return (parse_error(fd, "Missing identifier string", FALSE));

	return (NO_ERROR);
}


/* find a string that is terminated by whitepace : [^ \r\l\n\t EOF]
 * RETURNS: error code and string in str_ret which has been malloc'ed */
static Cardinal
GetWhiteSpaceString(fd, str_ret)
FILE *fd;
String *str_ret;
{
	String tmp = big_buffer;

	while (current_char != EOF && !isspace(current_char)) {
		*tmp = current_char;
		current_char = NEXTCHAR(fd);
		tmp++;
	};
	*tmp = '\0';
	*str_ret = XtNewString(big_buffer);

	if (current_char == EOF)
		return (CAUGHT_EOF);

	if (tmp == big_buffer)
		return (parse_error(fd, "Missing string", FALSE));

	return (NO_ERROR);
}


/* read in the whole line and store it in the buffer */
static Cardinal
GetLine(fd, str_ret, allow_blanks)
FILE *fd;
String *str_ret;
Boolean allow_blanks;
{
	String tmp = big_buffer;
	Boolean found_escape = FALSE;

	while (current_char != EOF && (found_escape || current_char != '\n')) {
		*tmp = current_char;

		if (found_escape) {
			found_escape = FALSE;
			if (current_char ==  '\n')
				/* found an embedded \n ? */
				*(tmp-1) = '\n';
			else
				tmp++;
		} else {
			if (current_char == '\\')
				found_escape = TRUE;
			tmp++;
		}
		current_char = NEXTCHAR(fd);
	};
	*tmp = '\0';
	*str_ret = XtNewString(big_buffer);

	if (*big_buffer == '\0' && !allow_blanks)
		return (parse_error(fd, "illegal blank line", FALSE));

	RESET_STATE(IN_COMMAND | IN_LINE);
	return (FindNewline(fd, FALSE));
}


static Cardinal
find_command_start(fd, comm_ret)
FILE *fd;
Cardinal *comm_ret;
{
	String tmp;

	if (current_char != '%')
		return (parse_error(fd, "Missing % at beginning of line", FALSE));
	SET_STATE(IN_LINE);
	current_char = NEXTCHAR(fd);
	RESET_STATE(IN_COMMAND);
	tmp = index(commands, current_char);
	if (tmp == NULL)
		return (parse_error(fd, "Missing or illegal command after % .. skipping line", FALSE));
	*comm_ret = tmp - commands;
	SET_STATE(IN_COMMAND);
	current_char = NEXTCHAR(fd);
	return (NO_ERROR);
}

static Cardinal
add_connection(fd, layout_parent, from_name, to_name, dir_name)
FILE *fd;
String layout_parent, from_name, to_name, dir_name;
{
	static XrmQuark horiz_before = NULL;
	static XrmQuark horiz_after = NULL;
	static XrmQuark vert_above = NULL;
	static XrmQuark vert_below = NULL;
	XrmQuark cmp_quark;
	ObjectPtr parent, from, to;
	Positioning direction;

	parent = SearchRecordsForComplexName(&new_head, layout_parent);
	if (parent == NULL) {
		char name[100];
		return(parse_error(fd, sprintf(name, "Unknown UKCLayout parent \"%s\"", layout_parent), FALSE));
	}
	if (!XtIsSubclass(parent->instance, ukcLayoutWidgetClass))
		return(parse_error(fd, "Parent not a subclass of UKCLayout", FALSE));

	from = SearchRecordsForComplexName(&new_head, from_name);
	if (from == NULL) {
		char name[100];
		return(parse_error(fd, sprintf(name, "Unknown widget \"%s\"", from_name), FALSE));
	}

	to = SearchRecordsForComplexName(&new_head, to_name);
	if (to == NULL) {
		char name[100];
		return(parse_error(fd, sprintf(name, "Unknown widget \"%s\"", to_name), FALSE));
	}

	if (horiz_before == NULL) { /* do this only once */
		horiz_before = XrmStringToQuark("horizontal_before");
		horiz_after = XrmStringToQuark("horizontal_after");
		vert_above = XrmStringToQuark("vertical_above");
		vert_below = XrmStringToQuark("vertical_below");
	}

	cmp_quark = XrmStringToQuark(dir_name);
	if (cmp_quark == horiz_before)
		direction = horizontal_before;
	else if (cmp_quark == horiz_after)
		direction = horizontal_after;
	else if (cmp_quark == vert_above)
		direction = vertical_above;
	else if (cmp_quark == vert_below)
		direction = vertical_below;

	StoreConnection(parent, from, to, direction);
	return (NO_ERROR);
}

static Cardinal
process_command_line(fd)
FILE *fd;
{
	int err;
	Cardinal command;
	String tmp;
	XrmDatabase rdb = NULL;

	if FOUND_EOF
		return (process_eof_state(fd));

	if ((err = find_command_start(fd, &command)) & ~NO_ERROR) {
		/* found an error or eof */
		switch (err) {
			case CONTINUE_AFTER_ERROR :
				break;
			case CAUGHT_EOF : /* read End-Of-File */
				return (CAUGHT_EOF);
			case CAUGHT_ERROR : /* fatal load error */
				return (CAUGHT_ERROR);
				/* NOTREACHED */
			default :
				XtError("unknown error in process_command_line()");
				/* NOTREACHED */
				break;
		}
	} else { /* found the start of the command ok */
		switch (command) {
			case COMMENT : /* %% */
				PROCESS_ERROR(GetLine(fd, &tmp, FALSE));
				/* filler to be stripped */
				if (strcmp(tmp, "%") == 0 ||
				    strncmp(tmp, "ID ", 3) == 0) {
					RELEASE_STORAGE(tmp);
				} else {
					/* save the comment */
					save_comment(tmp); 
					RELEASE_STORAGE(tmp);
				}
				break;
			case APP_NAME : /* %A */
				if (CHECK_STATE(COMPLETED))
					return (parse_error(fd, "application name defined in wrong place", TRUE));
				if (CHECK_STATE(APP_DEFINED))
					return (parse_error(fd, "redefinition of application details", FALSE));
				RESET_STATE(COMPLETED);
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &tmpstr));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				SET_STATE(NAME);
				new_object = CreateObjectRecord();
				new_object->parent = GetHeadObject();
				new_object->name = XtNewString(tmpstr);
				num_args = 0;
				RELEASE_STORAGE(tmpstr);
				break;
			case TEMPLATE_NAME : /* %T */
				if (CHECK_STATE(COMPLETED))
					return (parse_error(fd, "template name defined in wrong place", TRUE));
				if (CHECK_STATE(APP_DEFINED | TEMPLATE_DEFINED))
					return (parse_error(fd, "redefinition of template details", FALSE));
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &tmpstr));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				SET_STATE(COMPLETED);
				SET_STATE(APP_DEFINED);
				SET_STATE(TEMPLATE_DEFINED);
				call_create_template(tmpstr);
				num_args = 0;
				RELEASE_STORAGE(tmpstr);
				break;
			case APP_CLASS : /* %C */
				CHECK_FOR_MISSING_APP_NAME();
				if (CHECK_STATE(CLASS))
					return (parse_error(fd, "redefinition of application class", FALSE));
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &class));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				SET_STATE(CLASS);
				if (CHECK_STATE(GOT_APP) == GOT_APP) {
					SET_STATE(APP_DEFINED);
					SET_STATE(COMPLETED);
				}
				new_object->class =
					FindClass(applicationShellWidgetClass);
				break;
			case APP_RES : /* %R */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_APP_NAME();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &res_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, SEPARATOR));
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));
				line_number--;
				error_return = set_a_resource(fd, new_object,
								res_name,
								res_value,
								TRUE);
				if (error_return != CONTINUE_AFTER_ERROR) {
					PROCESS_ERROR(error_return);
					RELEASE_STORAGE(res_name);
					RELEASE_STORAGE(res_value);
				} else {
					/* the resource conversion failed
					 * so save it and try it again after
					 * all the widgets have been created.
					 */
					unset_resources[num_unset].name =
								res_name;
					unset_resources[num_unset].value =
								res_value;
					unset_resources[num_unset].object =
								new_object;
					unset_resources[num_unset].line_number =
								line_number;
					num_unset++;
				}
				line_number++;
				break;
			case APP_HELP : /* %H */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_APP_NAME();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));
				new_object->help = res_value;
				break;
			case APP_COMPLETED : /* %F */
				if (!CHECK_STATE(APP_DEFINED))
					return (parse_error(fd, "application details incompletely defined", TRUE));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				RESET_STATE(COMPLETED);
				if (!CHECK_STATE(TEMPLATE_DEFINED))
					call_create_application();
				break;
			case WIDGET_NAME : /* %N */
				CHECK_FOR_APP_DETAILS();
				if (CHECK_STATE(COMPLETED) != NULL)
					return (parse_error(fd, "Missing %f for previous widget definition", TRUE));
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &tmpstr));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				SET_STATE(NAME);
				new_object = CreateObjectRecord();
				new_object->name = XtNewString(tmpstr);
				num_args = 0;
				RELEASE_STORAGE(tmpstr);
				break;
			case WIDGET_CLASS : /* %c */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_REDEFINITION(CLASS, "widget class");
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &tmpstr));
				new_object->class = GetClassFromName(tmpstr);
				RELEASE_STORAGE(tmpstr);
				if (new_object->class == 0)
					return(parse_error(fd, "unknown widget class", TRUE));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				SET_STATE(CLASS);
				break;
			case WIDGET_PARENT : /* %p */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				CHECK_FOR_REDEFINITION(PARENT, "widget's parent");
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetWhiteSpaceString(fd, &tmpstr));
				new_object->parent =
					SearchRecordsForComplexName(&new_head,
								tmpstr);
				if (new_object->parent == NULL)
					return(parse_error(fd, "Unable to find parent", TRUE));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				SET_STATE(PARENT);
				RELEASE_STORAGE(tmpstr);
				break;
			case WIDGET_MANAGED : /* %m */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				CHECK_FOR_REDEFINITION(MANAGED, "widget's managed line");
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &tmpstr));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				if (strcmp(tmpstr, "true") == 0)
					new_object->managed = TRUE;
				SET_STATE(MANAGED);
				RELEASE_STORAGE(tmpstr);
				break;
			case WIDGET_DEFERRED : /* %o */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				CHECK_FOR_REDEFINITION(DEFERRED, "widget's deferred creation line");
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &tmpstr));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				if (strcmp(tmpstr, "true") == 0)
					new_object->deferred_creation = TRUE;
				SET_STATE(DEFERRED);
				RELEASE_STORAGE(tmpstr);
				break;
			case WIDGET_HELP : /* %h */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_APP_NAME();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));
				new_object->help = cvt_to_newlines(res_value);
				RELEASE_STORAGE(res_value);
				break;
			case WIDGET_RDB : /* %d */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				SkipWhiteSpace(fd);
#if defined(XlibSpecificationRelease) && XlibSpecificationRelease > 4
				rdb = XtScreenDatabase(XtScreen(
						GetHeadObject()->instance));
#else
				rdb = XtDisplay(GetHeadObject()->instance)->db;
#endif /* X11R4/R5 */

#ifdef LOG_SUB /* ||| */
				PROCESS_ERROR(GetWhiteSpaceString(fd, &res_name));
				if (res_name[strlen(res_name)-1] == ':')
					res_name[strlen(res_name)-1] = '\0';
				else
					PROCESS_ERROR(FindAndDiscardSeparator(fd, ':'));
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));
				XrmPutStringResource(&rdb), res_name, res_value);
				(void)XukcAddObjectToList(&sub_resources_list,
					res_name, FALSE);
				(void)XukcAddObjectToList(&sub_values_list,
					res_value, FALSE);
				res_value = NULL;
#else
				PROCESS_ERROR(GetLine(fd, &res_name, FALSE));
				XrmPutLineResource(&rdb, res_name);
				(void)XukcAddObjectToList(
					&new_object->sub_resources_list,
					res_name, FALSE);
#endif /* ||| */
				res_name = NULL;
				if (new_object->instance != NULL) {
					line_number--;
					return (parse_error(fd, "%d line must come before any %r lines", TRUE));
				}
				break;
			case WIDGET_RES : /* %r */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &res_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, SEPARATOR));
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));

				line_number--;
				if (new_object->instance == NULL) {
					if (CHECK_STATE(COMPLETED) != COMPLETED)
						return (parse_error(fd, "widget definition incomplete", TRUE));
					call_create_widget();
				}

				error_return = set_a_resource(fd, new_object,
								res_name,
								res_value,
								FALSE);
				if (error_return != CONTINUE_AFTER_ERROR) {
					PROCESS_ERROR(error_return);
					RELEASE_STORAGE(res_name);
					RELEASE_STORAGE(res_value);
				} else {
					/* the resource conversion failed
					 * so save it and try it again after
					 * all the widgets have been created.
					 */
					unset_resources[num_unset].name =
								res_name;
					unset_resources[num_unset].value =
								res_value;
					unset_resources[num_unset].object =
								new_object;
					unset_resources[num_unset].line_number =
								line_number;
					num_unset++;
				}
				line_number++;
				break;
			case WIDGET_SETTING : /* %s */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &res_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, SEPARATOR));
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));
				line_number--;
				PROCESS_ERROR(set_a_resource(fd, new_object, res_name, res_value, TRUE));
				line_number++;
				RELEASE_STORAGE(res_name);
				RELEASE_STORAGE(res_value);
				if (new_object->instance != NULL) {
					line_number--;
					return (parse_error(fd, "%s line must come before any %r lines", TRUE));
				}
				break;
			case WIDGET_CONSTRAINT : /* %z */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &res_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, SEPARATOR));
				PROCESS_ERROR(GetLine(fd, &res_value, TRUE));
				line_number--;
				error_return = set_a_resource(fd, new_object,
								res_name,
								res_value,
								TRUE);
				if (error_return != CONTINUE_AFTER_ERROR) {
					PROCESS_ERROR(error_return);
					RELEASE_STORAGE(res_name);
					RELEASE_STORAGE(res_value);
				} else {
					/* the resource conversion failed
					 * so save it and try it again after
					 * all the widgets have been created.
					 */
					unset_resources[num_unset].name =
								res_name;
					unset_resources[num_unset].value =
								res_value;
					unset_resources[num_unset].object =
								new_object;
					unset_resources[num_unset].line_number =
								line_number;
					num_unset++;
				}
				line_number++;
				break;
			case WIDGET_COMPLETED : /* %f */
				CHECK_FOR_APP_DETAILS();
				CHECK_FOR_MISSING_NAME();
				if (CHECK_STATE(COMPLETED) != COMPLETED)
					return (parse_error(fd, "widget definition incomplete", TRUE));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				RESET_STATE(COMPLETED | MANAGED | DEFERRED);
				if (new_object->instance == NULL)
					call_create_widget();
				if (num_args > 0) {
					Boolean manage =
			!IsShellClass(GetClass(new_object->class)) &&
			IsCompositeClass(GetClass(new_object->parent->class))
			&& XtIsRectObj(new_object->instance)
			&& new_object->managed;

					if (manage)
						XtUnmanageChild(
							new_object->instance);
					XtSetValues(new_object->instance,
						    (ArgList)args, num_args);
					if (manage)
						XtManageChild(
							new_object->instance);
				}
				release_argument_storage(args, num_args,
								free_value);
				num_args = 0;
				new_object = NULL;
				break;
			case LAYOUT_CONN : /* %L */
				CHECK_FOR_APP_DETAILS();
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetWhiteSpaceString(fd, &layout_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, ';'));
				PROCESS_ERROR(GetWhiteSpaceString(fd, &w1_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, ';'));
				PROCESS_ERROR(GetWhiteSpaceString(fd, &w2_name));
				PROCESS_ERROR(FindAndDiscardSeparator(fd, ';'));
				PROCESS_ERROR(GetIdentifierString(fd, &direct_name));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				RESET_STATE(IN_COMMAND | IN_LINE);
				PROCESS_ERROR(add_connection(fd, layout_name, w1_name, w2_name, direct_name));
				RELEASE_STORAGE(layout_name);
				RELEASE_STORAGE(w1_name);
				RELEASE_STORAGE(w2_name);
				RELEASE_STORAGE(direct_name);
				break;
			case ACTION_NAME : /* %Y */
				if (CHECK_STATE(APP_DEFINED))
					return(parse_error(fd, "Action names should be placed before application details", TRUE));
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &res_name));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				RegisterName(GetUIContext(), res_name, TRUE);
				RELEASE_STORAGE(res_name);
				break;
			case CALLBACK_NAME : /* %Z */
				if (CHECK_STATE(APP_DEFINED))
					return(parse_error(fd, "Callback names should be placed before application details", TRUE));
				SkipWhiteSpace(fd);
				PROCESS_ERROR(GetIdentifierString(fd, &res_name));
				PROCESS_ERROR(FindNewline(fd, FALSE));
				RegisterName(GetUIContext(), res_name, FALSE);
				RELEASE_STORAGE(res_name);
				break;
			default :
				XtError("unknown command in process_command_line()");
				/* NOTREACHED */
				break;
		}
	}
	RESET_STATE(IN_COMMAND | IN_LINE);
	return (NO_ERROR);
}


static void
show_busy_window(up)
Boolean up;
{
	if (up) {
		Dimension width, height;

		XtVaGetValues(XtParent(WI_busy_window),
				XtNwidth, &width, XtNheight, &height,
				NULL);
		XtVaSetValues(WI_busy_window,
				XtNwidth, width, XtNheight, height, NULL);
		XtManageChild(WI_busy_window);
		XRaiseWindow(XtDisplay(WI_busy_window),
				XtWindow(WI_busy_window));
		XSync(XtDisplay(WI_busy_window), FALSE);
	} else
		XtUnmanageChild(WI_busy_window);
}


/* Kludge: see below */
static XErrorHandler
dev_null_error_handler()
{
}


/****** Module Exported Routines ******/

Boolean
ProcessFile(fd, return_new_head)
FILE *fd;
ObjectPtr *return_new_head;
{
	Cardinal err;

	line_number = 1;
	new_object = new_start = NULL;
	num_unset = 0;
	widget_definition_state = NULL;
#ifdef LOG_SUB /* ||| */
	sub_resources_list = sub_values_list = 0;
#endif /* ||| */
	current_char = NEXTCHAR(fd);
	SkipWhiteSpace(fd);

	show_busy_window(TRUE);

	while ((err = process_command_line(fd))
	       & (NO_ERROR | CONTINUE_AFTER_ERROR));

	if (err != CAUGHT_EOF
	    || (err = try_unset_resources_again(fd)) != CAUGHT_EOF) {
		*return_new_head = NULL;
		widget_definition_state = NULL;
		/* clean up the data structures */
		if (new_object != NULL && new_object->instance == NULL)
			DestroyObjectRecord(new_object);
		if (new_start != NULL) {
			if (new_start->in_app) {
				XtUnmapWidget(new_start->instance);
				XtDestroyWidget(new_start->instance);
			} else {
				new_object = new_start->children;
				while (new_object != NULL) {
					Widget to_die = new_object->instance;
					new_object = new_object->next;
					XtUnmapWidget(to_die);
					XtDestroyWidget(to_die);
				}
				DestroyObjectRecord(new_start);
			}
		}
	} else {
		*return_new_head = new_start;
#ifdef LOG_SUB /* ||| */
		if (sub_resources_list > 0) {
			Cardinal i;

			(void)XukcGetListPointer(sub_resources_list, &i);
			for (; i > 0; i--)
				LogResourceDatabaseSetting(new_start->instance, (String)XukcGetObjectOnList(sub_resources_list, i), (String)XukcGetObjectOnList(sub_values_list, i), FALSE);
		}
#endif /* ||| */
		ChangeStartObjectRealization(TRUE);
		RealizeAllChildren(new_start);
		{
			/* Kludge: to stop BadWindow messages appearing
			 *         currently I don't know where they
			 *         are being generated!
			 */
			XErrorHandler old_err_handler =
				XSetErrorHandler(dev_null_error_handler);
			DrainUIEventQueue();
			(void)XSetErrorHandler(old_err_handler);
		}
		/* raise the main Dirt window above all other so we don't
		 * have to scrabble for it.
		 */
		XRaiseWindow(XtDisplay(WI_dirt), XtWindow(WI_dirt));
	}

	for (; num_unset > 0; num_unset--) {
		RELEASE_STORAGE(unset_resources[num_unset-1].name);
		RELEASE_STORAGE(unset_resources[num_unset-1].value);
	}

	show_busy_window(FALSE);
	return (err == CAUGHT_EOF && new_start != NULL);
}
