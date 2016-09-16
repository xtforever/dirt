#ifndef lint
static char sccsid[] = "@(#)comments.c	1.2 (UKC) 5/10/92";
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

/*
 * copy.c : contains the functions for copying instances of widgets
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/text.h>
#include "dirt_structures.h"

/* externally defined routines */
extern ObjectPtr GetStartOfList();
extern ObjectPtr CurrentlyEditing();
extern void XukcChangeLabel();

extern Widget WI_comment_title, WI_comment_text, WI_help_text;

static ObjectPtr current = NULL;

static void
store_comments(obj)
ObjectPtr obj;
{
	String comments, help;

	if (obj == NULL)
		return;

	if (obj->comments != NULL)
		XtFree(obj->comments);
	if (obj->help != NULL)
		XtFree(obj->help);

	comments = XukcGetTextWidgetString(WI_comment_text);
	obj->comments = XtNewString(comments);
	XawAsciiSourceFreeString(XawTextGetSource(WI_comment_text));
	help = XukcGetTextWidgetString(WI_help_text);
	obj->help = XtNewString(help);
	XawAsciiSourceFreeString(XawTextGetSource(WI_help_text));
}


/******************* Module Exported Routines ********************/

/* update the comments and help popup .. save any existing comments to the
 * old object and display the new comments on the new object.
 */
void
EditComments(obj)
ObjectPtr obj;
{
	char tmp[100];
	String comments, help;

	if (current != obj)
		store_comments(current);

	if (obj == NULL) {
		comments = "";
		help = "";
		(void)strcpy(tmp, "Comments");
	} else {
		comments = (obj->comments == NULL ? "" : obj->comments);
		help = (obj->help == NULL ? "" : obj->help);
		(void)sprintf(tmp, "Comments for \"%s\"", obj->name);
	}
	XukcNewTextWidgetString(WI_comment_text, comments);
	XukcNewTextWidgetString(WI_help_text, help);
	XukcChangeLabel(WI_comment_title, tmp, FALSE); 
	current = obj;
}
