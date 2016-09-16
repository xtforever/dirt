/* SCCSID: @(#)memory.h	1.1 (UKC) 5/10/92 */

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

/* public routine definitions in memory.c */

extern void XukcSetSlotIncrement(/* slot_increment */);
/* Cardinal slot_increment;
 */

extern XtPointer *XukcGetListPointer(/* list_num, num_on_list */);
/* Cardinal list_num;
 * Cardinal *num_on_list;
 */

#define XukcGetAndCastListPointer(list_num, num_on_list, cast) \
			     (cast)*XukcGetListPointer(list_num, num_on_list)

extern XtPointer XukcGetObjectOnList(/* list_num, entry_num */);
/* Cardinal list_num;
 * Cardinal entry_num;
 */

extern Cardinal XukcFindObjectOnList(/* list_num, object */);
/* Cardinal list_num;
 * XtPointer object;
 */

extern void XukcRemoveObjectOnList(/* list_num, object */);
/* Cardinal *list_num;
 * XtPointer object;
 */

extern void XukcRemoveEntryOnList(/* list_num, entry_num */);
/* Cardinal *list_num;
 * Cardinal entry_num;
 */

extern void XukcDestroyList(/* list_num, free_contents */);
/* Cardinal *list_num;
 * Boolean free_contents;
 */

extern Cardinal XukcAddObjectToList(/* list_num, object, unique_only */);
/* Cardinal *list_num; 
 * XtPointer object;
 * Boolean unique_only;
 */

extern Boolean XukcChangeObjectOnList(/* list_num, entry_num, new_object */);
/* Cardinal list_num;
 * Cardinal entry_num;
 * XtPointer new_object;
 */

extern String XukcGrowString(/* old_str, increase */);
/* String old_str;
 * Cardinal increase;
 */
