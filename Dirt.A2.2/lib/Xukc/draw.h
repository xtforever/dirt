/* SCCSID: @(#)draw.h	1.1 (UKC) 5/10/92 */

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


/********* Buffered Drawing Functions for X Toolkit Widgets ********/

extern void XukcDrawRectangle(/* w, gc, x, y, width, height */);
/*  Widget w;
    GC gc;
    Position x, y;
    Dimension width, height;
 */

extern void XukcDrawLine(/* w, gc, x1, y1, x2, y2 */);
/*  Widget w;
    GC gc;
    Position x1, y1, x2, y2;
 */

extern void XukcDrawPoint(/* w, gc, x1, y1 */);
/*  Widget w;
    GC gc;
    Position x1, y1;
 */

extern void XukcFlushBufferedLines();
extern void XukcFlushBufferedRectangles();
extern void XukcFlushBufferedPoints();
extern void XukcFlushBufferedOutput();

extern void XukcBufferDrawRectangles(/* Boolean mode */);
extern void XukcBufferDrawLines(/* Boolean mode */);
extern void XukcBufferDrawPoints(/* Boolean mode */);

extern void XukcDrawPointsMode(/* int mode */);
