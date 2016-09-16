/* SCCSID: @(#)text.h	1.1 (UKC) 5/10/92 */

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

/* public header file for text.c */

#include <X11/Xaw/Text.h>

#ifdef HP_WIDGETS
#include <X11/Xw/TextEdit.h>
#endif /* HP_WIDGETS */

#define XukcNewTextWidgetString(w,string) \
	XukcModifyText(w, string, 0l, XawTextGetLastPos(w))

#define XukcClearTextWidget(w) XukcNewTextWidgetString(w, "")

#define XukcAppendToTextWidget(w,string) \
	XukcModifyText(w, string, XawTextGetLastPos(w), XawTextGetLastPos(w))

/* the following routines are defined in text.c for general use: */

extern String XukcGetTextWidgetString(/* w */);
/* Widget w;
 */

extern Boolean XukcModifyText(/* w, string, start_pos, end_pos */);
/* Widget w;
 * String string;
 * XawTextPosition start_pos, end_pos;
 *
 *	- inserts "string" into the TextWidget "w" between "start_pos" and
 *	  "end_pos".
 *	RETURNS: FALSE if failed to modify text.
 */

extern XawTextPosition XawTextGetLastPos();

#ifdef HP_WIDGETS
/* simple definitions for HP Widget Set text widget */

/* to clear the text use XwTextClearBuffer(w) */

#define XukcXwNewTextString(w, string) \
	XwTextReplace(w, 0l, XwTextGetLastPos(w, 0l), (unsigned char *)string);

#define XukcXwAppendToText(w, string) \
	XwTextSetInsertPos(w, XwTextGetLastPos(w, 0l)); \
	XwTextInsert(w, string)
#endif /* HP_WIDGETS */
