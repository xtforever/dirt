/* SCCSID: @(#)resources.h	1.1 (UKC) 5/10/92 */

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


/* each resource in the displayed list has a resource_entry */

struct resource_entry {
	XtResourceList type;	/* entry returned by GetResource() */
	Widget resource_name;	/* label/command widget in resource table */
	Widget resource_value;	/* widget representation of editable value */
	XtPointer value;	/* any new value set */
	Boolean value_set;	/* to show new value has been set */
	Boolean is_constraint;	/* is a constraint resource? */
};

/* Each displayed resource list has an associated structure as follows :- */

struct resource_list_details {
	ObjectPtr object;		/* these are instance resources ? */
	Widget resources_viewport;	/* the viewport containing the .. */
	Widget resources_table;		/* table of available resources */
	XtResourceList res;		/* resource list of class resources */
	XtResourceList cres;		/* constraint resources (if any) */
	struct resource_entry *list;	/* list of resource entries */
	Cardinal list_length;		/* total number of editable resources */
};

#define MAX_LISTS 10

#define NAME(e, field) (e->type->field)
#define RLF(field) (resource_lists[resource_list_number].field)
