#ifndef lint
static char sccsid[] = "@(#)text.c	1.2 (UKC) 5/10/92";
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

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include "text.h"

/* ||| kludge : this should be in Text.c ||| */
XawTextPosition
XawTextGetLastPos(w)
Widget w;
{
	XawTextSetInsertionPoint(w, 999999l);
	return (XawTextGetInsertionPoint(w));
}


Boolean
XukcModifyText(w, string, start_pos, end_pos)
Widget w;
String string;
XawTextPosition start_pos, end_pos;
{
	XawTextBlock text;

	if (!XtIsSubclass(w, textWidgetClass)) {
		XtWarningMsg("invalidWidget", "modifyText",
				"XukcToolkitWarning",
				"widget passed to XukcModifyText() is not a subclass of Text",
				(String *)NULL, (Cardinal *)NULL);
		return (FALSE);
	}

	text.format = FMT8BIT;
	text.firstPos = 0;
	text.length = strlen(string);
	text.ptr = string;

	if (XawTextReplace(w, start_pos, end_pos, &text) != XawEditDone) {
		String params[1];
		Cardinal num_params = 1;

		params[0] = XtName(w);
		XtWarningMsg("textReplaceFailure", "modifyText",
				"XukcToolkitWarning",
				"Unable to modify text in \"%s\" text widget",
				params, &num_params);
		return (FALSE);
	}
	return (TRUE);
}


String
XukcGetTextWidgetString(w)
Widget w;
{
	Arg args[1];
	String str;

	XtSetArg(args[0], XtNstring, &str);
	XtGetValues(w, (ArgList)args, 1);

	if (str == NULL)
		return ("");
	return (str);
}
