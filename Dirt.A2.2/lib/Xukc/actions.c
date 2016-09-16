#ifndef lint
/* From:   "$Xukc: actions.c,v 1.6 91/12/17 10:01:27 rlh2 Rel $"; */
static char sccsid[] = "@(#)actions.c	1.2 (UKC) 5/10/92";
#endif /* !lint */


#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

#include <X11/Xukc/memory.h>

#if defined(XtSpecificationRelease) && XtSpecificationRelease < 5


/* ||| HAZARD WARNING ||| */

typedef struct {
        String name;
        XrmQuark signature;
        XtActionProc proc;
} CompiledAction, *CompiledActionTable;

typedef struct _ActionListRec *ActionList;
typedef struct _ActionListRec {
        ActionList          next;
        CompiledActionTable table;
} ActionListRec;


/* This is taken straight out of Xt/InitialI.h and really is private to Xt
 * we need it hear through to look through action lists.
 */
/* $XConsortium: InitialI.h,v 1.32 90/07/15 21:40:45 swick Exp $ */

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

/* NOTE: these are not the original definitions as given in Xt/Initial.h */

typedef struct _TimerEventRec {
	char mumble;
} TimerEventRec;

typedef struct _InputEvent {
	char mumble;
} InputEvent;

typedef struct _WorkProcRec {
	char mumble;
} WorkProcRec;


typedef struct 
{
	char mumble;
} FdStruct;

typedef struct _ProcessContextRec {
	char mumble;
} ProcessContextRec, *ProcessContext;

typedef struct _XtAppStruct {
    XtAppContext next;		/* link to next app in process context */
    ProcessContext junk1;
    caddr_t junk2;
    Display **junk3;
    TimerEventRec *junk4;
    WorkProcRec *junk5;
    InputEvent **junk6;
    InputEvent *junk7;
    XrmDatabase junk8;
    XtErrorMsgHandler junk9, junk10;
    XtErrorHandler junk11, junk12;
    struct _ActionListRec *action_table;
    caddr_t junk13;
    unsigned long junk14;
    FdStruct junk15;
    short junk16;
    short junk17;
    short junk18;
    Boolean sync, being_destroyed, error_inited, in_phase2_destroy;
    caddr_t junk19;
    String *junk20;
    struct _ActionHookRec* junk21;
} XtAppStruct;


/************************************************************************/

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

static int
compare_strings(a, b)
String a, b;
{
	return (strcmp(*(String *)a, *(String *)b));
}

/***** Module Exported Routines *****/

String *
XukcWidgetClassActionList(app, wc, num_names)
XtAppContext app;
WidgetClass wc;
Cardinal *num_names;
{
	ActionList rec;
	CompiledActionTable cAT;
	int i;
	Cardinal j = 0;
	String *ret_str, *tmp;

    /* get names of all the actions in the global/application action tables */
	for (rec = app->action_table; rec != NULL; rec = rec->next) {
		cAT = rec->table;

		/* look through the current action table */
		for (i = 0; cAT[i].name != NULL; i++) {
			if (j == 0)
				XukcAddObjectToList(&j,
						(XtPointer)cAT[i].name, FALSE);
			else {
				Cardinal k, l;
				XtPointer *list = XukcGetListPointer(j, &k);

				for (l = 0; l < k; l++)
					if (strcmp((char *)list[l],
							cAT[i].name) == 0)
						break;

				if (l == k) /* not so add */
					XukcAddObjectToList(&j,
						(XtPointer)cAT[i].name, FALSE);
			}
		}
	}

	/* get names of all the actions in the specific widget's class */
	/* gets the names from the "actions" field of the core_class for
	 * each sub-class .. chained up to the topmost superclass */
	for (; wc != NULL; wc = wc->core_class.superclass) {
		cAT = (CompiledActionTable)wc->core_class.actions;

		for (i = wc->core_class.num_actions-1; i >= 0; i--) {
			if (j == 0)
				XukcAddObjectToList(&j, (XtPointer)cAT[i].name,
								FALSE);
			else {
				Cardinal k, l;
				XtPointer *list = XukcGetListPointer(j, &k);

				for (l = 0; l < k; l++)
					if (strcmp((char *)list[l],
						cAT[i].name) == 0)
						break;

				if (l == k) /* not so add */
					XukcAddObjectToList(&j,
						(XtPointer)cAT[i].name, FALSE);
			}
		}
	}

	/* sort the names alphabetically */
	ret_str = (String *)XukcGetListPointer(j, &i);
	*num_names = i;
	qsort(ret_str, i, sizeof(String), compare_strings);

	tmp = (String *)XtMalloc(sizeof(String) * *num_names);
	for (i = 0; i < *num_names; i++)
		tmp[i] = ret_str[i];
	XukcDestroyList(&j, FALSE);

	return (tmp);
}

#endif /* X11R4 Intrinsics only */

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4

/* This is the X11R5 version */

/* ||| HAZARD WARNING ||| */

typedef unsigned short TMShortCard;

typedef struct {
        XrmQuark signature;
        XtActionProc proc;
} CompiledAction, *CompiledActionTable;

typedef struct _ActionListRec *ActionList;
typedef struct _ActionListRec {
        ActionList          next;
        CompiledActionTable table;
	TMShortCard	    count;
} ActionListRec;


/* This is taken straight out of Xt/InitialI.h and really is private to Xt
 * we need it hear through to look through action lists.
 */
/* $XConsortium: InitialI.h,v 1.61 91/07/12 12:33:51 rws Exp $ */

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

/* NOTE: these are not the original definitions as given in Xt/Initial.h */

typedef struct _TimerEventRec {
	char mumble;
} TimerEventRec;

typedef struct _InputEvent {
	char mumble;
} InputEvent;

typedef struct _WorkProcRec {
	char mumble;
} WorkProcRec;


typedef struct 
{
	char mumble;
} FdStruct;


typedef struct _LangProcRec {
	XtLanguageProc      proc;
	XtPointer           closure;
} LangProcRec;

typedef struct _ProcessContextRec {
	char mumble;
} ProcessContextRec, *ProcessContext;

typedef struct {
	char*       start;
	char*       current;
	int         bytes_remaining;
} Heap;

typedef struct _DestroyRec DestroyRec;


typedef struct _XtAppStruct {
    XtAppContext next;		/* link to next app in process context */
    ProcessContext junk1;
    caddr_t *junk2;
    Display **junk3;
    TimerEventRec *junk4;
    WorkProcRec *junk5;
    InputEvent **junk6;
    InputEvent *junk7;
    XrmDatabase junk8;
    XtErrorMsgHandler junk9, junk10;
    XtErrorHandler junk11, junk12;
    struct _ActionListRec *action_table;
    caddr_t *junk13;
    unsigned long junk14;
    FdStruct junk15;
    short junk16;
    short junk17;
    short junk18;
    short junk19;
    Boolean sync, being_destroyed, error_inited;
#ifndef NO_IDENTIFY_WINDOWS
    Boolean identify_windows;           /* debugging hack */
#endif
    Heap heap;
    String * fallback_resources;        /* Set by XtAppSetFallbackResources. */
    struct _ActionHookRec* action_hook_list;
    int destroy_list_size;              /* state data for 2-phase destroy */
    int destroy_count;
    int dispatch_level;
    DestroyRec* destroy_list;
    Widget in_phase2_destroy;
    LangProcRec langProcRec;
} XtAppStruct;


/************************************************************************/

/* 
 * Copyright 1991 Richard Hesketh / rlh2@ukc.ac.uk
 *                Computing Lab. University of Kent at Canterbury, UK
 *
 * See previous notice.
 */

static int
compare_strings(a, b)
String a, b;
{
	return (strcmp(*(String *)a, *(String *)b));
}

/***** Module Exported Routines *****/

String *
XukcWidgetClassActionList(app, wc, num_names)
XtAppContext app;
WidgetClass wc;
Cardinal *num_names;
{
	ActionList rec;
	CompiledActionTable cAT;
	int i;
	Cardinal j = 0;
	String *ret_str, *tmp;

    /* get names of all the actions in the global/application action tables */
	for (rec = app->action_table; rec != NULL; rec = rec->next) {
		cAT = rec->table;

		/* look through the current action table */
		for (i = 0; i < rec->count; i++) {
			if (j == 0) {
				char *nm = XrmQuarkToString(cAT[i].signature);
				if (nm != NULL)
					XukcAddObjectToList(&j, (XtPointer)nm,
								FALSE);
			} else {
				Cardinal k, l;
				XtPointer *list = XukcGetListPointer(j, &k);
				char *nm;

				nm = XrmQuarkToString(cAT[i].signature);
				for (l = 0; l < k; l++)
					if (strcmp(nm, (char *)list[l]) == 0)
						break;

				if (l == k) /* not so add */
					XukcAddObjectToList(&j,
						(XtPointer)nm, FALSE);
			}
		}
	}

	/* get names of all the actions in the specific widget's class */
	/* gets the names from the "actions" field of the core_class for
	 * each sub-class .. chained up to the topmost superclass */
	for (; wc != NULL; wc = wc->core_class.superclass) {
		cAT = (CompiledActionTable)wc->core_class.actions;

		for (i = wc->core_class.num_actions-1; i >= 0; i--) {
			if (j == 0) {
				char *nm = XrmQuarkToString(cAT[i].signature);
				if (nm != NULL)
					XukcAddObjectToList(&j, (XtPointer)nm,
								FALSE);
			} else {
				Cardinal k, l;
				XtPointer *list = XukcGetListPointer(j, &k);
				char *nm;

				nm = XrmQuarkToString(cAT[i].signature);
				for (l = 0; l < k; l++)
					if (strcmp(nm, (char *)list[l]) == 0)
						break;

				if (l == k) /* not so add */
					XukcAddObjectToList(&j,
						(XtPointer)nm, FALSE);
			}
		}
	}

	/* sort the names alphabetically */
	ret_str = (String *)XukcGetListPointer(j, &i);
	*num_names = i;
	qsort(ret_str, i, sizeof(String), compare_strings);

	tmp = (String *)XtMalloc(sizeof(String) * *num_names);
	for (i = 0; i < *num_names; i++)
		tmp[i] = ret_str[i];
	XukcDestroyList(&j, FALSE);

	return (tmp);
}

#endif /* X11R5 Intrinsics */
