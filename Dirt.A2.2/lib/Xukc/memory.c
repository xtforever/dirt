#ifndef lint
static char sccsid[] = "@(#)memory.c	1.2 (UKC) 5/10/92";
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


/* various list and array management routines */
/* forward definitions of exported routines are given in memory.h */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#define SLOT_INC 10
#define MAX_SLOT_INC 1000

struct big_list {
	XtPointer	*list;
	Cardinal num_on_list;
	Cardinal num_slots;
};

static struct big_list *list_list = NULL;
static Cardinal num_on_list = 0;
static Cardinal num_slots = 0;
static Cardinal destroyed_list = 0;
static Cardinal slot_inc = SLOT_INC;

#ifdef DEBUG
#   define CHECK_LIST(list_num) { \
	if (list_num < 1 || list_num > num_on_list) \
		XtWarning("Bad list_num passed to XukcMemory function"); \
}
#else
#   define CHECK_LIST(list_num) { \
	if (list_num < 1 || list_num > num_on_list) { \
		XtWarning("Bad list_num passed to XukcMemory function"); \
		abort(1); \
	} \
}
#endif DEBUG

static void
memory_initialize()
{
	num_on_list = 0;
	num_slots = SLOT_INC;
	list_list = (struct big_list *)XtCalloc(sizeof(struct big_list),
								num_slots);
}


static Cardinal
make_new_list()
{
	Cardinal i;

	if (list_list == NULL)
		memory_initialize();

	if (destroyed_list > 0) { /* find the existing empty slot and use it */
		destroyed_list--;
		for (i = 0; list_list[i].num_on_list > 0 && i < num_on_list;)
			i++;
		if (list_list[i].num_on_list > 0)
			XtError("corrupt list_list in XukcMemory function"); 
	} else {
		if (num_on_list == num_slots) {
			num_slots += SLOT_INC;
			list_list = (struct big_list *)XtRealloc(
					(char *)list_list,
					sizeof(struct big_list) * num_slots);
		}
		i = num_on_list;
		num_on_list++;
	}

	list_list[i].list = (XtPointer *)XtCalloc(sizeof(XtPointer), slot_inc);
	list_list[i].num_on_list = 0;
	list_list[i].num_slots = slot_inc;
	return (i+1);
}


/**** Module Exported Routines ****/

/* set the "slot increment" step value.  Large numbers are better for
 * storing long lists.  Smaller numbers give better performance for lots
 * of short lists.  By default its SLOT_INC.
 */
void
XukcSetSlotIncrement(new_inc)
Cardinal new_inc;
{
	if (new_inc == 0 || new_inc > MAX_SLOT_INC) {
		XtWarning("Bad slot increment value passed to XukcSetSlotIncrement()");
		return;
	}
	slot_inc = new_inc;
}


/* return the pointer to a list and the number of objects on the list */
XtPointer *
XukcGetListPointer(list_num, on_list)
Cardinal list_num;	/* list to retrieve (> 0) */
Cardinal *on_list;	/* address for return of the number of objects */
{
	CHECK_LIST(list_num);
	*on_list = list_list[list_num-1].num_on_list;
	return (list_list[list_num-1].list);
}


/* retrieve an object in an object array/list.
 * RETURNS: NULL if entry_num is out of range */
XtPointer
XukcGetObjectOnList(list_num, entry_num)
Cardinal list_num;	/* list to look in (> 0)*/
Cardinal entry_num;	/* entry whose value we require (> 0)*/
{
	CHECK_LIST(list_num);
	if (entry_num < 1 || entry_num > list_list[list_num-1].num_on_list)
		return (NULL);
	else
		return (list_list[list_num-1].list[entry_num-1]);
}


/* look through the list denoted by list_num and return the entry number
 * that contains the given object.
 * RETURNS: 0 if object was not on list */
Cardinal
XukcFindObjectOnList(list_num, object)
Cardinal list_num;	/* number of object list (> 0)*/
XtPointer object;		/* object to find */
{
	XtPointer *tmp;
	Cardinal i;

	if (list_num == 0)
		return 0;
	CHECK_LIST(list_num);
	tmp = list_list[list_num-1].list;
	for (i = list_list[list_num-1].num_on_list; i > 0; i--)
		if (tmp[i-1] == object)
			break;
	return (i);
}


void
XukcDestroyList(list_num, free_contents)
Cardinal *list_num;	/* number of object list */
Boolean free_contents;  /* ALL the contents of the list can be XtFree'd */
{
	if (*list_num == 0) /* an empty list id ... so assume its been freed */
		return;

	CHECK_LIST(*list_num);
	if (free_contents) {
		XtPointer *tmp;
		Cardinal i;

		tmp = list_list[*list_num-1].list;
		i = list_list[*list_num-1].num_on_list;
		while (i > 0)
			XtFree((char *)tmp[--i]);
	}
	XtFree((char *)(list_list[*list_num-1].list));
	list_list[*list_num-1].num_on_list = 0;  /* zero the list */
	list_list[*list_num-1].num_slots = 0;  /* zero the list */
	destroyed_list++;	/* hint to say we have a spare slot ready */
	*list_num = 0;
}


void
XukcRemoveEntryOnList(list_num, entry_num)
Cardinal *list_num;	/* number of object list */
Cardinal entry_num;	/* entry of object to remove */
{
	XtPointer *tmp;

	CHECK_LIST(*list_num);
	if (entry_num < 1 || entry_num > list_list[*list_num-1].num_on_list)
		return;

	tmp = list_list[*list_num-1].list;
	for (; entry_num < list_list[*list_num-1].num_on_list; entry_num++)
		tmp[entry_num-1] = tmp[entry_num];

	if (--list_list[*list_num-1].num_on_list == 0)
		XukcDestroyList(list_num, FALSE);
}


void
XukcRemoveObjectOnList(list_num, object)
Cardinal *list_num;	/* number of object list */
XtPointer object;		/* object to remove */
{
	Cardinal i;
	XtPointer *tmp;

	CHECK_LIST(*list_num);
	tmp = list_list[*list_num-1].list;
	if ((i = XukcFindObjectOnList(*list_num, object)) == 0)
		XtError("unknown object address past to XukcRemoveObjectOnList()");

	tmp = list_list[*list_num-1].list;
	for (; i < list_list[*list_num-1].num_on_list; i++)
		tmp[i-1] = tmp[i];

	if (--list_list[*list_num-1].num_on_list == 0)
		XukcDestroyList(list_num, FALSE);
}


Cardinal
XukcAddObjectToList(list_num, object, unique_only)
Cardinal *list_num; 
XtPointer object;
Boolean unique_only;
{
	/* a new list to create ?? */
	if (*list_num == 0)
		*list_num = make_new_list();

	if (unique_only && XukcFindObjectOnList(*list_num, object) > 0)
		return (0);

	if (list_list[*list_num-1].num_on_list == list_list[*list_num-1].num_slots) {
		list_list[*list_num-1].num_slots += slot_inc;
		list_list[*list_num-1].list = (XtPointer *)XtRealloc(
			(char *)(list_list[*list_num-1].list),
			list_list[*list_num-1].num_slots * sizeof(XtPointer));
	}

	list_list[*list_num-1].list[list_list[*list_num-1].num_on_list] = object;
	list_list[*list_num-1].num_on_list++;
	return (list_list[*list_num-1].num_on_list);
}


/* change an existing entry in the list.
 * RETURNS: TRUE is successful.
 */
Boolean
XukcChangeObjectOnList(list_num, entry_num, new_object)
Cardinal list_num; 
Cardinal entry_num; 
XtPointer new_object;
{
	CHECK_LIST(list_num);
	if (entry_num < 1 || entry_num > list_list[list_num-1].num_on_list)
		return (FALSE);

	list_list[list_num-1].list[entry_num-1] = new_object;
	return (TRUE);
}


String
XukcGrowString(str, size)
String str;
Cardinal size;
{
#define CHUNK_SIZE 256
	String tmp;

	if (str == NULL) { /* allocate a new string */
		tmp = (String)XtMalloc(((size/CHUNK_SIZE)+1)*CHUNK_SIZE);
		*tmp = '\0';
		return (tmp);
	}
	return ((String)XtRealloc(str, ((size/CHUNK_SIZE)+1)*CHUNK_SIZE));
}


/* for debugging only */
#include<stdio.h>

XukcDumpLists()
{
	Cardinal i, j;

	for (i = 0; i < num_on_list; i++) {
		fprintf(stderr, "\nDumping contents of list %d\n", i+1);
		for (j = 0; j < list_list[i].num_on_list; j++)
			fprintf(stderr, "\tentry %d = %s\n", j+1,
					(String)list_list[i].list[j]);
	}
}
