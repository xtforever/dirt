#ifndef lint
static char sccsid[] = "@(#)xt_extensions.c	1.2 (UKC) 5/10/92";
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

/* routines that ought to be but aren't or aren't done correctly in Xt */

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

/* UNALIGNED will be defined in the Makefile for BIGENDIAN machines only */

/* If the alignment characteristics of your machine are right, these may be
   faster */

#ifdef UNALIGNED

void
XukcCopyFromArgVal(src, dst, size)
XtArgVal src;
char* dst;
register unsigned int size;
{
    if	    (size == sizeof(long))	*(long *)dst = (long)src;
    else if (size == sizeof(short))	*(short *)dst = (short)src;
    else if (size == sizeof(char))	*(char *)dst = (char)src;
    else if (size == sizeof(char*))	*(char **)dst = (char*)src;
    else if (size == sizeof(XtPointer))	*(XtPointer *)dst = (XtPointer)src;
    else if (size == sizeof(XtArgVal))	*(XtArgVal *)dst = src;
    else if (size > sizeof(XtArgVal))
	bcopy((char *)  src, (char *) dst, (int) size);
    else
	bcopy((char *) &src, (char *) dst, (int) size);
} /* CopyFromArg */


void
XukcCopyToArgVal(src, dst, size)
char* src;
XtArgVal *dst;
register unsigned int size;
{
	if	(size == sizeof(long))	   *dst = (XtArgVal)(*(long*)src);
	else if (size == sizeof(short))    *dst = (XtArgVal)(*(short*)src);
	else if (size == sizeof(char))	   *dst = (XtArgVal)(*(char*)src);
	else if (size == sizeof(char*))    *dst = (XtArgVal)(*(char**)src);
	else if (size == sizeof(XtPointer))  *dst = (XtArgVal)(*(XtPointer*)src);
	else if (size == sizeof(XtArgVal)) *dst = *(XtArgVal*)src;
	else bcopy((char*)src, (char*)dst, (int)size);
} /* CopyToArg */

#else

void
XukcCopyFromArgVal(src, dst, size)
XtArgVal src;
char* dst;
register unsigned int size;
{
    if (size > sizeof(XtArgVal))
	bcopy((char *)  src, (char *) dst, (int) size);
    else {
	union {
	    long	longval;
	    short	shortval;
	    char	charval;
	    char*	charptr;
	    XtPointer	ptr;
	} u;
	char *p = (char*)&u;
	if	(size == sizeof(long))	    u.longval = (long)src;
	else if (size == sizeof(short))	    u.shortval = (short)src;
	else if (size == sizeof(char))	    u.charval = (char)src;
	else if (size == sizeof(char*))	    u.charptr = (char*)src;
	else if (size == sizeof(XtPointer))   u.ptr = (XtPointer)src;
	else				    p = (char*)&src;

	bcopy(p, (char *) dst, (int) size);
    }
} /* CopyFromArg */


void
XukcCopyToArgVal(src, dst, size)
char* src;
XtArgVal *dst;
register unsigned int size;
{
	bcopy((char*)src, (char*)dst, (int)size);
} /* CopyToArg */

#endif


XukcRegisterApplicationDefaults(dpy, defs, n_defs)
Display *dpy;
String defs[];
Cardinal n_defs;
{
	XrmDatabase rdb = NULL;
	Cardinal i;

	for (i = 0; i < n_defs; i++)
		XrmPutLineResource(&rdb, defs[i]);

	if (rdb != NULL) {
#if 1
		XrmMergeDatabases(dpy->db, &rdb);
		dpy->db = rdb;   /* original dpy->db has been destroyed */
#else
		XrmMergeDatabases(rdb, &dpy->db);
#endif
	}
}


void
XukcAppConditionalMainLoop(app, condition)
XtAppContext app;
Boolean *condition;
{
	XEvent event;

	while (*condition) {
		XtAppNextEvent(app, &event);
		XtDispatchEvent(&event);
	}
}
