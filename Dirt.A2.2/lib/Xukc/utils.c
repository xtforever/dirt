#ifndef lint
static char sccsid[] = "@(#)utils.c	1.2 (UKC) 5/10/92";
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
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include "utils.h"

/* various utility functions which keep opaque types happy .. only
 * kept here for backwards compatibility
 */

Position
XukcGetX(w)
Widget w;
{
	Position x;

	XtVaGetValues(w, XtNx, &x, NULL);
	return (x);
}

Position
XukcGetY(w)
Widget w;
{
	Position y;

	XtVaGetValues(w, XtNy, &y, NULL);
	return (y);
}

Dimension
XukcGetWidth(w)
Widget w;
{
	Dimension width;

	XtVaGetValues(w, XtNwidth, &width, NULL);
	return (width);
}


Dimension
XukcGetHeight(w)
Widget w;
{
	Dimension height;

	XtVaGetValues(w, XtNheight, &height, NULL);
	return (height);
}


Dimension
XukcGetBorder(w)
Widget w;
{
	Dimension bw;

	XtVaGetValues(w, XtNborderWidth, &bw, NULL);
	return (bw);
}


String
XukcClassName(class)
WidgetClass class;
{
	return (class->core_class.class_name);
}


Widget
XukcGetShellManagedChild(shell)
ShellWidget shell;
{
	Widget childwid = NULL;
	int i;

	if (!XtIsSubclass((Widget)shell, shellWidgetClass))
		XtError("Non Shell Widget passed to XukcGetShellManagedChild()");
	for (i = 0; i < shell->composite.num_children; i++) {
		if (XtIsManaged(shell->composite.children[i])) {
			childwid = shell->composite.children[i];
			break;
		}
	}
	return (childwid);
}
