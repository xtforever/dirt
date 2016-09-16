#ifndef lint
static char sccsid[] = "@(#)file.c	1.2 (UKC) 5/10/92";
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

/* file.c --- contains the file routines for UNIX files.  This file should
 *	      the only one you need to change between different varieties of
 *	      UNIX's 8-}.
 */

#include <X11/Xos.h>
#include <X11/Intrinsic.h>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

extern Widget WI_dirt;

extern void GiveWarning();
extern char *reason();
extern XtPointer *XukcGetListPointer();
extern Cardinal XukcAddObjectToList();

/* for quick sort */
static int
compare_names(a, b)
String a, b;
{
        return (strcmp(*(String *)a, *(String *)b));
}


static void
find_matching_files(dir, suffix, file_list)
String dir;
String suffix;
Cardinal *file_list;
{
	DIR *dd;
	struct dirent *dir_entry;

	if (dir == NULL || *dir == '\0')
		return;

	dd = opendir(dir);

	if (dd == NULL) {
		/* some error happened .. try to recover */
		if (suffix != NULL)
			GiveWarning(WI_dirt, "failedToOpen",
				"searchDirectoryForSuffixed",
	"Could not examine directory \"%s\" for \"%s\" suffixed files (%s)",
				3, dir, suffix, reason());
		else
			GiveWarning(WI_dirt, "failedToOpen", "searchDirectory",
				"Could not examine directory \"%s\" (%s)",
				2, dir, reason());
		return;
	}

	dir_entry = readdir(dd);
	while (dir_entry != NULL) {
		String file = NULL;

		if (suffix == NULL) {
			/* we want all entries in the directory */
			file = (String)XtMalloc(strlen(dir)
					      + strlen(dir_entry->d_name) + 2);
			(void)sprintf(file, "%s/%s", dir, dir_entry->d_name);
			(void)XukcAddObjectToList(file_list, (XtPointer)file,
							FALSE);
		} else {
			String basename;

			basename = rindex(dir_entry->d_name, '.');
			if (basename != NULL &&
			    strcmp(basename, suffix) == 0) {
				*basename = '\0';
				file = (String)XtMalloc(strlen(dir)
					      + strlen(dir_entry->d_name) + 2);
				(void)sprintf(file, "%s/%s", dir,
							dir_entry->d_name);
				(void)XukcAddObjectToList(file_list,
							(XtPointer)file, FALSE);
				*basename = '.';
			}
		}
		dir_entry = readdir(dd);
	}
	closedir(dd);
}


/************************* Module Exported Routines *********************/

String
GetUniqueFilename(name, prefix)
String *name, prefix;
{
	return (*name);
}


Cardinal
SuffixedFilesInDirectories(directories, suffix)
String directories;  /* colon separated list of directory paths */
String suffix;       /* suffix of file types to look for e.g. ".dt" Or NULL */
{
	Cardinal string_list = 0; /* list of strings to return */
	String next_dir;

	if (directories == NULL)
		return 0; /* ||| this is really an internal error ||| */

	next_dir = index(directories, ':');
	while (*directories != '\0') {
		if (next_dir != NULL)
			*next_dir = '\0';

		find_matching_files(directories, suffix, &string_list);
		if (next_dir != NULL) {
			*next_dir = ':';
			directories = next_dir + 1;
		} else
			/* zoom to the end of the string */
			directories += strlen(directories);
	}

	/* finally also check the current directory */
	find_matching_files(".", suffix, &string_list);

	if (string_list != NULL) {
		Cardinal num;
		String *files = (String *)XukcGetListPointer(string_list,
								&num);
		qsort(files, num, sizeof(String), compare_names);
		return (string_list);
	} else
		return 0;
}
