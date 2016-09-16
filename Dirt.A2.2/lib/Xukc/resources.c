#ifndef lint
static char sccid[] = "@(#)resources.c	1.1 (UKC) 5/8/92";
#endif /* !lint */

/* 
 * Copyright 1992 Richard Hesketh / rlh2@ukc.ac.uk
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

/* Routines to fiddle with resource databases. Currently only one, which
 * merges in a programs bitmaps directory into BitmapFilePath resource
 * for XmuLocateBitmap().
 */

#include <X11/Intrinsic.h>

void
XukcAddToBitmapFilePath(rdb, app_name, app_class, pathname)
XrmDatabase *rdb;
String app_name, app_class, pathname;
{
	XrmValue entry;
	char res_name[200];
	char res_class[200];
	char *dummy, *new = NULL;
	int len;

	(void)sprintf(res_name, "%s.bitmapFilePath", app_name);
	(void)sprintf(res_class, "%s.BitmapFilePath", app_class);

	if (XrmGetResource(*rdb, res_name, res_class, &dummy, &entry) &&
	    (char *)entry.addr != (char *)0) {
		dummy = (char *)entry.addr;
		len = strlen(dummy) + strlen(pathname) + 2;
		new = XtMalloc(len);
		(void)sprintf(new, "%s:%s", dummy, pathname);
		entry.addr = new;
	} else
		entry.addr = pathname;

	entry.size = strlen(entry.addr) + 1;
	(void)XrmPutResource(rdb, ".bitmapFilePath", "String", &entry);
	if (new != NULL) XtFree(new);
}
