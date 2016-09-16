#ifndef lint
static char sccsid[] = "@(#)CvtTrans2Str.c	1.2 (UKC) 5/10/92";
#endif /* !lint */

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

#include <X11/Intrinsic.h>

#if defined(XtSpecificationRelease) && XtSpecificationRelease < 5

/* Modified from Xt/TMstate.c to allow dynamic editing of translations */
/* Richard Hesketh, rlh2@ukc.ac.uk : Feb'89 */

#include <X11/Xlib.h>
#define XK_MISCELLANY
#define XK_LATIN1

#include <stdio.h>
#include <X11/keysymdef.h>
#include <X11/StringDefs.h>

#include <X11/IntrinsicP.h>

/* The following are from Intrinsics private header files
 *
 * ||| WARNING |||
 *
 * If/When the intrinsics implementation changes we may have to change things
 * in here.  I would like to include all of these definitions and structures
 * via "IntrinsicI.h" but this isn't public and other implementations may
 * not have it.  I don't know what will happen when used with Motif??
 */

/* $XConsortium: TranslateI.h,v 1.23 90/07/13 07:35:51 swick Exp $ */

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

/* 
 * TranslateI.h - Header file private to translation management
 * 
 * Author:	Charles Haynes
 * 		Digital Equipment Corporation
 * 		Western Research Laboratory
 * Date:	Sat Aug 29 1987
 */

/*#define REFCNT_TRANSLATIONS*/
#define CACHE_TRANSLATIONS

#define _XtRStateTablePair "_XtStateTablePair"

typedef Boolean (*MatchProc)();
  /* Event parsed;
     XEvent incoming;
  */

typedef struct _ModToKeysymTable {
    Modifiers mask;
    int count;
    int index;
} ModToKeysymTable;

typedef struct _LateBindings {
    Boolean knot;
    Boolean pair;
    KeySym keysym;
} LateBindings, *LateBindingsPtr;

typedef short ModifierMask;
typedef struct _EventObjRec *EventObjPtr;
typedef struct _EventRec {
    unsigned long modifiers;
    unsigned long modifierMask;
    LateBindingsPtr lateModifiers;
    unsigned long eventType;
    unsigned long eventCode;
    unsigned long eventCodeMask;
    MatchProc matchEvent;
    Boolean standard;
} Event;

typedef enum _TMkind {override,augment} TMkind;

typedef struct _EventObjRec {
    Event event;	/* X event description */
    StatePtr state;	/* pointer to linked lists of state info */
} EventObjRec;

typedef struct _ActionsRec *ActionPtr;
typedef struct _ActionsRec {
    int index;			/* index into quarkTable to find proc */
    String *params;		/* pointer to array of params */
    Cardinal num_params;	/* number of params */
    ActionPtr next;		/* next action to perform */
} ActionRec;

typedef struct _StateRec {
    int index;		/* index of event into EventObj table */
    ActionPtr actions;	/* rhs list of actions to perform */
    StatePtr nextLevel;	/* the next level points to the next event
			   in one event sequence */
    StatePtr next;	/* points to other event state at same level */
    StatePtr forw;	/* points to next state in list of all states */
    Boolean cycle;	/* true iff nextLevel is a loop */
}  StateRec;
typedef enum {XtTableReplace,XtTableAugment,XtTableOverride} _XtTranslateOp;

typedef struct _StateTableData {
    _XtTranslateOp	operation; /*replace,augment,override*/
    unsigned int	numEvents;
    unsigned int	eventTblSize;
    EventObjPtr		eventObjTbl;
    unsigned int	numQuarks;   /* # of entries in quarkTable */
    unsigned int	quarkTblSize; /*total size of quarkTable */
    XrmQuark*		quarkTable;  /* table of quarkified rhs*/
    unsigned int	accNumQuarks;
    unsigned int	accQuarkTblSize;
    XrmQuark*		accQuarkTable;
    StatePtr		head;	/* head of list of all states */
    Boolean		mappingNotifyInterest;
} StateTableData, *StateTablePtr;

typedef struct _XtBoundAccActionRec {
    Widget widget;    /*widgetID to pass to action Proc*/
    XtActionProc proc; /*action procedure */
} XtBoundAccActionRec;

typedef struct _TranslationData {
    StateTablePtr	 stateTable;
    XtBoundAccActionRec* accProcTbl;
} TranslationData;

typedef struct _TMConvertRec {
   StateTablePtr old; /* table to merge into */
   StateTablePtr new; /* table to merge from */
   TMkind  operation; /* merge or augment     */
} TMConvertRec;

#define _XtEventTimerEventType ((unsigned long)-1L)
#define KeysymModMask		(1<<27) /* private to TM */
#define AnyButtonMask		(1<<28)	/* private to TM */

typedef struct _EventSeqRec *EventSeqPtr;
typedef struct _EventSeqRec {
    Event event;	/* X event description */
    StatePtr state;	/* private to state table builder */
    EventSeqPtr next;	/* next event on line */
    ActionPtr actions;	/* r.h.s.   list of actions to perform */
} EventSeqRec;

typedef EventSeqRec EventRec;
typedef EventSeqPtr EventPtr;
typedef struct _TMEventRec {
    XEvent *xev;
    Event event;
}TMEventRec,*TMEventPtr;

extern void _XtAddEventSeqToStateTable();
extern void _XtInitializeStateTable(); /* stateTable */
    /* _XtTranslations *stateTable; */

typedef struct _ActionHookRec {
    struct _ActionHookRec* next; /* must remain first */
    XtAppContext app;
    XtActionHookProc proc;
    XtPointer closure;
} ActionHookRec, *ActionHook;

typedef struct _TMContext {
    XEvent *event;
    unsigned long serial;
    KeySym keysym;
    Modifiers modifiers;
} TMContextRec, *TMContext;

extern Boolean _XtMatchUsingStandardMods();
extern Boolean _XtMatchUsingDontCareMods();
extern Boolean _XtRegularMatch();
extern Boolean _XtMatchAtom();
extern void _XtConvertCase();
extern void _XtBuildKeysymTables();

extern void _XtPopupInitialize();

extern void _XtInstallTranslations(); /* widget, stateTable */
    /* Widget widget; */
    /* XtTranslations stateTable; */

extern void _XtBindActions(); /* widget, stateTable */
    /* Widget widget; */
    /* XtTranslations stateTable; */

extern void _XtTranslateInitialize();

extern XtTranslations _XtParseTranslationTable(); /* source */
    /* String source; */

extern void _XtRegisterGrabs(
#if NeedFunctionPrototypes
    Widget /* widget */,
#if NeedWidePrototypes
    int /* acceleratorsOnly */
#else
    Boolean /* acceleratorsOnly */
#endif
#endif
);

extern void _XtPopup(); /* widget, grab_kind, spring_loaded */
    /* Widget      widget; */
    /* XtGrabKind  grab_kind; */
    /* Boolean     spring_loaded; */

extern XtTranslations _XtCondCopyTranslations(
#if NeedFunctionPrototypes
    XtTranslations	translations
#endif
);


/*************************** End of Xt/TranslateI.h ********************/

#define STR_THRESHOLD 25
#define STR_INCAMOUNT 100
#define CHECK_STR_OVERFLOW \
    if (str - *buf > *len - STR_THRESHOLD) {			  \
	String old = *buf;					  \
	*buf = XtRealloc(old, (Cardinal)(*len += STR_INCAMOUNT)); \
	str = str - old + *buf;					  \
    }

#define ExpandForChars( nchars ) \
    if (str - *buf > *len - STR_THRESHOLD - nchars) { 		\
	String old = *buf;					\
	*buf = XtRealloc(old,					\
	    (Cardinal)(*len += STR_INCAMOUNT + nchars));	\
	str = str - old + *buf;					\
    }

#define ExpandToFit(more) \
    {								\
	int l = strlen(more);					\
	ExpandForChars(l);					\
    }


static String trans = NULL;
static Cardinal trans_length = 0;


/* all the events available in the translation manager */

static String event_names[] = {
  "<Btn1Down>", "<Btn1Motion>", "<Btn1Up>", "<Btn2Down>", "<Btn2Motion>",
  "<Btn2Up>", "<Btn3Down>", "<Btn3Motion>", "<Btn3Up>", "<Btn4Down>",
  "<Btn4Motion>", "<Btn4Up>", "<Btn5Down>", "<Btn5Motion>", "<Btn5Up>",
  "<BtnDown>", "<BtnMotion>", "<BtnUp>", "<ButtonPress>", "<ButtonRelease>",
  "<CirculateNotify>", "<CirculateRequest>", "<ClientMessage>",
  "<ColormapNotify>", "<ConfigureNotify>", "<ConfigureRequest>",
  "<CreateNotify>", "<Ctrl>", "<DestroyNotify>", "<EnterWindow>",
  "<EventTimer>", "<Expose>", "<FocusIn>", "<FocusOut>", "<GraphicsExpose>",
  "<GravityNotify>", "<Key>", "<KeyDown>", "<KeyPress>", "<KeyRelease>",
  "<KeyUp>", "<KeymapNotify>", "<LeaveWindow>", "<MapNotify>", "<MapRequest>",
  "<MappingNotify>", "<Meta>", "<Motion>", "<MotionNotify>", "<MouseMoved>",
  "<NoExpose>", "<PropertyNotify>", "<PtrMoved>", "<ReparentNotify>",
  "<ResizeRequest>", "<SelectionClear>", "<SelectionNotify>",
  "<SelectionRequest>", "<Shift>", "<Timer>", "<UnmapNotify>",
  "<VisibilityNotify>",
};

static String PrintModifiers(buf, len, str, mask, mod)
    String *buf;
    int *len;
    String str;
    unsigned long mask, mod;
{
    Boolean notfirst = False;
    CHECK_STR_OVERFLOW;

    if (mask == ~0L && mod == 0) {
	*str++ = '!';
	*str = '\0';
	return str;
    }

#define PRINTMOD(modmask,modstring) \
    if (mask & modmask) {		 \
	if (! (mod & modmask)) {	 \
	    *str++ = '~';		 \
	    notfirst = True;		 \
	}				 \
	else if (notfirst)		 \
	    *str++ = ' ';		 \
	else notfirst = True;		 \
	strcpy(str, modstring);		 \
	str += strlen(str);		 \
    }

    PRINTMOD(ShiftMask, "Shift");
    PRINTMOD(ControlMask, "Ctrl");	/* name is not CtrlMask... */
    PRINTMOD(LockMask, "Lock");
    PRINTMOD(Mod1Mask, "Mod1");
    PRINTMOD(Mod2Mask, "Mod2");
    PRINTMOD(Mod3Mask, "Mod3");
    PRINTMOD(Mod4Mask, "Mod4");
    PRINTMOD(Mod5Mask, "Mod5");
    PRINTMOD(Button1Mask, "Button1");
    PRINTMOD(Button2Mask, "Button2");
    PRINTMOD(Button3Mask, "Button3");
    PRINTMOD(Button4Mask, "Button4");
    PRINTMOD(Button5Mask, "Button5");

#undef PRINTMOD

    return str;
}

static String PrintButtonEvent(buf, len, str, button, type)
    String *buf;
    int *len;
    register String str;
    unsigned long button, type;
{
	switch (type) {
		case ButtonPress:
			if (button > 0)
				(void)sprintf(str, "<Btn%dDown>", button);
			else
				(void)sprintf(str, "<ButtonPress>");
			break;
		case ButtonRelease:
			if (button > 0)
				(void)sprintf(str, "<Btn%dUp>", button);
			else
				(void)sprintf(str, "<ButtonRelease>");
			break;
		case MotionNotify:
			if (button > 0)
				(void)sprintf(str, "<Btn%dMotion>", button);
			else
				(void)sprintf(str, "<MotionNotify>");
			break;
	}
	str += strlen(str);
	return str;
}


static String PrintEventType(buf, len, str, event)
    String *buf;
    int *len;
    register String str;
    unsigned long event;
{
    CHECK_STR_OVERFLOW;
    switch (event) {
#ifdef __STDC__
#define PRINTEVENT(event) case event: (void)strcpy(str, "<" #event ">"); break;
#else
#define PRINTEVENT(event) case event: (void)strcpy(str, "<event>"); break;
#endif /* __STDC__ */
	PRINTEVENT(KeyPress)
	PRINTEVENT(KeyRelease)
	PRINTEVENT(ButtonPress)
	PRINTEVENT(ButtonRelease)
	PRINTEVENT(MotionNotify)
	PRINTEVENT(EnterNotify)
	PRINTEVENT(LeaveNotify)
	PRINTEVENT(FocusIn)
	PRINTEVENT(FocusOut)
	PRINTEVENT(KeymapNotify)
	PRINTEVENT(Expose)
	PRINTEVENT(GraphicsExpose)
	PRINTEVENT(NoExpose)
	PRINTEVENT(VisibilityNotify)
	PRINTEVENT(CreateNotify)
	PRINTEVENT(DestroyNotify)
	PRINTEVENT(UnmapNotify)
	PRINTEVENT(MapNotify)
	PRINTEVENT(MapRequest)
	PRINTEVENT(ReparentNotify)
	PRINTEVENT(ConfigureNotify)
	PRINTEVENT(ConfigureRequest)
	PRINTEVENT(GravityNotify)
	PRINTEVENT(ResizeRequest)
	PRINTEVENT(CirculateNotify)
	PRINTEVENT(CirculateRequest)
	PRINTEVENT(PropertyNotify)
	PRINTEVENT(SelectionClear)
	PRINTEVENT(SelectionRequest)
	PRINTEVENT(SelectionNotify)
	PRINTEVENT(ColormapNotify)
	PRINTEVENT(ClientMessage)
	case _XtEventTimerEventType: (void) strcpy(str,"<EventTimer>"); break;
	default: (void) sprintf(str, "<0x%x>", (int) event);
#undef PRINTEVENT
    }
    str += strlen(str);
    return str;
}

static String PrintCode(buf, len, str, mask, code)
    String *buf;
    int *len;
    register String str;
    unsigned long mask, code;
{
    CHECK_STR_OVERFLOW;
    if (mask != 0) {
	if (mask != (unsigned long)~0L)
	    (void) sprintf(str, "0x%lx:0x%lx", mask, code);
	else (void) sprintf(str, /*"0x%lx"*/ "%d", code);
	str += strlen(str);
    }
    return str;
}

static String PrintKeysym(buf, len, str, keysym)
    String *buf;
    int *len;
    register String str;
    KeySym keysym;
{
    String keysymName;

    if (keysym == 0) return str;

    CHECK_STR_OVERFLOW;
    keysymName = XKeysymToString(keysym);
    if (keysymName == NULL)
	return PrintCode(buf,len,str,(unsigned long)~0L,(unsigned long)keysym);

    strcpy(str, keysymName);
    return str + strlen(str);
}

static String PrintAtom(buf, len, str, dpy, atom)
    String *buf;
    int *len;
    register String str;
    Display *dpy;
    Atom atom;
{
    String atomName;

    if (atom == 0) return str;

    if (dpy == NULL)
	atomName = NULL;
    else
	atomName = XGetAtomName(dpy, atom);

    if (atomName == NULL)
	return PrintCode(buf, len, str,(unsigned long)~0L,(unsigned long)atom);

    ExpandToFit( atomName );
    strcpy(str, atomName);
    return str + strlen(str);
}

static String PrintLateModifiers(buf, len, str, lateModifiers)
    String *buf;
    int *len;
    register String str;
    LateBindingsPtr lateModifiers;
{
    for (; lateModifiers->keysym != NULL; lateModifiers++) {
	CHECK_STR_OVERFLOW;
	if (lateModifiers->knot) {
	    *str++ = '~';
	} else {
	    *str++ = ' ';
	}
	strcpy(str, XKeysymToString(lateModifiers->keysym));
	str += strlen(str);
	if (lateModifiers->pair) {
	    *(str -= 2) = '\0';	/* strip "_L" */
	    lateModifiers++;	/* skip _R keysym */
	}
    }
    return str;
}

static String PrintEvent(buf, len, str, event, dpy)
    String *buf;
    int *len;
    register String str;
    register Event *event;
    Display *dpy;
{
    if (event->standard) *str++ = ':';

    str = PrintModifiers(buf, len, str, event->modifierMask, event->modifiers);
    if (event->lateModifiers != NULL)
	str = PrintLateModifiers(buf, len, str, event->lateModifiers);
    switch (event->eventType) {
	case KeyPress:
	case KeyRelease:
		str = PrintEventType(buf, len, str, event->eventType);
		str = PrintKeysym(buf, len, str, (KeySym)event->eventCode);
		break;
	case PropertyNotify:
	case SelectionClear:
	case SelectionRequest:
	case SelectionNotify:
	case ClientMessage:
		str = PrintEventType(buf, len, str, event->eventType);
		str = PrintAtom(buf, len, str, dpy, (Atom)event->eventCode);
		break;
	case ButtonPress :
	case ButtonRelease :
	case MotionNotify :
		if (event->eventCodeMask != NULL) {
			str = PrintButtonEvent(buf, len, str, event->eventCode,
							event->eventType);
			break;
		}
	default:
		str = PrintEventType(buf, len, str, event->eventType);
		str = PrintCode(buf, len, str, event->eventCodeMask,
							event->eventCode);
		break;
    }
    return str;
}

static String PrintParams(buf, len, str, params, num_params)
    String *buf;
    int *len;
    register String str, *params;
    Cardinal num_params;
{
    register Cardinal i;
    for (i = 0; i<num_params; i++) {
	ExpandToFit( params[i] );
	if (i != 0) {
	    *str++ = ',';
	    *str++ = ' ';
	}
	*str++ = '"';
	strcpy(str, params[i]);
	str += strlen(str);
	*str++ = '"';
    }
    *str = '\0';
    return str;
}

static String PrintActions(buf, len, str, actions, translateData)
    String *buf;
    int *len;
    register String str;
    register ActionPtr actions;
    XtTranslations translateData;
{
    StateTablePtr stateTable = translateData->stateTable;
    while (actions != NULL) {
	String proc;
	*str++ = ' ';
	if (actions->index >= 0) {
	    /* normal translation */
	    proc = XrmQuarkToString( stateTable->quarkTable[actions->index] );
	} else {
	    /* accelerator */
	    int temp = -(actions->index+1);
	    if (translateData->accProcTbl != NULL) {
		Widget w = translateData->accProcTbl[temp].widget;
		if (w != NULL) {
		    String name = XtName(w);
		    int nameLen = strlen(name);
		    ExpandForChars( nameLen );
		    bcopy( name, str, nameLen );
		    str += nameLen;
		    *str++ = '`';
		}
	    }
	    proc = XrmQuarkToString( stateTable->accQuarkTable[temp] );
	}
	ExpandToFit( proc );
	strcpy(str, proc);
	str += strlen(proc);
	*str++ = '(';
	str = PrintParams(buf, len, str, actions->params, actions->num_params);
	*str++ = ')';
	actions = actions->next;
    }
    (void) sprintf(str, "\n");
    str += strlen(str);
    return str;
}



static Boolean LookAheadForCycleOrMulticlick(state, eot, countP, nextLevelP)
    register StatePtr state;
    EventObjPtr eot;
    int *countP;
    StatePtr *nextLevelP;
{
    int index = state->index;
    int repeatCount = 0;

    *nextLevelP = NULL;
    for (state = state->nextLevel; state != NULL; state = state->nextLevel) {
	if (state->cycle) {
	    *countP = repeatCount;
	    return True;
	}
	if (state->index == index) {
	    repeatCount++;
	    *nextLevelP = state;
	}
	else if (eot[state->index].event.eventType == _XtEventTimerEventType)
	    continue;
	else /* not same event as starting event and not timer */ {
	    Event *start_evP = &eot[index].event;
	    Event *evP = &eot[state->index].event;
	    unsigned int type = eot[index].event.eventType;
	    unsigned int t = eot[state->index].event.eventType;
	    if (   (type == ButtonPress	  && t != ButtonRelease)
		|| (type == ButtonRelease && t != ButtonPress)
		|| (type == KeyPress	  && t != KeyRelease)
		|| (type == KeyRelease	  && t != KeyPress)
		|| evP->eventCode != start_evP->eventCode
		|| evP->modifiers != start_evP->modifiers
		|| evP->modifierMask != start_evP->modifierMask
		|| evP->lateModifiers != start_evP->lateModifiers
		|| evP->eventCodeMask != start_evP->eventCodeMask
		|| evP->matchEvent != start_evP->matchEvent
		|| evP->standard != start_evP->standard)
		/* not inverse of starting event, either */
		break;
	}
    }
    *countP = repeatCount;
    return False;
}

static String PrintState(buf, len, str, start, state, translateData, dpy)
    String *buf;
    int *len;
    register String str;
    int start;			/* offset of current LHS; -1 =>print *buf */
    StatePtr state;
    XtTranslations translateData;
    Display *dpy;
{
    int oldOffset;
    int clickCount;
    Boolean cycle;
    StatePtr nextLevel;
    StatePtr sameLevel;
    StateTablePtr stateTable;

    /* print the current state */
    if (state == NULL) return str;

    oldOffset = str - *buf;
    stateTable = translateData->stateTable;
    sameLevel = state->next;
    str = PrintEvent( buf, len, str,
		      &stateTable->eventObjTbl[state->index].event,
		      dpy );

    cycle = LookAheadForCycleOrMulticlick( state, stateTable->eventObjTbl,
					   &clickCount, &nextLevel )
	|| state->cycle;

    if (cycle || clickCount > 0) {
	if (clickCount > 0)
	    sprintf( str, "(%d%s)", clickCount+1, cycle ? "+" : "" );
	else
	    bcopy("(+)", str, 4);
	str += strlen(str);
	if (state->actions == NULL && nextLevel != NULL)
	    state = nextLevel;
    }

    if (state->actions != NULL) {
	if (start == -1) {
	    int offset = str - *buf;
	    CHECK_STR_OVERFLOW;
	    *str++ = ':';
	    *str = '\0';
	    (void) PrintActions(buf, len, str, state->actions, translateData);
#if 1
	while (*str) str++;
	trans_length += str - *buf;
	trans = XtRealloc(trans, trans_length + 1);
	(void)strcat(trans, *buf);
#endif 1
	    str = *buf + offset; *str = '\0';
	}
    } else {
	*str++ = ',';
	*str = '\0';
    }

    /* print succeeding states */
    if (state->nextLevel != NULL && !state->cycle) {
	if (state->actions && start != -1) {
	    *str++ = '\n';
	    ExpandForChars( oldOffset - start );
	    bcopy( *buf+start, str, oldOffset - start );
	    str += oldOffset - start;
    	}
	str = PrintState( buf, len, str, start,
			  state->nextLevel, translateData, dpy );
    }

    if (sameLevel != NULL) {	/* print sibling states */
	if (start == -1)
	    str = *buf + oldOffset;
	else {
	    *str++ = '\n';
	    ExpandForChars( oldOffset - start );
	    bcopy( *buf+start, str, oldOffset - start );
	    str += oldOffset - start;
	}
	str = PrintState(buf, len, str, start, sameLevel, translateData, dpy);
    }

    if (start == -1) str = *buf + oldOffset;
    *str = '\0';
    return str;
}


static void
TranslateTableDisplay(translateData)
XtTranslations translateData;
{
    StateTablePtr translations;
    register Cardinal i;
    int len = 1000;
    char *buf;
    
    if (translateData == NULL) return;
    translations = translateData->stateTable;
    buf = XtMalloc((Cardinal)1000);

    for (i = 0; i < translations->numEvents; i++) {
	buf[0] = '\0';
	PrintState(
	   &buf,
	   &len,
	   buf,
	   -1,
	   translations->eventObjTbl[i].state,
           translateData,
	   NULL);
    }
    XtFree(buf);
}


/* 
 * Copyright 1990 Richard Hesketh / rlh2@ukc.ac.uk
 *                Computing Lab. University of Kent at Canterbury, UK
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
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
 * RICHARD HESKETH AND THE UNIVERSITY OF KENT AT CANTERBURY DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL RICHARD HESKETH OR THE
 * UNIVERSITY OF KENT AT CANTERBURY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author:  Richard Hesketh / rlh2@ukc.ac.uk, 
 *                Computing Lab. University of Kent at Canterbury, UK
 */

Boolean
XukcCvtTranslationsToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	Cardinal i = 0;

	if (*num_args != 0)
		XtAppErrorMsg(XtDisplayToApplicationContext(display),
			"wrongParameters", "cvtTranslationsToString",
			"XukcToolkitError",
			"Translations to String conversion needs no extra arguments",
			(String *)NULL, (Cardinal *)&i);

	if (trans != NULL)
		XtFree(trans);

	trans_length = 0;

	if (*(XtTranslations *)fromVal->addr == NULL)
		trans = XtNewString("");
	else {
		trans = (String)XtMalloc(1000);
		trans[0] = '\0';
		TranslateTableDisplay(*(XtTranslations *)fromVal->addr);
	}

	trans_length++;

	if (toVal->addr != NULL) {
		if (toVal->size < trans_length) {
			toVal->size = trans_length;
			return FALSE;
		}
		(void)strcpy((char *)toVal->addr, (char *)trans);
		XtFree(trans);
		toVal->size = trans_length;
		trans = NULL;
	} else {
		static String static_val;
		static_val = trans;
		toVal->size = trans_length;
		toVal->addr = (XtPointer)static_val;
	}
	return TRUE;
}

#endif /* X11R4 Intrinsics */

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4


/* Thankfully at R5 the Xt Intrinsics includes routines to print out
 * the Translations table from the internal compiled form. So we use
 * those routines instead of copying in the stuff from Xt/TMstate.c
 */
Boolean
XukcCvtTranslationsToString(display, args, num_args, fromVal, toVal, conv_data)
Display     *display;
XrmValuePtr args;           /* unused */
Cardinal    *num_args;      /* unused */
XrmValuePtr fromVal;
XrmValuePtr toVal;
XtPointer   *conv_data;
{
	extern String _XtPrintXlations();
	static String trans = NULL;
	Cardinal i = 0;

	if (*num_args != 1)
		XtAppErrorMsg(XtDisplayToApplicationContext(display),
			"wrongParameters", "cvtTranslationsToString",
			"XukcToolkitError",
			"Translations to String conversion needs single Widget base offset argument",
			(String *)NULL, (Cardinal *)&i);

	if (trans != NULL)
		XtFree(trans);

	if (*(XtTranslations *)fromVal->addr == NULL)
		trans = XtNewString("");
	else {
		trans = _XtPrintXlations(*(Widget *)args[0].addr,
					 *(XtTranslations *)fromVal->addr,
					 NULL,
					 True);
	}

	i = strlen(trans) + 1;

	if (toVal->addr != NULL) {
		if (toVal->size < i) {
			toVal->size = i;
			return FALSE;
		}
		(void)strcpy((char *)toVal->addr, (char *)trans);
		XtFree(trans);
		toVal->size = i;
		trans = NULL;
	} else {
		toVal->size = i;
		toVal->addr = (XtPointer)trans;
	}
	return TRUE;
}

#endif /* X11R5 Intrinsics */
