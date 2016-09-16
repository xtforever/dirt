/* SCCSID: @(#)Convert.h	1.1 (UKC) 5/10/92 */

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

#ifndef _XukcConvertH_		/* Allow multiple inclusions -- bt@irfu.se */
#define _XukcConvertH_

/* The code in Converters*.c can be reused across widgets for converting named
 * values to integers or flags, etc.
 */

/* resource converters */
struct eList {
	String name;
	int value;
};

typedef struct eList EnumListRec;
typedef EnumListRec *EnumList;

extern Boolean
XukcCvtStringToNamedValue(/* display, args, num_args, fromVal, toVal, dummy */);
/* Display     *display;
 * XrmValuePtr args;
 * Cardinal    *num_args;
 * XrmValuePtr fromVal;
 * XrmValuePtr toVal;
 * XtPointer   *dummy;
 */

extern Boolean XukcCvtNamedValueToString();
extern Boolean XukcCvtStringToFlagSettings();
extern Boolean XukcCvtFlagSettingsToString();
extern Boolean XukcCvtTranslationsToString();
extern Boolean XukcCvtStringToPixmap();

#endif /* _XukcConvertH_ */
