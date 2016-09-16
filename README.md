/* SCCSID: @(#)README	1.1 (UKC) 5/10/92 */

        The UKC Widget Set and Dirt User Interface Builder Distribution

Release: Alpha 2
Patchlevel: 2

Please read all of this file before you start building Dirt!

Introduction:

This software distribution contains the UKC Widget Set and "Dirt" User
Interface Builder.  The UKC Widget set contains a number of widgets and
utilities required by Dirt but can also be used as a stand-alone widget
library.

There is a user manual for Dirt in ./dirt_editor/DOC/dirt.ps.Z which can be
printed to a PostScript printer using  ..

	"zcat ./dirt_editor/DOC/dirt.ps.Z | lpr -Plw"

This release is the second Alpha release and is therefore subject to major
changes.  I would be very grateful for any bug reports to be sent to
me at ..

Richard Hesketh		:	rlh2@ukc.ac.uk
Computing Officer, Computing Lab., University of Kent at Canterbury,
Canterbury, Kent, CT2 7NF, United Kingdom.
        Tel: +44 227 764000 ext 7620/7590      Fax: +44 227 762811

I would also like to hear any views you have about the usefulness (or not)
of the builder.  Particularly ... what do you think of the interface, whats
missing and what is wrong with it?

Distributed Files:

./README		- this file
./AUTHOR		- authors of this distribution
./CHANGES		- lists the changes between first and second release
./Motif.Changes		- patch required to Motif (1.1) for Dirt to work.
./patchlevel.h		- patch level of this distribution
./copyright.h		- details the copyrights in this distribution
./Imakefile		- Imakefile for the whole distribution (use xmkmf)
./Xukc.tmpl		- Template file for Imakefiles in distribution
./dirt_site.def		- Local changes to template file are placed here

./lib/Wc		- David Smyth's/Martin Brunecky's Widget Creation Library
./lib/Wc/Imakefile	- Imakefile to use instead of the Wc distribution's

./lib/Xukc		- The UKC Widget Set and utilities
./lib/Xukc/README	- details contents of UKC widget set
./lib/Xukc/patchlevel.h	- patch level of the UKC widget set
./lib/Xukc/Imakefile	- Imakefile for the UKC widget set
./lib/Xukc/Imakefile.GNU
			- Imakefile for the UKC widget set and GNUmalloc
./lib/Xukc/DOC		- widget class documentation
./lib/Xukc/GNUmalloc	- directory to place GNU or other malloc routines
			  NB: this directory is *empty* you need to copy
			      your favourite fast malloc in here if you want.
			      If you have a fast malloc then you
			      should use the Imakefile.GNU and adjust it
			      accordingly.

./lib/bitmaps		- Xukc/Dirt specific bitmaps
./lib/bitmaps/Imakefile	- Imakefile for the bitmaps

./dirt_editor		- The Dirt User Interface Builder
./dirt_editor/Imakefile - Imakefile for Dirt
./dirt_editor/README	- Describes Dirt 
./dirt_editor/DOC	- The Documentation for Dirt in LaTeX form.
./dirt_editor/DOC/dirt.ps.Z
			- A PostScript version of the Dirt Manual.
./dirt_editor/examples	- A collection of Dirt generated interfaces.


How To Build Dirt
-----------------

1) Make sure you have either the MIT distribution of X11R4 (patchlevel 18)
   or X11R5.  Dirt is not guaranteed to work with anything else (I have
   not tried it with DECwindows or OpenWindows 3.0 for example) and
   may not even build with different X releases.  I can only support
   X11R4 (patchlevel 18) or X11R5.

   [How do you know if you have the right libraries?  Well, if you find
    that you cannot build "lib/Xukc/Layout.c" with messages such as:

	"Layout.c", line 641: object undefined
	"Layout.c", line 652: rectangle undefined

    then the Xt intrinsics you are using are too old to be of any use.
    Drop them and get hold of the R5 (or R4 patchlevel 18) libraries from
    MIT's export machine or other X11 archive sites.  You only need the
    libraries, header and config files, not the server or clients.
    If you try to fix your copy of the R4 intrinsics (for example) then
    although Dirt might build it will core dump sooner rather than later.]

2) CD to the top of this distribution.

3) Decide whether to use the GNUmalloc (you can use malloc.c and
   getpagesize.h from GNU emacs) or to use the native malloc routines.
   Other fast malloc's based on the Caltech one can be used; I have found
   these malloc's to be about twice as fast at runtime .. of course there
   is a trade off in size and you may think that Dirt uses up enough
   swap space already 8-).

   If you want to use a new malloc then place it in the GNUmalloc directory
   and use something like the Imakefile.GNU and rename it just Imakefile;
   remembering of course to renaming the original Imakefile to something
   else!

4) Edit the dirt_site.def file.  You should make changes for your site
   here.  These changes will include whether you are using X11R5 or R4,
   Motif or wanting debugging libraries etc.  The file should be
   self-explanatory!

   If you want to use Motif then you will probably have to apply a small
   patch to the Motif source (assuming you have it).  This patch is given
   in ./Motif.Changes; it is no way an official patch and you should only
   apply it if you want to use Dirt and even then I would suggest that
   you build a different Motif library just for Dirt.

5) Get a Makefile from the Imakefile in this directory using either "xmkmf"
   or "imake -DUseInstalled -I/usr/lib/X11/config".

6) Type "make World" and go off and have a cup of coffee 8-)
   You should have at least 10-15 Megabytes of free file space for dirt
   to build .. of course this depends on what type of architecture you are
   using .. RISC based boxes generate noticable large object code!

Dirt will build relative to this directory.  The executable will be put
in ./dirt_editor/dirt and picks up bitmaps and resource files from
./lib/bitmaps and ./dirt_editor/Dirt respectively.


Installing Dirt
---------------

Do steps 1 - 5 above and then type:
	"make install"

All the header files, the Xukc Library, Widget Creation Library, bitmaps,
application defaults files and dirt binary will be installed in the relevant
system directories (you must have permission to create files in the system
directories).

You can edit dirt_site.def to decide if you want the libraries installed
or not to stop treading on existing ones.  You can also decide if you
want to link against installed Wc and Xukc libraries or not.


Notes on Running Dirt
---------------------

When Dirt runs it is *huge*, I mean its pretty big.  Don't be suprised if
on your SPARCstation it easily eats into 3 Meg of your swap space; remember
it has to contain every single Widget Class even if you don't use them.
Due to memory leaks in various places that I cannot easily fix; i.e. the
Athena Text Widget, a running Dirt process will steadily grow.  If you
find a memory leak and track it down I would very much like to hear about it.
In the mean time you will have keep to quiting and starting Dirt again
if you run out (or dangerously low) of swap space (about >= 1/2 hour constant
creation and editing of objects will do this on a SPARCstation with 20 Meg
of swap configured).  Saving and re-loading of user interfaces is quite quick
so this shouldn't impact on your time too much.  A bit like the saying
"Write out your edits frequently" !

Dirt will generate some warnings when run .. these are perfectly natural
and you don't have to worry about them (they will disappear in time,
probably).  The warnings will be of the type:

    No type converter registered for 'TranslationTable' to 'AcceleratorTable'
    No type converter registered for 'Pixmap' to 'Bitmap' conversion.
    No type converter registered for 'Pixmap' to 'Bitmap' conversion.
