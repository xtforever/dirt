#ifndef lint
static char sccsid[] = "@(#)MotifConv.c	1.2 (UKC) 5/10/92";
#endif /* !lint */

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
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "Convert.h"
#include "MotifConv.h"
#include <Xm/XmP.h>
#include <X11/Xatom.h>

struct _XmFontListRec {
	XFontStruct	*font;
	XmStringCharSet	charset;
}

#define ErrorMessage(routine, message) { \
	Cardinal np = 0; \
	XtAppErrorMsg(XtDisplayToApplicationContext(display), \
			"wrongParameters", routine, "XukcToolkitError", \
			message, (String *)NULL, &np); \
}

/* copied from Xt/Converters.c !
 */
#define done(type, value) \
	{                                                       \
	    if (toVal->addr != NULL) {                          \
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;                               \
		}						\
		*(type*)(toVal->addr) = (value);		\
	    } else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->size = sizeof(type);			\
		toVal->addr = (XtPointer)&static_val;		\
	    }							\
	    return True;					\
	}


#define done_string(value) \
	{                                                       \
	    Cardinal size;					\
	    if (value == NULL) /* we shouldn't have caught this */	\
		return False;					\
	    size = strlen(value) + 1;				\
	    if (toVal->addr != NULL) {                          \
		if (toVal->size < size) {			\
		    toVal->size = size;				\
		    return False;                               \
		}						\
		(void)strcpy((char *)toVal->addr, value);	\
		toVal->size = size;				\
	    } else {						\
		toVal->size = size;				\
		toVal->addr = (XtPointer)(value);		\
	    }							\
	    return True;					\
	}


extern XukcRegisterPixmapConverters();


static Boolean
IsPositiveInteger(string, value)
String string;
Cardinal *value;
{
	Boolean foundDigit = FALSE;
	Cardinal val = 0;
	char ch;

	/* skip leading whitespace */
	while ((ch = *string) == ' ' || ch == '\t') string++;

	while (ch = *string++) {
		if (ch >= '0' && ch <= '9') {
		    val *= 10;
		    val += ch - '0';
		    foundDigit = True;
		    continue;
		}
		if (ch == ' ' || ch == '\t') {
		    if (!foundDigit) return False;
		    /* make sure only trailing whitespace */
		    while (ch = *string++) {
			if (ch != ' ' && ch != '\t')
			    return FALSE;
		    }
		    break;
		}
		return False;
	}

	if (ch != '\0')
		return FALSE;
	*value = val;
	return TRUE;
}


static Boolean
XukcCvtMotifFlagsToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char bitNames[1000];
	Cardinal i = 0, list_num;
	unsigned char value;
	EnumList enumList;

	if (*num_args != 3)
		ErrorMessage("cvtMotifFlagsToString",
			"Bit flag to String conversion needs EnumList array argument, its size and the resource name");

	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;
	bitNames[0] = '\0';
	value = *(unsigned char *)fromVal->addr;

	for (i = 0; i < list_num; i++) {
		if (value & (unsigned char)enumList[i].value) {
			value &= ~(unsigned char)enumList[i].value;
			(void)strcat(bitNames, enumList[i].name);
			(void)strcat(bitNames, " ");
		}
	}

	if (value > 0) {
		char value_str[30];

		(void)sprintf(value_str, "%d", (int)value);
		(void)strcat(bitNames, value_str);
	}

	if (list_num > 0 && *bitNames == '\0') {
		i = 1;
		XtAppWarningMsg(XtDisplayToApplicationContext(display),
				"cannotConvert", "cvtMotifFlagsToString",
				"XukcToolkitWarning",
				"Unable to convert %s resource type value to String representation",
				(String *)&args[2].addr, &i);
		return FALSE;
	}

	done_string(bitNames);
}


static Boolean
XukcCvtStringToMotifFlags(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
#define	SKIP(func) { while(*nextName != '\0' && func(*nextName)) nextName++; }

	static unsigned char bitSettings;
	int value;
	Cardinal i = 0, list_num;
	char lowerName[1000];
	char *nextName, *currentName;
	EnumList enumList;
	extern void LowerCase();

	if (*num_args != 3)
		ErrorMessage("cvtStringToMotifFlags",
			"String to bit flag conversion needs QList array argument, its size and the resource name");

	XmuCopyISOLatin1Lowered(lowerName, (char *)fromVal->addr);

	bitSettings = 0;
	currentName = nextName = lowerName;
	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;

	/* skip any whitespace or puncuation */
	SKIP(!isalpha);
	while (*nextName != '\0') {
		currentName = nextName;
		/* find end of the current name */
		SKIP(isalpha);
		if (nextName != currentName) {
			if (*nextName != '\0') {
				/* not at end of input string so divide it */
				*nextName = '\0';
				nextName++;
			}
			/* find internal representation of name */
			for (i = 0; i < list_num; i++) {
				if (strcmp(currentName, enumList[i].name) == 0)
					break;
			}
			/* OR this new status flag with any previous flags set */
			if (i != list_num)
				bitSettings |= (unsigned char)enumList[i].value;
			else if (IsPositiveInteger(currentName, &value))
				bitSettings |= (unsigned char)value;
			else {
				/* bad value */
				XtDisplayStringConversionWarning(display,
						currentName, args[2].addr);
				return False;
			}
		}
		SKIP(!isalpha);
	}
	done(unsigned char, bitSettings);
#undef SKIP
}


static Boolean
XukcCvtMotifValueToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	Cardinal i = 0, list_num;
	EnumList enumList;

	if (*num_args != 3)
		ErrorMessage("cvtMotifValueToString",
			"Motif value to String conversion needs EnumList array argument, its size and the resource name");

	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;

	for (i = 0; i < list_num; i++)
		if (*(unsigned char *)fromVal->addr
		    == (unsigned char)enumList[i].value)
			break;

	if (i != list_num)
		done_string(enumList[i].name)
	else {
		static char value_str[30];

		(void)sprintf(value_str, "%d",
					(int)*(unsigned char *)fromVal->addr);
		done_string(value_str);
	}
}


static Boolean
XukcCvtStringToMotifValue(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	Cardinal i = 0, list_num;
	static unsigned char value;
	char lowerName[1000];
	EnumList enumList;
	extern void LowerCase();

	if (*num_args != 3)
		ErrorMessage("cvtStringToMotifValue",
			"String to Motif value conversion needs EnumList array argument, its size and the resource name");

	XmuCopyISOLatin1Lowered(lowerName, (char *)fromVal->addr);

	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;

	/* find internal representation of name */
	for (i = 0; i < list_num; i++) {
		if (strcmp(lowerName, enumList[i].name) == 0)
			break;
	}

	if (i != list_num)
		done(unsigned char, (unsigned char)enumList[i].value)
	else if (IsPositiveInteger(lowerName, &value))
		done(unsigned char, (unsigned char)value);

	XtDisplayStringConversionWarning(display, (String)fromVal->addr,
							(String)args[2].addr);
	return False;
}


/********************************************************************/
/* Conversion routines for TypeToString conversions only            */
/********************************************************************/

static Boolean
XukcCvtWidgetToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	static String widget_name;
	Widget tmp;

	if (*num_args != 0)
		ErrorMessage("cvtWidgetToString",
			"Widget to String needs no extra parameters");

	tmp = *(Widget *)fromVal->addr;

	if (tmp != NULL)
		widget_name = XtName(tmp);
	else
		widget_name = "NULL";

	done_string(widget_name);
}


static Boolean
CvtXmStringToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	String str = "";
	XmString tmp;

	if (*num_args != 0)
		ErrorMessage("cvtXmStringToString",
			"XmString to String needs no extra parameters");

	tmp = *(String *)fromVal->addr;

	done_string(str);
}


static Boolean
CvtXmFontListToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	static String str = NULL;
	char *name = NULL;
	XmFontList tmp;
	  /* members = (XFontStruct *)font and (XmStringCharSet)charset */
	unsigned long val;
	Cardinal i = 0;

	if (*num_args != 0)
		ErrorMessage("cvtXmFontListToString",
			"FontList to String needs no extra parameters");

	tmp = *(XmFontList *)fromVal->addr;

	if (str != NULL)
		XtFree(str);
	str = NULL;

	while (tmp != NULL && tmp->font != NULL && tmp->charset != NULL) {
		if (!XGetFontProperty(tmp->font, XA_FONT, &val)) {
			/* error */
			return False;
		}
		name = XGetAtomName(display, (Atom)val);
		if (name == NULL)
			name = XtNewString("fixed"); /* yukky */

		if (str == NULL) {
			i = strlen(name) + strlen(tmp->charset) + 5;
			str = XtMalloc(i + 1);
		} else {
			i += strlen(name) + strlen(tmp->charset) + 6;
			str = XtRealloc(str, i + 1);
			(void)strcat(str, ",");
		}
		(void)strcat(str, "\"");
		(void)strcat(str, name);
		(void)strcat(str, "\"");
		(void)strcat(str, "=");
		(void)strcat(str, "\"");
		(void)strcat(str, tmp->charset);
		(void)strcat(str, "\"");
		tmp++;
	}
	done_string(str);
}


Boolean
XukcCvtKeySymToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	String str = "";
	KeySym tmp;

	if (*num_args != 0)
		ErrorMessage("cvtKeySymToString",
			"KeySym to String needs no extra parameters");

	tmp = *(KeySym *)fromVal->addr;

	done_string(str);
}


/************** Module Exported Routines ***************/


/******************** Initialization Routine ************************/

void
XukcAddMotifConverters()
{
	/* these are for the Motif Widget set only */

#if 0
	XukcRegisterPixmapConverters(XmRManBottomShadowPixmap);
	XukcRegisterPixmapConverters(XmRManHighlightPixmap);
	XukcRegisterPixmapConverters(XmRManTopShadowPixmap);
	XukcRegisterPixmapConverters(XmRBackgroundPixmap);
	XukcRegisterPixmapConverters("XmBackgroundPixmap");
	XukcRegisterPixmapConverters(XmRPrimBottomShadowPixmap);
	XukcRegisterPixmapConverters(XmRPrimHighlightPixmap);
	XukcRegisterPixmapConverters(XmRPrimTopShadowPixmap);
	XukcRegisterPixmapConverters(XmRGadgetPixmap);
#endif

	XtSetTypeConverter(XmRMenuWidget, XtRString, XukcCvtWidgetToString,
			NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRFontList, XtRString, CvtXmFontListToString,
			NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRNavigationType, XtRString,
			XukcCvtMotifValueToString,
			xm_navigation_type_Args,
			XtNumber(xm_navigation_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRStringDirection, XtRString,
			XukcCvtMotifValueToString,
			xm_string_direction_Args,
			XtNumber(xm_string_direction_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRArrowDirection, XtRString,
			XukcCvtMotifValueToString,
			xm_arrow_direction_Args,
			XtNumber(xm_arrow_direction_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRUnitType, XtRString,
			XukcCvtMotifValueToString,
			xm_unit_type_Args,
			XtNumber(xm_unit_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRShadowType, XtRString,
			XukcCvtMotifValueToString,
			xm_shadow_type_Args,
			XtNumber(xm_shadow_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRResizePolicy, XtRString,
			XukcCvtMotifValueToString,
			xm_resize_policy_Args,
			XtNumber(xm_resize_policy_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRDialogStyle, XtRString,
			XukcCvtMotifValueToString,
			xm_dialog_style_Args,
			XtNumber(xm_dialog_style_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRStringDirection, XtRString,
			XukcCvtMotifValueToString,
			xm_string_direction_Args,
			XtNumber(xm_string_direction_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRLabelType, XtRString,
			XukcCvtMotifValueToString,
			xm_label_type_Args,
			XtNumber(xm_label_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRDialogType, XtRString,
			XukcCvtMotifValueToString,
			xm_dialog_type_Args,
			XtNumber(xm_dialog_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRFileTypeMask, XtRString,
			XukcCvtMotifValueToString,
			xm_file_type_mask_Args,
			XtNumber(xm_file_type_mask_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRAlignment, XtRString,
			XukcCvtMotifValueToString,
			xm_alignment_Args,
			XtNumber(xm_alignment_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRSelectionPolicy, XtRString,
			XukcCvtMotifValueToString,
			xm_selection_policy_Args,
			XtNumber(xm_selection_policy_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRListSizePolicy, XtRString,
			XukcCvtMotifValueToString,
			xm_list_size_policy_Args,
			XtNumber(xm_list_size_policy_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRScrollBarDisplayPolicy, XtRString,
			XukcCvtMotifValueToString,
			xm_scrollbar_display_policy_Args,
			XtNumber(xm_scrollbar_display_policy_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRScrollingPolicy, XtRString,
			XukcCvtMotifValueToString,
			xm_scrolling_policy_Args,
			XtNumber(xm_scrolling_policy_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRVisualPolicy, XtRString,
			XukcCvtMotifValueToString,
			xm_visual_policy_Args,
			XtNumber(xm_visual_policy_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRScrollBarPlacement, XtRString,
			XukcCvtMotifValueToString,
			xm_scrollbar_placement_Args,
			XtNumber(xm_scrollbar_placement_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRCommandWindowLocation, XtRString,
			XukcCvtMotifValueToString,
			xm_command_window_location_Args,
			XtNumber(xm_command_window_location_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRProcessingDirection, XtRString,
			XukcCvtMotifValueToString,
			xm_processing_direction_Args,
			XtNumber(xm_processing_direction_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRSeparatorType, XtRString,
			XukcCvtMotifValueToString,
			xm_separator_type_Args,
			XtNumber(xm_separator_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRWhichButton, XtRString,
			XukcCvtMotifValueToString,
			xm_which_button_Args,
			XtNumber(xm_which_button_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRPacking, XtRString,
			XukcCvtMotifValueToString,
			xm_packing_Args,
			XtNumber(xm_packing_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmROrientation, XtRString,
			XukcCvtMotifValueToString,
			xm_orientation_Args,
			XtNumber(xm_orientation_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRAttachment, XtRString,
			XukcCvtMotifValueToString,
			xm_attachment_Args,
			XtNumber(xm_attachment_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRIndicatorType, XtRString,
			XukcCvtMotifValueToString,
			xm_indicator_type_Args,
			XtNumber(xm_indicator_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRDefaultButtonType, XtRString,
			XukcCvtMotifValueToString,
			xm_default_button_type_Args,
			XtNumber(xm_default_button_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmRRowColumnType, XtRString,
			XukcCvtMotifValueToString,
			xm_row_column_type_Args,
			XtNumber(xm_row_column_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

/******************************/
	XtSetTypeConverter(XmRBooleanDimension, XtRString,
			XukcCvtMotifValueToString,
			xm_boolean_dimension_Args,
			XtNumber(xm_boolean_dimension_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XmREditMode, XtRString,
			XukcCvtMotifValueToString,
			xm_edit_mode_Args,
			XtNumber(xm_edit_mode_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
/******************************/
}
#endif /* MOTIF_WIDGETS */
