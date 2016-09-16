#ifndef lint
static char sccsid[] = "@(#)hash.c	1.2 (UKC) 5/10/92";
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
 *  An attempt at some hash storage routines.  These are used for lookup
 *  of string names and should be useful on their own.  The hash function
 *  works independently of the storage, which uses linked lists for colliding
 *  hashes.  Routines to add, delete and check names in the hash table are
 *  included.
 */

#include <X11/Intrinsic.h>
#include <X11/Xukc/memory.h>
#include "dirt_structures.h"

/* externally defined routines */
extern XtPointer XukcGetObjectOnList();

#define HASH_SIZE 512

static Cardinal tables = 0;  /* list of hash tables */
static Cardinal app_hash = 0;
static Cardinal template_hash = 0;

static Cardinal
initialize_hash_table()
{
	Cardinal i;
	Cardinal *hash_table =
			(Cardinal *)XtMalloc(sizeof(Cardinal) * HASH_SIZE);

	for (i = 0; i < HASH_SIZE; i++)
		hash_table[i] = 0;

	return (XukcAddObjectToList(&tables, (XtPointer)hash_table, FALSE));
}


static Cardinal
hash_name(name)
String name;
{
	/* perform the hash function and return the entry in the hash table */
	Cardinal i;
	int n;

	if (name == NULL)
		abort(); /* should never get here really */

	for (n = 0, i = strlen(name); i > 0; i--)
		n += name[i-1];
	n = n % HASH_SIZE;
	return ((Cardinal)n);
}


static Boolean
hash_full(table, hash)
Cardinal table, hash;
{
	Cardinal *hash_table;

	hash_table = (Cardinal *)XukcGetObjectOnList(tables, (XtPointer)table);
	return ((Boolean)(hash_table[hash] > 0));
}


static int
find_name(names, num_names, name)
XrmQuark *names;
Cardinal num_names;
String name;
{
	XrmQuark saved_name;
	Cardinal i;

	saved_name = XrmStringToQuark(name);

	for (i = 0; i < num_names; i++)
		if (names[i] == saved_name)
			return ((int)i);

	return (-1); /* not found */
}


static void
add_to_table(table, hash, name)
Cardinal table, hash;
String name;
{
	Cardinal *hash_table;
	Cardinal *collision_list;
	XrmQuark saved_name;

	hash_table = (Cardinal *)XukcGetObjectOnList(tables, (XtPointer)table);
	collision_list = hash_table + hash;
	saved_name = XrmStringToQuark(name);
	XukcAddObjectToList(collision_list, (XtPointer)saved_name, FALSE);
}


static int
find_name_in_collision_list(table, hash, name)
Cardinal table, hash;
String name;
{
        Cardinal *hash_table;
        Cardinal *collision_list;
        Cardinal num_collisions;
        XrmQuark *names;

        hash_table = (Cardinal *)XukcGetObjectOnList(tables, (XtPointer)table);
        collision_list = hash_table + hash;
        if (*collision_list != 0) {
                names = (XrmQuark *)XukcGetListPointer(*collision_list,
                                                        &num_collisions);
                return (find_name(names, num_collisions, name));
        }
	return (-1);
}


static void
remove_from_hash_table(table, hash, entry)
Cardinal table, hash, entry;
{
        Cardinal *collisions;

        collisions = (Cardinal *)XukcGetObjectOnList(tables, (XtPointer)table);
        collisions += hash;
        if (*collisions != 0)
		XukcRemoveEntryOnList(collisions, entry + 1);
}


/**************** Module Exported Routines *********************/


void
AddNameToHashTable(table, name)
Cardinal *table;
String name;
{
	Cardinal hash;

	if (*table == 0)
		*table = initialize_hash_table();

	hash = hash_name(name);
	add_to_table(*table, hash, name);
}


Boolean
RemoveNameFromHashTable(table, name)
Cardinal table;
String name;
{
	int i;
	Cardinal hash;

	if (table == 0)
		return FALSE;

	hash = hash_name(name);

	if (!hash_full(table, hash))
		return FALSE;

	i = find_name_in_collision_list(table, hash, name);
	if (i < 0)
		return FALSE;

	remove_from_hash_table(table, hash, (Cardinal)i);

	return TRUE;
}


Boolean
IsUniqueHash(table, name)
Cardinal table;
String name;
{
	Cardinal hash;

	if (table == 0)
		return TRUE;

	hash = hash_name(name);

	if (!hash_full(table, hash))
		return TRUE;

	return (find_name_in_collision_list(table, hash, name) < 0);
}


/* see if given name is already in a child of the parent object. */
Boolean
IsNotUniqueName(parent, name)
ObjectPtr parent;
String name;
{
	if (parent->in_app)
		return (!IsUniqueHash(app_hash, name));
	else
		return (!IsUniqueHash(template_hash, name));
}  


void
AddHashedName(obj)
ObjectPtr obj;
{
	if (obj->in_app)
		AddNameToHashTable(&app_hash, obj->name);
	else
		AddNameToHashTable(&template_hash, obj->name);
}


void
RemoveHashedName(obj)
ObjectPtr obj;
{
	if (obj->in_app)
		RemoveNameFromHashTable(app_hash, obj->name);
	else
		RemoveNameFromHashTable(template_hash, obj->name);
}
