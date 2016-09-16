#ifndef lint
static char sccsid[] = "@(#)Converters.c	1.3 (UKC) 5/10/92";
#endif /* !lint */

/* 
 * Copyright 1990,1992 Richard Hesketh / rlh2@ukc.ac.uk
 *                     Computing Lab. University of Kent at Canterbury, UK
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

#include<ctype.h>
#include<X11/Xatom.h>
#include<X11/Xos.h>
#include<X11/IntrinsicP.h>
#include<X11/StringDefs.h>
#include <X11/Xukc/memory.h>
#include "Converters.h"

#ifdef MOTIF_WIDGETS
extern void XukcAddMotifConverters();
#endif /* MOTIF_WIDGETS */

extern void XmuCopyISOLatin1Lowered();
extern Pixmap XukcLocatePixmapFile();

#define ErrorMessage(routine, message) { \
	Cardinal np = 0; \
	XtAppErrorMsg(XtDisplayToApplicationContext(display), \
			"wrongParameters", routine, "XukcToolkitError", \
			message, (String *)NULL, &np); \
	return False; \
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


/************** Module Exported Routines ***************/

String
XukcCreateMeasurementString(screen, value, metric)
Screen *screen;
int value;
Boolean metric;
{
	static char out[100];
	int mm_size, size;

	/* This only works for screens with Square Pixels !!! */

	/* size of the screen in millimeters */
	mm_size = XWidthMMOfScreen(screen);

	/* size of the screen in pixels */
	size = XWidthOfScreen(screen);

	if (metric)
		(void)sprintf(out, "%fmm",
				(((double)value / size) * (double)mm_size));
	else
		(void)sprintf(out, "%fin",
			((((double)value / size) * (double)mm_size) / 25.4));
	return (out);
}


Boolean
XukcParseMeasurementString(screen, string, result, allow_negative)
Screen *screen;
String string;
int *result;
Boolean allow_negative;
{
	Boolean found_negative, is_real, found_error;
	int mm_size, size;
	double metric_size, conversion_factor;
	String current;

	/* This only works for screens with Square Pixels !!! */

	/* size of the screen in millimeters */
	mm_size = XWidthMMOfScreen(screen);

	/* size of the screen in pixels */
	size = XWidthOfScreen(screen);

	found_negative = is_real = found_error = FALSE;
	current = string;
	metric_size = 0;
	conversion_factor = 0;

	/* remove whitespace */
	while (isspace(*current)) current++;

	/* minus number? */
	if (*current == '-') {
		if (allow_negative) {
			found_negative = TRUE;
			current++;
		} else
			found_error = TRUE;
	}

	while (!found_error && *current && isdigit(*current)) {
		metric_size *= 10;
		metric_size += *current - '0';
		current++;
	}

	while (!found_error && *current && *current == '.') {
		double metric_fraction = 1;

		is_real = TRUE;
		conversion_factor = 1;
		current++;

		while (*current && isdigit(*current)) {
			metric_fraction /= 10;
			metric_size += (*current - '0')
						* metric_fraction;
			current++;
		}
		if (metric_fraction == 1) /* no number after the '.' */
			found_error = TRUE;
	}

	while (!found_error && *current) {
		switch (*current) {
			case 'm': /* millimetres */
				current++;
				if (*current == 'm')
					conversion_factor = 1;
				else
					found_error = TRUE;
				break;
			case 'c': /* centimetres */
				current++;
				if (*current == 'm')
					conversion_factor = 10;
				else
					found_error = TRUE;
				break;
			case 'M': /* Metres 8-) */
				conversion_factor = 1000;
				break;
			case 'i': /* inches */
				current++;
				if (*current != 'n')
					found_error = TRUE;
			case '\'': /* Inches */
				conversion_factor = 25.4;
				break;
			case 'f': /* feet */
				current++;
				if (*current != 't')
					found_error = TRUE;
			case '\"': /* Feet 8-} */
				conversion_factor = 308.8;
				break;
			default:
				found_error = TRUE;
				break;
		}
		current++;
	}

	found_error |= *current;

	if (!found_error) {
		if (is_real || conversion_factor != 0) {
			*result = (int)((metric_size * conversion_factor
					* (double)size) / (double)mm_size +
					0.5);
		} else /* absolute pixel size */
			*result = metric_size;
		if (found_negative)
			*result = -*result;
	}
	return (!found_error);
}


Boolean
XukcCvtFlagSettingsToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char bitNames[1000];
	Cardinal i = 0, list_num;
	int value;
	EnumList enumList;

	if (*num_args != 3)
		ErrorMessage("cvtFlagSettingsToString",
			"Bit flag to String conversion needs EnumList array argument, its size and the resource name");

	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;
	bitNames[0] = '\0';
	value = *(int *)fromVal->addr;

	for (i = 0; i < list_num; i++) {
		if (value & enumList[i].value) {
			value &= ~enumList[i].value;
			(void)strcat(bitNames, enumList[i].name);
			(void)strcat(bitNames, " ");
		}
	}

	if (value > 0) {
		char value_str[30];

		(void)sprintf(value_str, "%d", value);
		(void)strcat(bitNames, value_str);
	}

	if (list_num > 0 && *bitNames == '\0') {
		i = 1;
		XtAppWarningMsg(XtDisplayToApplicationContext(display),
				"cannotConvert", "cvtFlagSettingsToString",
				"XukcToolkitWarning",
				"Unable to convert %s resource type value to String representation",
				(String *)&args[2].addr, &i);
		return FALSE;
	}

	done_string(bitNames);
}


Boolean
XukcCvtStringToFlagSettings(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
#define	SKIP(func) { while(*nextName != '\0' && func(*nextName)) nextName++; }

	int bitSettings, value;
	Cardinal i = 0, list_num;
	char lowerName[1000];
	char *nextName, *currentName, *orig_name, *test_name;
	EnumList enumList;
	extern void LowerCase();

	if (*num_args != 3)
		ErrorMessage("cvtStringToFlagSettings",
			"String to bit flag conversion needs QList array argument, its size and the resource name");

	orig_name = (char *)fromVal->addr;
	XmuCopyISOLatin1Lowered(lowerName, orig_name);

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
			test_name = orig_name + (currentName - lowerName);

			if (*nextName != '\0') {
				/* not at end of input string so divide it */
				*nextName = '\0';
				nextName++;
			}
			/* find internal representation of name */
			for (i = 0; i < list_num; i++) {
				if (strcmp(currentName, enumList[i].name) == 0)
					break;
				else /* check with non-lowercased version */
				     if (strncmp(test_name, enumList[i].name,
					 nextName - currentName) == 0)
					break;
			}
			/* OR this new status flag with any previous flags set */
			if (i != list_num)
				bitSettings |= enumList[i].value;
			else if (IsPositiveInteger(currentName, &value))
				bitSettings |= value;
			else {
				/* bad value */
				XtDisplayStringConversionWarning(display,
						currentName, args[2].addr);
				return False;
			}
		}
		SKIP(!isalpha);
	}
	done(int, bitSettings);
#undef SKIP
}


Boolean
XukcCvtNamedValueToString(display, args, num_args, fromVal, toVal, conv_data)
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
		ErrorMessage("cvtNamedValueToString",
			"Named value to String conversion needs EnumList array argument, its size and the resource name");

	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;

	for (i = 0; i < list_num; i++)
		if (*(int *)fromVal->addr == enumList[i].value)
			break;

	if (i != list_num)
		done_string(enumList[i].name)
	else {
		static char value_str[30];

		(void)sprintf(value_str, "%d", *(int *)fromVal->addr);
		done_string(value_str);
	}
}


Boolean
XukcCvtStringToNamedValue(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	Cardinal i = 0, list_num;
	int value;
	char lowerName[1000];
	EnumList enumList;
	extern void LowerCase();

	if (*num_args != 3)
		ErrorMessage("cvtStringToNamedValue",
			"String to named value conversion needs EnumList array argument, its size and the resource name");

	XmuCopyISOLatin1Lowered(lowerName, (char *)fromVal->addr);

	enumList = (EnumList)args[0].addr;
	list_num = *(Cardinal *)args[1].addr;

	/* find internal representation of name */
	for (i = 0; i < list_num; i++) {
		if (strcmp((char *)fromVal->addr, enumList[i].name) == 0 ||
		    strcmp(lowerName, enumList[i].name) == 0)
			break;
	}

	if (i != list_num)
		done(int, enumList[i].value)
	else if (IsPositiveInteger(lowerName, &value))
		done(int, value);

	XtDisplayStringConversionWarning(display, (String)fromVal->addr,
							(String)args[2].addr);
	return False;
}

/***********************************************************/

static Cardinal font_name_list = 0;
static Cardinal font_value_list = 0;

static void
free_fontStruct_entry(app, to, conv_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer conv_data;
XrmValue *args;
Cardinal *num_args;
{
	Cardinal i;

	if (font_name_list == 0 || conv_data == NULL)
		return;

	i = XukcFindObjectOnList(font_name_list, conv_data);

	if (i == 0)
		return;

	XtFree(conv_data);
	XukcRemoveEntryOnList(&font_name_list, i);
	XukcRemoveEntryOnList(&font_value_list, i);
}


static String
save_font_struct(name, fs)
String name;
XFontStruct *fs;
{
	String tmp = XtNewString(name);

	XukcAddObjectToList(&font_name_list, (XtPointer)tmp, FALSE);
	XukcAddObjectToList(&font_value_list, (XtPointer)fs, FALSE);
	return (tmp);
}


/* Taken from Xt/Converters.c */
/*ARGSUSED*/
static Boolean
CvtStringToFontStruct(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    XFontStruct	    *f;
    Display*	display = dpy;

    if (*num_args != 1)
		ErrorMessage("cvtStringToFontStruct",
		     "String to font conversion needs display argument");

    display = DisplayOfScreen((*(Screen **)args[0].addr));

    if (strcmp((String)fromVal->addr, XtDefaultFont) != 0) {
	f = XLoadQueryFont(display, (char *)fromVal->addr);
	if (f != NULL) {
	    save_font_struct((char *)fromVal->addr, f);
  Done:	    done( XFontStruct*, f);
	}

	XtDisplayStringConversionWarning( dpy, (char*)fromVal->addr,
					  "FontStruct" );
    }

    /* try and get the default font */

    {
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;

	xrm_name[0] = XrmStringToName ("xtDefaultFont");
	xrm_name[1] = NULL;
	xrm_class[0] = XrmStringToClass ("XtDefaultFont");
	xrm_class[1] = NULL;
	if (XrmQGetResource(XtDatabase(dpy), xrm_name, xrm_class, 
			    &rep_type, &value)) {
	    if (rep_type == XrmStringToName("String")) {
		f = XLoadQueryFont(display, (char*)value.addr);
		if (f != NULL) {
		    save_font_struct((char *)fromVal->addr, f);
		    goto Done;
		} else {
		    XtDisplayStringConversionWarning( dpy, (char*)value.addr,
						      "FontStruct" );
		}
	    } else if (rep_type == XrmStringToName("Font")) {
		f = XQueryFont(dpy, *(Font*)value.addr );
		if (f != NULL) goto Done;
	    } else if (rep_type == XrmStringToName("FontStruct")) {
		f = (XFontStruct*)value.addr;
		goto Done;
	    }
	}
    }
    /* Should really do XListFonts, but most servers support this */
    f = XLoadQueryFont(dpy,"-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");
    if (f != NULL) {
	save_font_struct("-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1", f);
	goto Done;
    }	

    ErrorMessage("cvtStringToFontStruct",
		     "Unable to load any useable ISO8859-1 font");
    
    return False;
}


/* ARGSUSED */
static void FreeFontStruct(app, toVal, closure, args, num_args)
    XtAppContext app;
    XrmValuePtr	toVal;
    XtPointer	closure;	/* unused */
    XrmValuePtr	args;
    Cardinal	*num_args;
{
    Display *display;
    if (*num_args != 1) {
	Cardinal np = 0;
	XtAppErrorMsg(XtDisplayToApplicationContext(display),
			"wrongParameters", "freeFontStruct",
			"XukcToolkitError", 
			"Free FontStruct requires display argument",
			(String *)NULL, &np);
	return;
    }

    display = *(Display**)args[0].addr;
    XFreeFont( display, *(XFontStruct**)toVal->addr );
    free_fontStruct_entry(app, toVal, closure, args, num_args);
}


static Boolean
XukcCvtFontStructToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char *name = NULL;
	XFontStruct *font_struct;
	unsigned long val;

	if (*num_args != 0)
		ErrorMessage("cvtFontStructToString",
			"FontStruct to String needs no extra arguments");

	if (name != NULL)
		XtFree(name);

	font_struct = *(XFontStruct **)fromVal->addr;

	if (font_name_list != 0) {
		Cardinal i = 0, fnum, num_fonts;
		int best_font;
		Boolean not_found;
		XFontStruct *font;
		char *fname;
		
		not_found = TRUE;
		i = 1;
		fnum = 0;
		best_font = -1;

		(void)XukcGetListPointer(font_value_list, &num_fonts);

		while (not_found && i <= num_fonts) {
			font = (XFontStruct *)XukcGetObjectOnList(
							font_value_list, i);
			if (font == font_struct) {
				Cardinal j = 0;
				String str;
				extern char *index();

				not_found = FALSE;
				str = (String)XukcGetObjectOnList(font_name_list, i);
				while ((str = index(str, '-')) != (char *)0)
					str++, j++;
				if (j > best_font) {
					fnum = i;
					best_font = j;
				}
			} else
				i++;
		}

		if (not_found)
			return False;

		fname = (String)XukcGetObjectOnList(font_name_list, fnum);
		name = XtNewString(fname);
		done_string(name);
	}

	if (font_struct == NULL ||
	    !XGetFontProperty(font_struct, XA_FONT, &val)) {
		Cardinal i = 0;
		XtAppWarningMsg(XtDisplayToApplicationContext(display),
			"cannotConvert", "cvtFontStructToString",
			"XukcToolkitWarning",
			"Unable to convert XFontStruct value to String representation : Font has no FONT (name) property",
			(String *)NULL, (Cardinal *)&i);
		return False;
	}

	name = XGetAtomName(display, (Atom)val);
	if (name == NULL)
		/* yuk, okay so its a default fount */
		name = XtNewString("-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");
	done_string(name);

}


/***********************************************************/


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
	static String widget_name = NULL;
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
XukcCvtFloatToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer*  conv_data;
{
	static char float_str[50];

	if (*num_args != 0)
		ErrorMessage("cvtFloatToString",
			"Float to String needs no extra parameters");

	(void)sprintf(float_str, "%f", *(float *)fromVal->addr);
	done_string(float_str);
}


static Boolean
XukcCvtBooleanToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static String ret;

	if (*num_args != 0)
		ErrorMessage("cvtBooleanToString",
			"Boolean to String needs no extra parameters");

	ret = *(Boolean *)(fromVal->addr) != NULL ? "true" : "false";
	done_string(ret);
}


static Boolean
XukcCvtAddressToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char name[50];

	if (*num_args != 0)
		ErrorMessage("cvtAddressToString",
			"Address to String needs no extra parameters");

	(void)sprintf(name, "0x%0x", *(XtPointer *)fromVal->addr);
	done_string(name);
}


static Boolean
XukcCvtIntegerToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char name[50];
	int value;

	if (*num_args != 0)
		ErrorMessage("cvtIntegerToString",
			"Integer to String needs no extra parameters");

	if (fromVal->size == sizeof(short))
		value = *(short *)fromVal->addr;
	else if (fromVal->size == sizeof(char))
		value = *(char *)fromVal->addr;
	else
		value = *(int *)fromVal->addr;

	(void)sprintf(name, "%ld", value);
	done_string(name);
}


static Boolean
XukcCvtUnsignedIntToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char name[50];
	int value;

	if (*num_args != 0)
		ErrorMessage("cvtUnsignedIntToString",
			"UnsignedInteger to String needs no extra parameters");

	if (fromVal->size == sizeof(unsigned short))
		value = *(unsigned short *)fromVal->addr;
	else if (fromVal->size == sizeof(unsigned char))
		value = *(unsigned char *)fromVal->addr;
	else
		value = *(unsigned int *)fromVal->addr;

	(void)sprintf(name, "%lu", value);
	done_string(name);
}


static Boolean
XukcCvtPixelToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char name[10];
	XColor color;
	Screen *screen;
	Colormap colormap;

	if (*num_args != 2)
		ErrorMessage("cvtPixelToString",
			"Pixel to String needs Screen and Colormap arguments");

	screen = *((Screen **)args[0].addr);
	colormap = *((Colormap *)args[1].addr);
	color.pixel = *(Pixel *)fromVal->addr;
	XQueryColor(DisplayOfScreen(screen), colormap, &color);
	(void)sprintf(name, "#%02x%02x%02x", color.red / 256,
						    color.green / 256,
						    color.blue / 256);
	done_string(name);
}


static Boolean
XukcCvtAtomToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char *name = NULL;
	Atom atom;
	Cardinal i = 0;

	if (*num_args != 0)
		ErrorMessage("cvtAtomToString",
			"Atom to String needs no extra arguments");

	atom = *(Atom *)fromVal->addr;
	if (atom == NULL) {
		XtAppWarningMsg(XtDisplayToApplicationContext(display),
			"cannotConvert", "cvtAtomToString",
			"XukcToolkitWarning",
			"Unable to convert Atom value of NULL to String representation",
			(String *)NULL, (Cardinal *)&i);
		return False;
	}
	if (name != NULL)
		XFree(name);
	name = XGetAtomName(display, atom);
	if (name == NULL)
		return False;
	done_string(name);
}


/**********************************************/
/* Added converters that override the Xt ones */
/**********************************************/

static Boolean
XukcCvtStringToDimension(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static Dimension value;
	int ret_int;
	Screen *screen;

	if (*num_args != 1)
		ErrorMessage("cvtStringToDimension",
			"String To Dimension needs Screen argument");

	screen = *((Screen **)args[0].addr);

	if (!XukcParseMeasurementString(screen, fromVal->addr, &ret_int, FALSE)) {
		XtDisplayStringConversionWarning(display,
						fromVal->addr, args[1].addr);
		return FALSE;
	}
	value = (Dimension)ret_int;

	done(Dimension, value);
}


static Boolean
XukcCvtStringToPosition(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static Position value;
	int ret_int;
	Screen *screen;

	if (*num_args != 1)
		ErrorMessage("cvtStringToPosition",
				"String To Position needs Screen argument");

	screen = *((Screen **)args[0].addr);

	if (!XukcParseMeasurementString(screen, fromVal->addr, &ret_int, TRUE)) {
		XtDisplayStringConversionWarning(display,
						fromVal->addr, args[1].addr);
		return FALSE;
	}
	value = (Position)ret_int;

	done(Position, value);
}

/***************** Pixmap/Bitmap/Cursor Conversion **************/

static Cardinal cur_name_list = 0;
static Cardinal cur_value_list = 0;
static Cardinal pix_name_list = 0;
static Cardinal pix_value_list = 0;

#if defined(__STDC__)
static XtConvertArgRec const screenConvertArg[] = {
    {XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
							sizeof(Screen *)}
};
#else
static XtConvertArgRec screenConvertArg[] = {
    {XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
							sizeof(Screen *)}
};
#endif

/* Destructors for pixmap/bitmap and cursor cache lists */
static void
free_pix_entry(app, to, conv_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer conv_data;
XrmValue *args;
Cardinal *num_args;
{
	Cardinal i;

	if (pix_name_list == 0 || conv_data == NULL)
		return;

	i = XukcFindObjectOnList(pix_name_list, conv_data);

	if (i == 0)
		return;

	XtFree(conv_data);
	XukcRemoveEntryOnList(&pix_name_list, i);
	XukcRemoveEntryOnList(&pix_value_list, i);
}


static void
free_cursor_entry(app, to, conv_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer conv_data;
XrmValue *args;
Cardinal *num_args;
{
	Cardinal i;

	if (cur_name_list == 0 || conv_data == NULL)
		return;

	i = XukcFindObjectOnList(cur_name_list, conv_data);

	if (i == 0)
		return;

	XtFree(conv_data);
	XukcRemoveEntryOnList(&cur_name_list, i);
	XukcRemoveEntryOnList(&cur_value_list, i);
}


static String
save_pixmap_cvt(name, pix)
String name;
Pixmap pix;
{
	String tmp = XtNewString(name);

	XukcAddObjectToList(&pix_name_list, (XtPointer)tmp, FALSE);
	XukcAddObjectToList(&pix_value_list, (XtPointer)pix, FALSE);
	return (tmp);
}


static String
save_cursor_cvt(name, cur)
String name;
Cursor cur;
{
	String tmp = XtNewString(name);

	XukcAddObjectToList(&cur_name_list, (XtPointer)tmp, FALSE);
	XukcAddObjectToList(&cur_value_list, (XtPointer)cur, FALSE);
	return (tmp);
}


static Boolean
XukcCvtCursorToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char *name = NULL;
	Cursor cursor, cur;
	Boolean not_found;
	Cardinal i = 0, num_cursors;

	if (*num_args != 0)
		ErrorMessage("cvtCursorToString",
				"Cursor to String needs no extra arguments");

	cursor = *(Cursor *)fromVal->addr;

	if (cursor == None)
		done_string("None");

	if (cur_value_list == 0)
		return FALSE;

	not_found = TRUE;
	i = 1;
	(void)XukcGetListPointer(cur_value_list, &num_cursors);

	while (not_found && i <= num_cursors) {
		cur = (Cursor)XukcGetObjectOnList(cur_value_list, i);
		if (cur == cursor)
			not_found = FALSE;
		else
			i++;
	}

	if (not_found)
		return FALSE;

	name = (String)XukcGetObjectOnList(cur_name_list, i);
	done_string(name);
}


static Boolean
XukcCvtPixmapToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	static char *name = NULL;
	Pixmap pixmap, pix;
	Boolean not_found;
	Cardinal i = 0, num_pixmaps;

	if (*num_args != 0)
		ErrorMessage("cvtPixmapToString",
			"Pixmap to String needs no extra arguments");

	pixmap = *(Pixmap *)fromVal->addr;

	if (pixmap == None)
		done_string("None");

	if (pixmap == XtUnspecifiedPixmap)
		done_string("Unspecified");

	if (pixmap == ParentRelative)
		done_string("ParentRelative");

	if (pix_name_list == 0)
		return FALSE;

	not_found = TRUE;
	i = 1;

	(void)XukcGetListPointer(pix_value_list, &num_pixmaps);

	while (not_found && i <= num_pixmaps) {
		pix = (Pixmap)XukcGetObjectOnList(pix_value_list, i);

		if (pix == pixmap)
			not_found = FALSE;
		else
			i++;
	}

	if (not_found)
		return False;

	name = (String)XukcGetObjectOnList(pix_name_list, i);
	done_string(name);
}


Boolean
XukcCvtStringToPixmap(display, args, num_args, fromVal, toVal, conv_data)
Display *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr	fromVal;
XrmValuePtr	toVal;
XtPointer 	*conv_data;
{
    static Pixmap pixmap;		/* static for cvt magic */
    char *name = (char *)fromVal->addr;
    Pixel fg, bg;
    Screen *screen;

    if (!(*num_args == 1 || *num_args == 4))
	ErrorMessage("cvtStringToPixmap",
	     "String to Pixmap conversion needs screen argument and optional foreground and background colours");

    if (conv_data != NULL)
	*conv_data = NULL;

    if (strcmp(name, "Unspecified") == 0) {
	pixmap = XtUnspecifiedPixmap;
	done(Pixmap, pixmap);
    }

    if (strcmp(name, "None") == 0) {
	pixmap = None;
	done(Pixmap, pixmap);
    }

    if (strcmp(name, "ParentRelative") == 0) {
	pixmap = ParentRelative;
	done(Pixmap, pixmap);
    }

    screen = *((Screen **)args[0].addr);

#ifdef UKC_WIDGETS
    if (*num_args == 4 &&
	*((WidgetClass *)args[1].addr) == ukcLabelWidgetClass) {
		fg = *((Pixel *)args[2].addr);
		bg = *((Pixel *)args[3].addr);
    } else {
		fg = BlackPixelOfScreen(screen);
		bg = WhitePixelOfScreen(screen);
    }
#else
    fg = BlackPixelOfScreen(screen);
    bg = WhitePixelOfScreen(screen);
#endif /* UKC_WIDGETS */

    pixmap = XukcLocatePixmapFile(screen, name, NULL, 0, NULL, NULL, fg, bg);

    if (pixmap != None) {
	if (conv_data != NULL)
		*conv_data = save_pixmap_cvt(name, pixmap);
	done(Pixmap, pixmap);
    }

    XtDisplayStringConversionWarning(display, name, "Pixmap");
    return FALSE;
}


void
XukcRegisterPixmapConverters(pixmap_resource_type)
String pixmap_resource_type;
{
	XtSetTypeConverter(XtRString, pixmap_resource_type,
			XukcCvtStringToPixmap,
			(XtConvertArgRec *)pixmap_Args, XtNumber(pixmap_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)free_pix_entry);

	XtSetTypeConverter(pixmap_resource_type, XtRString,
			XukcCvtPixmapToString,
			(XtConvertArgRec *)NULL, 0, XtCacheNone,
			(XtDestructor)NULL);
}


/* The following has been taken from Xmu/StrToBmap.c and Xmu/StrCurs.c
 * and then embellished a bit .. to support caching of names so that
 * they can be found and used by the added PixmapToString and CursorToString
 * converters.
 */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include	<X11/Xmu/Drawing.h>

/*
 * XmuConvertStringToBitmap:
 *
 * creates a depth-1 Pixmap suitable for window manager icons.
 * "string" represents a bitmap(1) filename which may be absolute,
 * or relative to the global resource bitmapFilePath, class
 * BitmapFilePath.  If the resource is not defined, the default
 * value is the build symbol BITMAPDIR.
 *
 * shares lots of code with XmuConvertStringToCursor.  
 *
 * To use, include the following in your ClassInitialize procedure:

static XtConvertArgRec screenConvertArg[] = {
    {XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
							sizeof(Screen *)}
};

    XtAddConverter("String", "Bitmap", XmuCvtStringToBitmap,
		   screenConvertArg, XtNumber(screenConvertArg));
 *
 */


static Boolean
XukcCvtStringToBitmap(display, args, num_args, fromVal, toVal, conv_data)
Display *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr	fromVal;
XrmValuePtr	toVal;
XtPointer 	*conv_data;
{
    static Pixmap pixmap;		/* static for cvt magic */
    char *name = (char *)fromVal->addr;

    if (*num_args != 1)
	ErrorMessage("cvtStringToBitmap",
		     "String to Bitmap conversion needs screen argument");

    *conv_data = NULL;
    /* these three are special cases */
    if (strcmp(name, "Unspecified") == 0) {
	pixmap = XtUnspecifiedPixmap;
	done(Pixmap, pixmap);
    }

    if (strcmp(name, "None") == 0) {
	pixmap = None;
	done(Pixmap, pixmap);
    }

    if (strcmp(name, "ParentRelative") == 0) {
	pixmap = ParentRelative;
	done(Pixmap, pixmap);
    }

    pixmap = XmuLocateBitmapFile (*((Screen **) args[0].addr), name,
				  NULL, 0, NULL, NULL, NULL, NULL);

    if (pixmap != None) {
	*conv_data = save_pixmap_cvt(name, pixmap);
	done(Pixmap, pixmap);
    }

    XtDisplayStringConversionWarning(display, name, "Bitmap");
    return FALSE;
}


#include	<X11/CoreP.h>		/* just to do XtConvert() */
#include	<sys/param.h>		/* just to get MAXPATHLEN */
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

/*
 * XmuConvertStringToCursor:
 *
 * allows String to specify a standard cursor name (from cursorfont.h), a
 * font name and glyph index of the form "FONT fontname index [[font] index]", 
 * or a bitmap file name (absolute, or relative to the global resource
 * bitmapFilePath, class BitmapFilePath).  If the resource is not
 * defined, the default value is the build symbol BITMAPDIR.
 *
 * shares lots of code with XmuCvtStringToPixmap, but unfortunately
 * can't use it as the hotspot info is lost.
 *
 * To use, include the following in your ClassInitialize procedure:

static XtConvertArgRec screenConvertArg[] = {
    {XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
							sizeof(Screen *)}
};

    XtAddConverter("String", "Cursor", XmuCvtStringToCursor,      
		   screenConvertArg, XtNumber(screenConvertArg));
 *
 */


#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif

#define FONTSPECIFIER		"FONT "

/*ARGSUSED*/
static Boolean
XukcCvtStringToCursor(display, args, num_args, fromVal, toVal, conv_data)
Display *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr	fromVal;
XrmValuePtr	toVal;
XtPointer	*conv_data;
{
    static Cursor cursor;		/* static for cvt magic */
    char *name = (char *)fromVal->addr;
    Screen *screen;
    register int i;
    char maskname[MAXPATHLEN];
    Pixmap source, mask;
    static XColor bgColor = {0, ~0, ~0, ~0};  /* XXX - make a resource */
    static XColor fgColor = {0, 0, 0, 0};     /* XXX - ditto */
    int xhot, yhot;
    int len;

    if (*num_args != 1)
	ErrorMessage("cvtStringToCursor",
		     "String to cursor conversion needs screen argument");

    screen = *((Screen **) args[0].addr);

    if (strcmp(name, "None") == 0) {
	cursor = None;
	*conv_data = save_cursor_cvt(name, cursor);
	done(Cursor, cursor);
    }

    if (0 == strncmp(FONTSPECIFIER, name, strlen(FONTSPECIFIER))) {
	char source_name[MAXPATHLEN], mask_name[MAXPATHLEN];
	int source_char, mask_char, fields;
	WidgetRec widgetRec;
	Font source_font, mask_font;
	XrmValue fromString, toFont;

	fields = sscanf(name, "FONT %s %d %s %d",
			source_name, &source_char,
			mask_name, &mask_char);
	if (fields < 2) {
	    XtDisplayStringConversionWarning(display, name, "Cursor");
	    return FALSE;
	}

	/* widgetRec is stupid; we should just use XtDirectConvert,
	 * but the names in Xt/Converters aren't public. */
	widgetRec.core.screen = screen;
	fromString.addr = source_name;
	fromString.size = strlen(source_name);
	XtConvert(&widgetRec, XtRString, &fromString, XtRFont, &toFont);
	if (toFont.addr == NULL) {
	    XtDisplayStringConversionWarning(display, name, "Cursor");
	    return FALSE;
	}
	source_font = *(Font*)toFont.addr;

	switch (fields) {
	  case 2:		/* defaulted mask font & char */
	    mask_font = source_font;
	    mask_char = source_char;
	    break;

	  case 3:		/* defaulted mask font */
	    mask_font = source_font;
	    mask_char = atoi(mask_name);
	    break;

	  case 4:		/* specified mask font & char */
	    fromString.addr = mask_name;
	    fromString.size = strlen(mask_name);
	    XtConvert(&widgetRec, XtRString, &fromString, XtRFont, &toFont);
	    if (toFont.addr == NULL) {
		XtDisplayStringConversionWarning( display, name, "Cursor" );
		return FALSE;
	    }
	    mask_font = *(Font*)toFont.addr;
	}

	cursor = XCreateGlyphCursor( DisplayOfScreen(screen), source_font,
				     mask_font, source_char, mask_char,
				     &fgColor, &bgColor );
	*conv_data = save_cursor_cvt(name, cursor);
	done(Cursor, cursor);
    }

    i = XmuCursorNameToIndex (name);
    if (i != -1) {
	cursor = XCreateFontCursor (DisplayOfScreen(screen), i);
	*conv_data = save_cursor_cvt(name, cursor);
	done(Cursor, cursor);
    }

    if ((source = XmuLocateBitmapFile (screen, name, 
				       maskname, (sizeof maskname) - 4,
				       NULL, NULL, &xhot, &yhot)) == None) {
	XtDisplayStringConversionWarning (display, name, "Cursor");
	return FALSE;
    }
    len = strlen (maskname);
    for (i = 0; i < 2; i++) {
	strcpy (maskname + len, i == 0 ? "Mask" : "msk");
	if ((mask = XmuLocateBitmapFile (screen, maskname, NULL, 0, 
					 NULL, NULL, NULL, NULL)) != None)
	  break;
    }

    cursor = XCreatePixmapCursor( DisplayOfScreen(screen), source, mask,
				  &fgColor, &bgColor, xhot, yhot );
    XFreePixmap( DisplayOfScreen(screen), source );
    if (mask != None) XFreePixmap( DisplayOfScreen(screen), mask );

    *conv_data = save_cursor_cvt(name, cursor);
    done(Cursor, cursor);
}

/****************************************************************/

static XtConvertArgRec widgetConvertArg[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.self),
							sizeof(Widget)}
};

static XtConvertArgRec parentConvertArg[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.parent),
							sizeof(Widget)}
};

/*ARGSUSED*/
static Boolean
XukcCvtStringToWidget(display, args, num_args, fromVal, toVal, conv_data)
Display *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr	fromVal;
XrmValuePtr	toVal;
XtPointer	*conv_data;
{
	static Widget widget;		/* return address for convert */
	Widget parent;

	if (*num_args != 1)
		ErrorMessage("cvtStringToWidget",
		     "String to Widget conversion needs needs parent argument");

	parent = *(Widget *)args[0].addr;
	widget = NULL;
	widget = XtNameToWidget(parent, (String)fromVal->addr);
	if (widget == NULL && strcmp((String)fromVal->addr, "NULL")) {
		while (XtParent(parent) != NULL)
			parent = XtParent(parent);
		widget = XtNameToWidget(parent, (String)fromVal->addr);
		if (widget == NULL) {
			XtDisplayStringConversionWarning(display,
					(String)fromVal->addr, "Widget");
			return FALSE;
		}
	}
	done(Widget, widget);
}

/******************** Initialization Routine ************************/

#ifdef HP_WIDGETS
void
XukcAddHPConverters()
{
	/* these are for the HP Widget set only */

	XtSetTypeConverter(XtRAlignment, XtRString, XukcCvtNamedValueToString,
			xhp_alignment_Args, XtNumber(xhp_alignment_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRDestroyMode, XtRString,
			XukcCvtNamedValueToString,
			xhp_destroy_mode_Args, XtNumber(xhp_destroy_mode_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRElementHighlight, XtRString,
			XukcCvtNamedValueToString,
			xhp_element_highlight_Args,
			XtNumber(xhp_element_highlight_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRGravity, XtRString, XukcCvtNamedValueToString,
			xhp_gravity_Args, XtNumber(xhp_gravity_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRGrow, XtRString, XukcCvtNamedValueToString,
			xhp_grow_Args, XtNumber(xhp_grow_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRHighlightStyle, XtRString,
			XukcCvtNamedValueToString,
			xhp_highlight_style_Args,
			XtNumber(xhp_highlight_style_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

#if 0
	XtSetTypeConverter(XtRImage, XtRString, XukcCvtNamedValueToString,
			xhp_image_Args, XtNumber(xhp_image_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
#endif 0

	XtSetTypeConverter(XtRLabelLocation, XtRString,
			XukcCvtNamedValueToString,
			xhp_label_location_Args,
			XtNumber(xhp_label_location_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRLabelType, XtRString, XukcCvtNamedValueToString,
			xhp_label_type_Args, XtNumber(xhp_label_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRLayout, XtRString, XukcCvtNamedValueToString,
			xhp_layout_Args, XtNumber(xhp_layout_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRLayoutType, XtRString, XukcCvtNamedValueToString,
			xhp_layout_type_Args, XtNumber(xhp_layout_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

#if 0
	XtSetTypeConverter(XtRMode, XtRString, XukcCvtNamedValueToString,
			xhp_mode_Args, XtNumber(xhp_mode_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
#endif 0

	XtSetTypeConverter(XtRScroll, XtRString, XukcCvtNamedValueToString,
			xhp_scroll_Args, XtNumber(xhp_scroll_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRSelectionBias, XtRString,
			XukcCvtNamedValueToString,
			xhp_selection_bias_Args,
			XtNumber(xhp_selection_bias_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRSelectionMethod, XtRString,
			XukcCvtNamedValueToString,
			xhp_selection_method_Args,
			XtNumber(xhp_selection_method_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRSelectionStyle, XtRString,
			XukcCvtNamedValueToString,
			xhp_selection_style_Args,
			XtNumber(xhp_selection_style_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRSeparatorType, XtRString,
			XukcCvtNamedValueToString,
			xhp_separator_type_Args,
			XtNumber(xhp_separator_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRSourceType, XtRString, XukcCvtNamedValueToString,
			xhp_source_type_Args, XtNumber(xhp_source_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRTitlePositionType, XtRString,
			XukcCvtNamedValueToString,
			xhp_title_position_type_Args,
			XtNumber(xhp_title_position_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRTitleType, XtRString, XukcCvtNamedValueToString,
			xhp_title_type_Args, XtNumber(xhp_title_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRTraversalType, XtRString,
			XukcCvtNamedValueToString,
			xhp_traversal_type_Args,
			XtNumber(xhp_traversal_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRWrap, XtRString, XukcCvtNamedValueToString,
			xhp_wrap_Args, XtNumber(xhp_wrap_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRWrapBreak, XtRString, XukcCvtNamedValueToString,
			xhp_wrap_break_Args, XtNumber(xhp_wrap_break_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRWrapForm, XtRString, XukcCvtNamedValueToString,
			xhp_wrap_form_Args, XtNumber(xhp_wrap_form_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRArrowDirection, XtRString,
			XukcCvtNamedValueToString,
			xhp_arrow_direction_Args,
			XtNumber(xhp_arrow_direction_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRSlideOrientation, XtRString,
			XukcCvtNamedValueToString,
			xhp_slide_orientation_Args,
			XtNumber(xhp_slide_orientation_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRTileType, XtRString, XukcCvtNamedValueToString,
			xhp_tile_type_Args, XtNumber(xhp_tile_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRRCMode, XtRString, XukcCvtNamedValueToString,
			xhp_rc_mode_Args, XtNumber(xhp_rc_mode_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
}
#endif /* HP_WIDGETS */


#ifdef XAW
void
XukcAddXawConverters()
{
 	/* these are the Athena Widget Set only */

	XtSetTypeConverter(XtRInitialState, XtRString,
			XukcCvtNamedValueToString, xt_initial_state_Args,
			XtNumber(xt_initial_state_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	/* really Xt */
	XtSetTypeConverter(XtRJustify, XtRString, XukcCvtNamedValueToString,
			xt_justify_Args, XtNumber(xt_justify_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtROrientation, XtRString,
			XukcCvtNamedValueToString,
			xaw_orientation_Args, XtNumber(xaw_orientation_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRScrollMode, XtRString, XukcCvtNamedValueToString,
			xaw_text_scroll_Args, XtNumber(xaw_text_scroll_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRString, XtRShapeStyle, XukcCvtStringToNamedValue,
			xaw_shape_style_Args, XtNumber(xaw_shape_style_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRShapeStyle, XtRString, XukcCvtNamedValueToString,
			xaw_shape_style_Args, XtNumber(xaw_shape_style_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRWrapMode, XtRString, XukcCvtNamedValueToString,
			xaw_text_wrap_Args, XtNumber(xaw_text_wrap_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRResizeMode, XtRString, XukcCvtNamedValueToString,
			xaw_text_resize_Args, XtNumber(xaw_text_resize_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtREditMode, XtRString, XukcCvtNamedValueToString,
			xaw_edit_modes_Args, XtNumber(xaw_edit_modes_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRAsciiType, XtRString, XukcCvtNamedValueToString,
			xaw_ascii_type_Args, XtNumber(xaw_ascii_type_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtREdgeType, XtRString, XukcCvtNamedValueToString,
			xaw_form_edges_Args, XtNumber(xaw_form_edges_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
}
#endif /* XAW */


void
XukcOverrideConverters()
{
	XtSetTypeConverter(XtRString, XtRPosition,
			XukcCvtStringToPosition, (XtConvertArgRec *)screen_Args,
			XtNumber(screen_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)NULL);

	XtSetTypeConverter(XtRString, XtRDimension,
			XukcCvtStringToDimension,
			(XtConvertArgRec *)screen_Args, XtNumber(screen_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)NULL);

	XtSetTypeConverter(XtRString, XtRCursor, XukcCvtStringToCursor,
			(XtConvertArgRec *)screen_Args, XtNumber(screen_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)free_cursor_entry);

	XtSetTypeConverter(XtRString, XtRBitmap, XukcCvtStringToBitmap,
			(XtConvertArgRec *)screen_Args, XtNumber(screen_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)free_pix_entry);

	XtSetTypeConverter(XtRString, XtRWidget, XukcCvtStringToWidget,
				(XtConvertArgRec *)parentConvertArg,
				XtNumber(parentConvertArg),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
				XtCacheAll,
				(XtDestructor)NULL);

	XtSetTypeConverter(XtRString, XtRFontStruct, CvtStringToFontStruct,
				(XtConvertArgRec *)screen_Args,
				XtNumber(screen_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
				XtCacheByDisplay,
				(XtDestructor)FreeFontStruct);
}


void
XukcAddConverters()
{
	static Boolean initialized = FALSE;

	if (initialized)
		return;
	else
		initialized = TRUE;

	/**** Pixmap/Bitmap/Cursor <-> String Conversion ****/

	XukcRegisterPixmapConverters(XtRPixmap);

	XtSetTypeConverter(XtRCursor, XtRString, XukcCvtCursorToString,
			(XtConvertArgRec *)NULL, 0, XtCacheNone,
			(XtDestructor)NULL);

	XtSetTypeConverter(XtRBitmap, XtRString, XukcCvtPixmapToString,
			(XtConvertArgRec *)NULL, 0, XtCacheNone,
			(XtDestructor)NULL);

	/**********************/

	XtSetTypeConverter(XtRVisual, XtRString,
			XukcCvtNamedValueToString,
			xt_visual_Args, XtNumber(xt_visual_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay, (XtDestructor)NULL);

	XtSetTypeConverter(XtRCardinal, XtRString, XukcCvtIntegerToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll,
			(XtDestructor)NULL);

	XtSetTypeConverter(XtRFloat, XtRString, XukcCvtFloatToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRFontStruct, XtRString, XukcCvtFontStructToString,
			(XtConvertArgRec *)NULL, 0, XtCacheNone,
			(XtDestructor)NULL);

	XtSetTypeConverter(XtRAtom, XtRString, XukcCvtAtomToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)NULL);

#ifdef XMU
	XtSetTypeConverter(XtRBackingStore, XtRString,
			XukcCvtNamedValueToString,
			xmu_backing_store_Args,
			XtNumber(xmu_backing_store_Args),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);
#endif /* XMU */

	XtSetTypeConverter(XtRWidget, XtRString, XukcCvtWidgetToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRBoolean, XtRString, XukcCvtBooleanToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRBool, XtRString, XukcCvtBooleanToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRLongBoolean, XtRString, XukcCvtBooleanToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRPixel, XtRString, XukcCvtPixelToString,
			(XtConvertArgRec *)pixelConvertArgs,
			XtNumber(pixelConvertArgs),
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheByDisplay,
			(XtDestructor)NULL);

	XtSetTypeConverter(XtRUnsignedChar, XtRString,
			XukcCvtUnsignedIntToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRShort, XtRString, XukcCvtIntegerToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRInt, XtRString, XukcCvtIntegerToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRDimension, XtRString, XukcCvtIntegerToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRPosition, XtRString, XukcCvtIntegerToString,
			(XtConvertArgRec *)NULL, 0,
#ifdef CACHE_REF_COUNT
			XtCacheRefCount |
#endif /* CACHE_REF_COUNT */
			XtCacheAll, (XtDestructor)NULL);

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4

	XtSetTypeConverter(XtRTranslationTable, XtRString,
			XukcCvtTranslationsToString,
			(XtConvertArgRec *)widgetConvertArg,
			XtNumber(widgetConvertArg),
			XtCacheNone, (XtDestructor)NULL);

	XtSetTypeConverter(XtRAcceleratorTable, XtRString,
			XukcCvtTranslationsToString,
			(XtConvertArgRec *)widgetConvertArg,
			XtNumber(widgetConvertArg),
			XtCacheNone, (XtDestructor)NULL);
#else
	/* X11R4 */
	XtSetTypeConverter(XtRTranslationTable, XtRString,
			XukcCvtTranslationsToString,
			(XtConvertArgRec *)NULL, 0,
			XtCacheNone, (XtDestructor)NULL);

	XtSetTypeConverter(XtRAcceleratorTable, XtRString,
			XukcCvtTranslationsToString,
			(XtConvertArgRec *)NULL, 0,
			XtCacheNone, (XtDestructor)NULL);

#endif

#ifdef HP_WIDGETS
	XukcAddHPConverters();
#endif /* HP_WIDGETS */

#ifdef XAW
	XukcAddXawConverters();
#endif /* XAW */

#ifdef MOTIF_WIDGETS
	XukcAddMotifConverters();
#endif /* MOTIF_WIDGETS */
}
