#ifndef lint
static char sccsid[] = "@(#)putenv.c	1.2 (UKC) 5/10/92";
#endif /* !lint */

#include <ctype.h>
#include <X11/Xos.h>

#define NULL 0

/*  Define putenv for machines the don't have it in the standard library.
 */
int
my_putenv(s)
char *s;
{
	int nlen;
	char *cptr;
	char **nenv, **eptr;
	extern char **environ;

	/*  First see if there is an existing 'name=value' with the
	 *  same name as s.
	 */
	for (cptr = s; *cptr != '=' && *cptr != '\0'; cptr++)
		;
	if (*cptr == '=' && cptr > s) {
		nlen = cptr - s + 1;
		for (eptr = environ; *eptr != NULL; eptr++) {
			if (strncmp(*eptr, s, nlen) == 0) {
				*eptr = s;
				return 0;
			}
		}
	}
	
	/*  New name, so must change environ.
	 */
	for (eptr = environ; *eptr != NULL; eptr++)
		;
	nenv = (char **) malloc((eptr - environ + 2) * sizeof(char *));
	if (nenv == NULL)
		return -1;
	eptr = environ;
	environ = nenv;
	while ((*nenv = *eptr) != NULL)
		nenv++, eptr++;
	*nenv = s;
	nenv[1] = NULL;
	return 0;
}
