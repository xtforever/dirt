#ifndef lint
static char sccsid[] = "@(#)wc_extra.c	1.2 (UKC) 5/10/92";
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

#include <ctype.h>

/*  -- X Window System includes */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h> 

/*  -- Widget Creation Library includes */
#include <X11/Wc/WcCreate.h>
#include <X11/Wc/WcCreateP.h>

#include "dirt_structures.h"

extern XtAppContext GetUIContext();
extern Cardinal XukcFindObjectOnList();
extern XtPointer XukcGetObjectOnList();
extern String WhichEvent();
extern Boolean ObjectRemoveCallback();
extern Cardinal XukcAddObjectToList();
extern void Output();

#define done(type, value) \
        {                                                       \
            if (toVal->addr != NULL) {                          \
                if (toVal->size < sizeof(type)) {               \
                    toVal->size = sizeof(type);                 \
                    return False;                               \
                }                                               \
                *(type*)(toVal->addr) = (value);                \
            } else {                                            \
                static type static_val;                         \
                static_val = (value);                           \
                toVal->size = sizeof(type);                     \
                toVal->addr = (XtPointer)&static_val;           \
            }                                                   \
            return True;                                        \
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
		static String static_val;			\
		static_val = (value);				\
		toVal->size = size;				\
		toVal->addr = (XtPointer)&static_val;		\
	    }							\
	    return True;					\
	}

static Cardinal wc_callbacks = 0;
static Cardinal wc_quarks = 0;


static void
free_callbacks(app, to, conv_data, args, num_args)
XtAppContext app;
XrmValue *to;
XtPointer conv_data;
XrmValue *args;
Cardinal *num_args;
{
	XtCallbackList callbacks = (XtCallbackList)conv_data;

	while (callbacks && callbacks->callback != NULL) {
		/* free callback_data structure */
		XtFree(callbacks->closure);
		callbacks++;
	}
	XtFree(conv_data);
}


/* Convert Callback List To String */

static Boolean
XukcCvtCallbacksToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	extern String XukcGrowString();
	static String cb_list = NULL;
	Cardinal i = 0, sz = 1;
	XtCallbackList callbacks = NULL;
	struct callback_data *cd;

	if (*num_args != 0)
		XtAppErrorMsg(XtDisplayToApplicationContext(display),
			"XukcCvtCallbacksToString", "wrongParameters",
			"XukcToolkitError",
			"Callback List to String conversion needs no extra parameters",
			(String *)NULL, (Cardinal *)&i);

	callbacks = *(XtCallbackList *)fromVal->addr;

	if (cb_list != NULL) {
		XtFree(cb_list);
		cb_list = NULL;
	}
	while (callbacks && callbacks->callback != NULL) {
		for (i = 0; i < callbacks_num; i++)
			if (callbacks_ptr[i].callback == callbacks->callback)
				break;

		if (i != callbacks_num) {
			cd = (struct callback_data *)callbacks->closure;
			
#ifndef ICRM_FIX /* ||| by default ICRM_FIX is *not* defined ||| */
			if (cd != NULL) {
				for (i = 0; i < callbacks_num; i++)
				   if (callbacks_ptr[i].quark == cd->quark)
				   	break;

				if (i != callbacks_num) {
				   sz += strlen(cd->name);
				   if (cd != NULL && cd->client_data != NULL)
				   	sz += strlen(cd->client_data);
				   sz += 3; /* space for "()<comma>" */
				   cb_list = XukcGrowString(cb_list, sz);
				   cb_list = strcat(cb_list, cd->name);
				   cb_list = strcat(cb_list, "(");
				   if (cd != NULL && cd->client_data != NULL)
				   	cb_list = strcat(cb_list,
							 cd->client_data);
				   cb_list = strcat(cb_list, "),");
				}
			} else {
#else
			{
#endif /* ICRM_FIX - ||| this is only here for testing purposes ||| */
				sz += strlen(callbacks_ptr[i].name);
				sz += 3; /* space for "()<comma>" */
				cb_list = XukcGrowString(cb_list, sz);
				cb_list = strcat(cb_list,
						callbacks_ptr[i].name);
				cb_list = strcat(cb_list, "(),");
			}
		}
		callbacks++;
	}

	if (cb_list == NULL)
		cb_list = XtNewString("");
	else
		/* remove trailing comma */
		cb_list[strlen(cb_list)-1] = '\0';

	done_string(cb_list);
}


/*
 * This code is a modified version of the StringToCallback conversion
 * routine given in the Wc - Widget Creation Library developed by
 * David Smyth.
 *
    -- Convert String To Callback
*******************************************************************************
    This conversion creates a callback list structure from the X resource
    database string in format:

    name(arg),name(arg).....

    Note "name" is not case sensitive, while "arg" may be - it is passed to
    a callback as client data as a null terminated string (first level
    parenthesis stripped off).  Even if nothing is specified e.g.,
    SomeCallback() there is a null terminated string passed as client
    data to the callback.  If it is empty, then it is the null string.

    Note also that the argument CANNOT be converted at this point: frequently,
    the argument refers to a widget which has not yet been created, or
    uses the context of the callback (i.e., WcUnmanageCB( this ) uses the
    widget which invoked the callback).  
*/

static Boolean
XukcCvtStringToCallback(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;
Cardinal    *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
    typedef struct 
    {
	char *nsta,*nend;		/* callback name start, end */
	char *asta,*aend;		/* argument string start, end */
    } Segment;

    static XtCallbackList cb;
    XtCallbackRec	  callback_list[MAX_CALLBACKS];
    int                   callback_num = 0;
    String                string = (char *) fromVal->addr;
    Segment               segs[MAX_CALLBACKS];	
    Segment              *seg;
    register char        *s;
    register int          i,ipar;

/*  -- assume error or undefined input argument */
    if (string == NULL) {
	toVal->size = 0;
	toVal->addr = (XtPointer) NULL;
	return (FALSE);
    }

/*  -- parse input string finding segments   "name(arg)" comma separated */
    ipar = 0;
    seg  = segs;
    seg->nsta = seg->nend = seg->asta = seg->aend = (char*)NULL;

    for ( s=string;  ;  s++ )
    {
	switch (*s)
	{
	case ',':  if ( ipar > 0 ) break;  /* commas in arguments ignored  */
	case NUL:  if ( seg->nend == NULL ) seg->nend = s-1;  /* no argument */
	           seg++;		   /* start the next segment */
    		   seg->nsta = seg->nend = seg->asta = seg->aend = (char*)NULL;
		   break;		   

	case '(':  if ( ipar++ == 0 ) { seg->nend = s-1; seg->asta = s+1; };
	           break;
		   
	case ')':  if ( --ipar == 0 ) { seg->aend = s-1; };
		   break;

	default:   if ( *s > ' '  &&  seg->nsta == NULL )
			/* only start a new segment on non-blank char */
	                seg->nsta = s;
	}
	if (*s == NUL) break;
    }

    if (ipar) {
	XtDisplayStringConversionWarning(display, string,
				"Callback, unbalanced parenthesis");
	return (FALSE);
    }

/*  -- process individual callback string segments "name(arg)" */
    for( seg = segs;  seg->nsta;   seg++)
    {
        char           	  cb_name[MAX_XRMSTRING];
	XtCallbackProc 	  found = (XtCallbackProc)NULL;
	XrmQuark       	  quark;
	register char    *d;

	/* our callback cache names are case insensitive, no white space */
	for ( s=seg->nsta, d=cb_name; s<=seg->nend; )
	   if ( *s > ' ')
             *d++ = (isupper(*s) ) ? tolower (*s++) : *s++;
	   else
	      s++;
	*d   = NUL;

        /* try to locate callback in our cache of callbacks */
        quark = XrmStringToQuark (cb_name);
	for (i=0; i<callbacks_num; i++)
	    if ( callbacks_ptr[i].quark == quark )
	    {
	        register XtCallbackRec *rec = &callback_list[callback_num];
		rec->callback = found = callbacks_ptr[i].callback;
	        rec->closure  = callbacks_ptr[i].closure;
		break;
	    }

	/* we have found a registered callback, process arguments */
	if (found)
	{
	   register char *arg;
	   register int   alen;
	   register XtCallbackRec *rec = &callback_list[callback_num];
	   register struct callback_data *cd;
	   
	   cd = XtNew(struct callback_data);
	   cd->quark = callbacks_ptr[i].quark;
	   cd->name = callbacks_ptr[i].name;
	   cd->client_data = rec->closure;
	   rec->closure = (XtPointer)cd;

	   if ( seg->asta )
	   {
	       alen = (int)seg->aend - (int)seg->asta +1;
	       arg  = XtMalloc(alen+1);
	       strncpy ( arg, seg->asta, alen );
	       arg[alen]    = NUL;
	       cd->client_data = (XtPointer)arg;
	   }
	   else
	   {
	       /* there is always a char[], it may have only a NUL */
	       cd->client_data = (XtPointer)"\0";
	   }
	   callback_num++;
        } else {
           XtDisplayStringConversionWarning(display, cb_name, 
			"Callback, unknown callback name");
	   return (FALSE);
	}
    } /* end for seg loop */

/*  -- terminate the callback list */
    {
	register XtCallbackRec *rec = &callback_list[callback_num];
        rec->callback = NULL;
	rec->closure  = NULL;
	callback_num++;
    }

/*  -- make a permanent copy of the new callback list, and return a pointer */
    cb = (XtCallbackList)XtMalloc( callback_num * sizeof (XtCallbackRec) );
    memcpy ( (char*)cb, (char*)callback_list,  
              callback_num * sizeof (XtCallbackRec));
    *conv_data = (XtPointer)cb; /* free this list after use */
    done(XtCallbackList, cb);
}


/*********** Dirt wrapper versions of Wc Callbacks ***********/

static void
dirtWcCallback(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	Cardinal idx;
	struct callback_data *cd = (struct callback_data *)client_data;
	XtCallbackProc proc;

	idx = XukcFindObjectOnList(wc_quarks, (XtPointer)cd->quark);

	if (idx == 0)
		GiveError("badIndex", "dirtWcCallback", "DirtError",
			"Internal corruption of callback data lists in DirtWcCallback()",
			0);

	proc = (XtCallbackProc)XukcGetObjectOnList(wc_callbacks, idx);
	Output("Callback-> %s(%s) from \"%s\" object\n\tCall data = \"%s\" (String), 0x%x (Hex), %d (Decimal)", 
		6, cd->name, cd->client_data, XtName(w),
		call_data, call_data, call_data);
	(*proc)(w, cd->client_data, call_data);
}


static void
dirtWcExitCB(w, client_data, call_data)
Widget w;
XtPointer client_data, call_data;
{
	struct callback_data *cd = (struct callback_data *)client_data;

	Output("Callback-> WcExitCB(%s) from \"%s\" object\n\tApplication would Exit Here",
		2, cd->client_data, XtName(w));
}


static void
register_Wc_callback(name, proc)
String name;
XtCallbackProc proc;
{
	String lower_case_name;

	WcRegisterCallback(GetUIContext(), name, dirtWcCallback,
				(XtPointer)NULL);
	(void)XukcAddObjectToList(&wc_callbacks, (XtPointer)proc, FALSE);
	lower_case_name = WcLowerCaseCopy(name);
	(void)XukcAddObjectToList(&wc_quarks,
			(XtPointer)XrmStringToQuark(lower_case_name), FALSE);
	XtFree(lower_case_name);
}


/******************** Dirt Wrapper version of Wc Actions *****************/

static void
action_output(w, event, params, num_params, action_name)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
String action_name;
{
        Output("Action \"%s\" called from \"%s\" object", 2, action_name,
								XtName(w));
	Output("\tEvent = %s", 1, WhichEvent(event));
	if (*num_params == 0)
		Output("\tNo parameters passed", 0);
	else {
		Cardinal i;

		Output("\tParameters (%d) = ", 1, *num_params);
		for (i = 0; i < *num_params; i++)
			Output("\t\t\"%s\"", 1, params[i]);
	}

}


static void
dirtWcCreatePopupsACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcCreatePopupsACT");
	WcCreatePopupsACT(w, event, params, num_params);
}


static void
dirtWcCreateChildrenACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcCreateChildrenACT");
	WcCreateChildrenACT(w, event, params, num_params);
}


static void
dirtWcManageACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcManageACT");
	WcManageACT(w, event, params, num_params);
}


static void
dirtWcUnmanageACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcUnmanageACT");
	WcUnmanageACT(w, event, params, num_params);
}


static void
dirtWcManageChildrenACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcManageChildrenACT");
	WcManageChildrenACT(w, event, params, num_params);
}


static void
dirtWcUnmanageChildrenACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcUnmanageChildrenACT");
	WcUnmanageChildrenACT(w, event, params, num_params);
}


static void
dirtWcDestroyACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcDestroyACT");
	WcDestroyACT(w, event, params, num_params);
}


static void
dirtWcSetValueACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcSetValueACT");
	WcSetValueACT(w, event, params, num_params);
}


static void
dirtWcSetTypeValueACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcSetTypeValueACT");
	WcSetTypeValueACT(w, event, params, num_params);
}


static void
dirtWcSetSensitiveACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcSetSensitiveACT");
	WcSetSensitiveACT(w, event, params, num_params);
}


static void
dirtWcSetInsensitiveACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcSetInsensitiveACT");
	WcSetInsensitiveACT(w, event, params, num_params);
}


static void
dirtWcLoadResourceFileACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcLoadResourceFileACT");
	WcLoadResourceFileACT(w, event, params, num_params);
}


static void
dirtWcTraceACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcTraceACT");
	WcTraceACT(w, event, params, num_params);
}


static void
dirtWcPopupACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcPopupACT");
	WcPopupACT(w, event, params, num_params);
}


static void
dirtWcPopupGrabACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcPopupGrabACT");
	WcPopupGrabACT(w, event, params, num_params);
}


static void
dirtWcPopdownACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcPopdownACT");
	WcPopdownACT(w, event, params, num_params);
}


static void
dirtWcMapACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcMapACT");
	WcMapACT(w, event, params, num_params);
}


static void
dirtWcUnmapACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcUnmapACT");
	WcUnmapACT(w, event, params, num_params);
}


static void
dirtWcSystemACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcSystemACT");
	WcSystemACT(w, event, params, num_params);
}


static void
dirtWcExitACT(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	action_output(w, event, params, num_params, "WcExitAct");
	Output("\tApplication would Exit Here", 0);
}


/***************** Module Exported Routines ******************/

void
AddDirtConverters()
{
	static XtConvertArgRec parentConvertArg[] = {
	    {XtWidgetBaseOffset, (XtPointer)XtOffset(Widget, core.parent),
	    sizeof(Widget)}
	};

	/* in convert.c */
	extern Boolean _dirtCvtWidgetToString();
	extern Boolean _dirtCvtStringToWidget();

	XtSetTypeConverter(XtRCallback, XtRString, XukcCvtCallbacksToString,
			NULL, 0, XtCacheNone, (XtDestructor)NULL);
	XtSetTypeConverter(XtRString, XtRCallback, XukcCvtStringToCallback,
			NULL, 0, XtCacheAll,
			(XtDestructor)free_callbacks);
	XtSetTypeConverter(XtRWidget, XtRString, _dirtCvtWidgetToString,
			NULL, 0, XtCacheNone, (XtDestructor)NULL);
	XtSetTypeConverter(XtRString, XtRWidget, _dirtCvtStringToWidget,
			parentConvertArg, XtNumber(parentConvertArg),
			XtCacheNone, (XtDestructor)NULL);
}


void
AddDirtCallbacks()
{
	/* register our versions of the Wc Callbacks.  Our versions are
	 * a simple wrapper that call the Wc Callback but extracts the
	 * the right client_data information first and output a message to
	 * say that the proc has been called.
	 */
	register_Wc_callback("WcCreatePopupsCB", WcCreatePopupsCB);
	register_Wc_callback("WcCreateChildrenCB", WcCreateChildrenCB);
	register_Wc_callback("WcManageCB", WcManageCB);
	register_Wc_callback("WcUnmanageCB", WcUnmanageCB);
	register_Wc_callback("WcManageChildrenCB", WcManageChildrenCB);
	register_Wc_callback("WcUnmanageChildrenCB", WcUnmanageChildrenCB);
	register_Wc_callback("WcDestroyCB", WcDestroyCB);
	register_Wc_callback("WcSetValueCB", WcSetValueCB);
	register_Wc_callback("WcSetTypeValueCB", WcSetTypeValueCB);
	register_Wc_callback("WcSetSensitiveCB", WcSetSensitiveCB);
	register_Wc_callback("WcSetInsensitiveCB", WcSetInsensitiveCB);
	register_Wc_callback("WcLoadResourceFileCB", WcLoadResourceFileCB);
	register_Wc_callback("WcTraceCB", WcTraceCB);
	register_Wc_callback("WcPopupCB", WcPopupCB);
	register_Wc_callback("WcPopupGrabCB", WcPopupGrabCB);
	register_Wc_callback("WcPopdownCB", WcPopdownCB);
	register_Wc_callback("WcMapCB", WcMapCB);
	register_Wc_callback("WcUnmapCB", WcUnmapCB);
	register_Wc_callback("WcSystemCB", WcSystemCB);
	WcRegisterCallback(GetUIContext(), "WcExitCB", dirtWcExitCB,
				(XtPointer)NULL);
}


void
AddDirtActions()
{
	/* register our versions of the Wc Actions.  Our versions are
	 * a simple wrapper that call the Wc Action but outputs a message
	 * first.
	 */
	static XtActionsRec WcActions[] = {
		{"WcCreatePopupsACT", dirtWcCreatePopupsACT },
		{"WcCreateChildrenACT", dirtWcCreateChildrenACT },
		{"WcManageACT", dirtWcManageACT },
		{"WcUnmanageACT", dirtWcUnmanageACT },
		{"WcManageChildrenACT", dirtWcManageChildrenACT },
		{"WcUnmanageChildrenACT", dirtWcUnmanageChildrenACT },
		{"WcDestroyACT", dirtWcDestroyACT },
		{"WcSetValueACT", dirtWcSetValueACT },
		{"WcSetTypeValueACT", dirtWcSetTypeValueACT },
		{"WcSetSensitiveACT", dirtWcSetSensitiveACT },
		{"WcSetInsensitiveACT", dirtWcSetInsensitiveACT },
		{"WcLoadResourceFileACT", dirtWcLoadResourceFileACT },
		{"WcTraceACT", dirtWcTraceACT },
		{"WcPopupACT", dirtWcPopupACT },
		{"WcPopupGrabACT", dirtWcPopupGrabACT },
		{"WcPopdownACT", dirtWcPopdownACT },
		{"WcMapACT", dirtWcMapACT },
		{"WcUnmapACT", dirtWcUnmapACT },
		{"WcSystemACT", dirtWcSystemACT }
	};
	XtAppAddActions(GetUIContext(), WcActions, XtNumber(WcActions));
	WcRegisterAction(GetUIContext(), "WcExitACT", dirtWcExitACT);
}


/* removes all the callbacks on the callback list that point to the
 * default_callback.
 */
void
RemoveWcCallbacks(obj, list_name, callbacks)
ObjectPtr obj;
String list_name;
XtCallbackList callbacks;
{
	while (callbacks && callbacks->callback != NULL) {
		if (callbacks->callback == (XtCallbackProc)dirtWcCallback)
			ObjectRemoveCallback(obj, list_name,
						dirtWcCallback,
						callbacks->closure);
		else if (callbacks->callback == (XtCallbackProc)dirtWcExitCB)
			ObjectRemoveCallback(obj, list_name,
						dirtWcExitCB,
						callbacks->closure);
		callbacks++;
	}
}
