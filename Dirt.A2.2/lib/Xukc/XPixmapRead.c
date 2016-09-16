#ifndef lint
/* From:   "$Xukc: XPixmapRead.c,v 1.7 91/12/17 10:02:14 rlh2 Rel $"; */
static char sccsid[] = "@(#)XPixmapRead.c	1.3 (UKC) 5/17/92";
#endif /* !lint */

/*
 * Copyright 1990 GROUPE BULL
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of GROUPE BULL not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  GROUPE BULL makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * GROUPE BULL disclaims all warranties with regard to this software,
 * including all implied warranties of merchantability and fitness,
 * in no event shall GROUPE BULL be liable for any special,
 * indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious
 * action, arising out of or in connection with the use 
 * or performance of this software.
 *
 */

/* NB: this is the XPM format 1 .. not 2 .. I'll merge this in later - rlh2 */

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/StringDefs.h>
#include <sys/param.h>			/* get MAXPATHLEN if possible */
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#include <X11/Xmu/CvtCache.h>
#include <X11/Xmu/Drawing.h>

#include <stdio.h>

#include "xpm.h"      /* PixmapOpenFailed, PixmapSuccess .. MAXPRINTABLE */

/*------------------------------------------------------------*/
/* following routines are used in XReadPixmapFile() function */
/*------------------------------------------------------------*/

/*
 * read the next line and jump blank lines 
 */
static char *
getline(s,pF)
     char * s ;
     FILE * pF ;
{
    s = fgets(s,256,pF);

    while (s) {
	int len = strlen(s);
	if (len && s[len-1] == '\015')
	    s[--len] = '\0';
	if (len==0) s = fgets(s,256,pF);
	else break;
    }
    return(s);
}
	    

/*
 * fatal message : return code, no exit 
 */
static int fatal(msg, p1, p2, p3, p4)
    char *msg;
{
#if 0
    fprintf(stderr,"\n");
    fprintf(stderr, msg, p1, p2, p3, p4);
    fprintf(stderr,"\n");
#endif
    return PixmapFileInvalid ;
}



/*
 *  Read a Pixmap file in a X Pixmap with specified depth and colormap
 * (THIS VERSION READ ONLY 2 CHARS PER PIXEL FORMAT : '_chars_per_pixel 2')
 *
 * Extended to read both one and 2 chars per pixel format: rlh2@ukc.ac.uk
 * 19/8/90
 */
int XReadPixmapFile (display,d,colormap,filename,width,height,depth,pixmap)
/*-----------------*/
     Display *display;
     Drawable d;
     Colormap colormap ;
     char *filename;
     unsigned int *width, *height;       /* RETURNED */
     int depth ;   
     Pixmap *pixmap;                     /* RETURNED */
{
  GC Gc ;
  XGCValues xgcv;

  FILE *fstream;			/* handle on file  */
  char linebuf[256] ;
  char namebuf[256];
  char type[256];
  char name[256];

  int ncolors ;

  CmapEntry * cmap ;           
  int * Tpixel ;

  char c1, c2;
  XColor xcolor ;
  int i,j,p;
  int chars_per_pixel;

  
  if ((fstream = fopen(filename, "r")) == NULL) {
    return PixmapOpenFailed;
  }

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "#define %[^_]%s %d", namebuf, type, &p) != 3) 
      || ((strcmp("_format",type)) && (strcmp("_paxformat",type)))
      || (p != XPM_FORMAT)) 
    return fatal("bad '#define NAME_format 1'"); 
  else strcpy(name,namebuf);

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "#define %[^_]%s %d", namebuf, type, width) != 3)
      || (strcmp(name,namebuf)) 
      || (strcmp("_width",type))) 
	return fatal("bad '#define NAME_width w'");

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "#define %[^_]%s %d", namebuf, type, height) != 3)
      || (strcmp(name,namebuf)) 
      || (strcmp("_height",type))) 
	return fatal("bad '#define NAME_height w'");

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "#define %[^_]%s %d", namebuf, type, &ncolors) != 3)
      || (strcmp(name,namebuf)) 
      || (strcmp("_ncolors",type))) 
	return fatal("bad '#define NAME_ncolors w'");

  if (ncolors > (MAXPRINTABLE*MAXPRINTABLE)) 
    return fatal("Too many different colors, version 1");

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "#define %[^_]%s %d", namebuf, type,
      &chars_per_pixel) != 3) || (strcmp(name,namebuf))
      || ((chars_per_pixel != 2) && (chars_per_pixel != 1))
      || (strcmp("_chars_per_pixel",type)))
	return fatal("bad '#define NAME_chars_per_pixel .. 1 or 2'");

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "static char * %[^_]%s = {",namebuf,type) != 2)
      || (strcmp(name,namebuf))
      || (strcmp("_colors[]",type))) 
	return fatal("bad 'static char * NAME_colors[] = {'");

  cmap = (CmapEntry *) malloc(ncolors*sizeof(CmapEntry)) ;
  Tpixel = (int *) malloc(ncolors*sizeof(int)) ;
  if ((cmap == NULL) || (Tpixel == NULL))
    return PixmapNoMemory ;

  getline(linebuf,fstream);
  if (chars_per_pixel == 1) {
    for (i=0; i<ncolors ; i++) {
       if (sscanf(linebuf, "\"%c\" , \"%[^\"]%s", &c1, namebuf,type) != 3)
	  return fatal("bad colormap entry : must be '\"c\" , \"colordef\",'"); 
       if (index(printable,c1)) {
   	      cmap[i].cixel.c1 = c1 ;
   	      cmap[i].cixel.c2 = NULL ;
   	      if (!XParseColor(display,colormap,namebuf,&xcolor))
   		return fatal("bad colordef specification : #RGB or colorname");
   	      XAllocColor(display,colormap,&xcolor);
   	      Tpixel[i] = xcolor.pixel ;
	} else
   	      return fatal("bad cixel value : must be printable");
    
   	    getline(linebuf,fstream);
     }
  } else { /* chars_per_pixel == 2 */
    for (i=0; i<ncolors ; i++) {
       if (sscanf(linebuf, "\"%c%c\" , \"%[^\"]%s", &c1,&c2, namebuf,type) != 4)
	  return fatal("bad colormap entry : must be '\"cC\" , \"colordef\",'"); 
       if ((index(printable,c1)) && (index(printable,c2))) {
   	      cmap[i].cixel.c1 = c1 ;
   	      cmap[i].cixel.c2 = c2 ;
   	      if (!XParseColor(display,colormap,namebuf,&xcolor))
   		return fatal("bad colordef specification : #RGB or colorname");
   	      XAllocColor(display,colormap,&xcolor);
   	      Tpixel[i] = xcolor.pixel ;
	} else
   	      return fatal("bad cixel value : must be printable");
    
   	    getline(linebuf,fstream);
     }
  }

  if (strncmp(linebuf, "} ;",3))
    return fatal("missing '} ;'");

  getline(linebuf,fstream);
  if ((sscanf(linebuf, "static char * %[^_]%s = {",namebuf,type) != 2)
      || (strcmp(name,namebuf))
      || (strcmp("_pixels[]",type))) 
	return fatal("bad 'static char * NAME_pixels[] = {'");

  *pixmap = XCreatePixmap(display,d,*width,*height,depth);
  Gc = XCreateGC(display,*pixmap,0,&xgcv);
  
  getline(linebuf,fstream);
  j = 0 ;
  while((j < *height) && strncmp(linebuf, "} ;",3)) {  
    if (chars_per_pixel == 2) {
       if (strlen(linebuf) < (2*(*width)+2)) 
	  return fatal("bad pixmap line length %d",strlen(linebuf));
       for (i=1; i<(2*(*width)) ; i+=2) {
         c1 = linebuf[i] ;
         c2 = linebuf[i+1] ;
         for (p = 0 ; p < ncolors ; p++)
           if ((cmap[p].cixel.c1 == c1)&&(cmap[p].cixel.c2 == c2)) break ;
         if (p != ncolors)
            XSetForeground(display,Gc,Tpixel[p]);
         else 
	    return fatal("cixel %c%c not in previous colormap",c1,c2);
         XDrawPoint(display,*pixmap,Gc,i/2,j) ;
       }
    } else {
       if (strlen(linebuf) < (*width+2)) 
	  return fatal("bad pixmap line length %d",strlen(linebuf));
       for (i=0; i < *width ; i++) {
         c1 = linebuf[i+1] ;
         for (p = 0 ; p < ncolors ; p++)
           if (cmap[p].cixel.c1 == c1) break ;
         if (p != ncolors)
            XSetForeground(display,Gc,Tpixel[p]);
         else 
	    return fatal("cixel %c not in previous colormap",c1);
         XDrawPoint(display,*pixmap,Gc,i,j) ;
       }
    }
    j++ ;
    getline(linebuf,fstream);    
  }

  if (strncmp(linebuf, "} ;",3))
    return fatal("missing '} ;'");

  if (j != *height)
    return fatal("%d too few pixmap line", *height - j);

  free(cmap) ;
  free(Tpixel) ;
  fclose(fstream) ;
  return PixmapSuccess ;
}


/*
 * XukcLocatePixmapFile - read a Pixmap file using the normal defaults
 *
 * Taken from XmuLocateBitmapFile() in mit/Xmu/LocBitmap.c
 */

/*
 * $XConsortium: LocBitmap.c,v 1.4 89/12/10 10:35:21 rws Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */


Pixmap XukcLocatePixmapFile(screen, name, srcname, srcnamelen,
			    widthp, heightp, fg, bg)
Screen *screen;
char *name;
char *srcname;			/* RETURN */
int srcnamelen;
int *widthp, *heightp;  /* RETURN */
unsigned long fg, bg;
{
    Display *dpy = DisplayOfScreen (screen);
    Window root = RootWindowOfScreen (screen);
    Bool try_plain_name = True;
    XmuCvtCache *cache = _XmuCCLookupDisplay (dpy);
    char **file_paths;
    char filename[MAXPATHLEN];
    unsigned int width, height, junk;
    int i;
    static char **dirt_split_path_string();


    /*
     * look in cache for bitmap path
     */
    if (cache) {
	if (!cache->string_to_bitmap.bitmapFilePath) {
	    XrmName xrm_name[2];
	    XrmClass xrm_class[2];
	    XrmRepresentation rep_type;
	    XrmValue value;

	    xrm_name[0] = XrmStringToName ("bitmapFilePath");
	    xrm_name[1] = NULL;
	    xrm_class[0] = XrmStringToClass ("BitmapFilePath");
	    xrm_class[1] = NULL;
	    /*
	     * XXX - warning, derefing Display * until XDisplayDatabase
	     */
	    if (!dpy->db) {
		/* what a hack; need to initialize dpy->db */
		(void) XGetDefault (dpy, "", "");
	    }
	    if (XrmQGetResource (dpy->db, xrm_name, xrm_class, 
				 &rep_type, &value) &&
		rep_type == XrmStringToQuark(XtRString)) {
		cache->string_to_bitmap.bitmapFilePath = 
		  dirt_split_path_string (value.addr);
	    }
	}
	file_paths = cache->string_to_bitmap.bitmapFilePath;
    }


    /*
     * Search order:
     *    1.  name if it begins with / or ./
     *    2.  "each prefix in file_paths"/name
     *    3.  BITMAPDIR/name
     *    4.  name if didn't begin with / or .
     */

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif
#ifndef PIXMAPDIR
#define PIXMAPDIR "/usr/include/X11/pixmaps"
#endif

    for (i = 1; i <= 5; i++) {
	char *fn = filename;
	unsigned char *datap = NULL;
	Pixmap pixmap;

	switch (i) {
	  case 1:
	    if (!(name[0] == '/' || (name[0] == '.') && name[1] == '/')) 
	      continue;
	    fn = name;
	    try_plain_name = False;
	    break;
	  case 2:
	    if (file_paths && *file_paths) {
		sprintf (filename, "%s/%s", *file_paths, name);
		file_paths++;
		i--;
		break;
	    }
	    continue;
	  case 3:
	    sprintf (filename, "%s/%s", PIXMAPDIR, name);
	    break;
	  case 4:
	    sprintf (filename, "%s/%s", BITMAPDIR, name);
	    break;
	  case 5:
	    if (!try_plain_name) continue;
	    fn = name;
	    break;
	}

	if (XReadPixmapFile(dpy, root, DefaultColormapOfScreen(screen),
			     fn, &width, &height, DefaultDepthOfScreen(screen),
			     &pixmap) == PixmapSuccess) {
	    if (widthp) *widthp = (int)width;
	    if (heightp) *heightp = (int)height;
	    if (srcname && srcnamelen > 0) {
		strncpy (srcname, fn, srcnamelen - 1);
		srcname[srcnamelen - 1] = '\0';
	    }
	    return pixmap;
	} else if (XmuReadBitmapDataFromFile(fn, &width, &height,
		   &datap, &junk, &junk) == BitmapSuccess) {
	    pixmap = XCreatePixmapFromBitmapData(dpy, root, datap, width,
				height, fg, bg, DefaultDepthOfScreen(screen));
	    if (widthp) *widthp = (int)width;
	    if (heightp) *heightp = (int)height;
	    if (srcname && srcnamelen > 0) {
		strncpy (srcname, fn, srcnamelen - 1);
		srcname[srcnamelen - 1] = '\0';
	    }
	    return pixmap;
	}
    }

    return None;
}


/*
 * dirt_split_path_string - split a colon-separated list into its constituent
 * parts; to release, free list[0] and list.
 */
static char **dirt_split_path_string (src)
    register char *src;
{
    int nelems = 1;
    register char *dst;
    char **elemlist, **elem;

    /* count the number of elements */
    for (dst = src; *dst; dst++) if (*dst == ':') nelems++;

    /* get memory for everything */
    dst = (char *) malloc (dst - src + 1);
    if (!dst) return NULL;
    elemlist = (char **) calloc ((nelems + 1), sizeof (char *));
    if (!elemlist) {
	free (dst);
	return NULL;
    }

    /* copy to new list and walk up nulling colons and setting list pointers */
    strcpy (dst, src);
    for (elem = elemlist, src = dst; *src; src++) {
	if (*src == ':') {
	    *elem++ = dst;
	    *src = '\0';
	    dst = src + 1;
	}
    }
    *elem = dst;

    return elemlist;
}


void _StringToPixmapInitCache (c)
    register XmuCvtCache *c;
{
    c->string_to_bitmap.bitmapFilePath = NULL;
}

void _StringToPixmapFreeCache (c)
register XmuCvtCache *c;
{
    if (c->string_to_bitmap.bitmapFilePath) {
	if (c->string_to_bitmap.bitmapFilePath[0]) 
	  free (c->string_to_bitmap.bitmapFilePath[0]);
	free ((char *) (c->string_to_bitmap.bitmapFilePath));
    }
}
