/* @(#)xpm.h	1.1 (UKC) 5/10/92 */

/* Copyright 1989 GROUPE BULL -- See licence conditions in file COPYRIGHT */
/**************************************************************************
 ****  Constant used in xpm library functions :
 ****
 ****  int XWritePixmapFile(dpy, cmap, filename, pixmap, w, h)
 ****  int XReadPixmapFile(dpy, d, cmap, filename, w, h, depth, pixmap)
 ****  Pixmap XCreatePixmapFromData(dpy, d, cmap, w, h, depth, n, c, col, pix)
 ****
 ****  Daniel Dardailler - Bull RC (89/02/22) e-mail: daniel@mirsa.inria.fr
 **************************************************************************/

/* Actually see the copyright in ./XPixmapRead.c - rlh2 */

#define XPM_FORMAT 1

/* we keep the same codes as for Bitmap management */
#ifndef _XUTIL_H_
#include <X11/Xutil.h>
#endif
#define PixmapSuccess        BitmapSuccess 
#define PixmapOpenFailed     BitmapOpenFailed
#define PixmapFileInvalid    BitmapFileInvalid
#define PixmapNoMemory       BitmapNoMemory


typedef struct _Cixel {
     char c1,c2 ;
   } Cixel ;                    /* 2 chars for one pixel value */

typedef struct _CmapEntry {
     Cixel cixel ;               
     int red,green,blue ;
   } CmapEntry ;                    


#define MAXPRINTABLE 93             /* number of printable ascii chars 
				       minus \ and " for string compat. */

static char * printable = " .XoO+@#$%&*=-;:?>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";
           /* printable begin with a space, so in most case, due to
	      my algorythm, when the number of different colors is
	      less than MAXPRINTABLE, it will give a char follow by 
	      "nothing" (a space) in the readable xpm file */

