/* SCCSID: @(#)menu.c	1.1 (UKC) 5/10/92 */

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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xukc/Med3Menu.h>

static MENU MM1 = {
	0x40,
	0,
	"Pick",
	80,
	-1,
	-1,
	78,
	35,
	28,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM2 = {
	0x40,
	0,
	"Single",
	116,
	130,
	25,
	230,
	53,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM3 = {
	0x40,
	496,
	"Decendants",
	114,
	130,
	53,
	230,
	81,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM4 = {
	0x404,
	53,
	NULL,
	0,
	130,
	25,
	230,
	81,
	-27,
	-10,
	0,
	NULL,
	&MM2,
	&MM3,
	NULL,
	NULL,
};
static MENU MM5 = {
	0x2260,
	0,
	"Copy",
	113,
	78,
	-1,
	157,
	35,
	0,
	0,
	0,
	NULL,
	&MM4,
	NULL,
	NULL,
	NULL,
};
static MENU MM6 = {
	0x40,
	0,
	"Move",
	77,
	157,
	-1,
	236,
	35,
	23036,
	5,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM7 = {
	0x8,
	157,
	NULL,
	113,
	78,
	-1,
	236,
	35,
	0,
	0,
	0,
	NULL,
	&MM5,
	&MM6,
	NULL,
	NULL,
};
static MENU MM8 = {
	0x8,
	78,
	NULL,
	80,
	-1,
	-1,
	236,
	35,
	0,
	1056,
	0,
	NULL,
	&MM1,
	&MM7,
	NULL,
	NULL,
};
static MENU MM9 = {
	0x40,
	0,
	"Delete",
	99,
	236,
	-1,
	316,
	35,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM10 = {
	0x40,
	0,
	"Shift",
	112,
	507,
	19,
	581,
	42,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM11 = {
	0x40,
	0,
	"Shrink",
	100,
	507,
	42,
	581,
	66,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM12 = {
	0x404,
	42,
	NULL,
	0,
	507,
	19,
	581,
	66,
	0,
	0,
	0,
	NULL,
	&MM10,
	&MM11,
	NULL,
	NULL,
};
static MENU MM13 = {
	0x260,
	0,
	"Tidy",
	97,
	356,
	20,
	498,
	43,
	0,
	0,
	0,
	NULL,
	&MM12,
	NULL,
	NULL,
	NULL,
};
static MENU MM14 = {
	0x40,
	0,
	"Add Horizontal",
	109,
	501,
	50,
	658,
	77,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM15 = {
	0x40,
	0,
	"Add Vertical",
	110,
	501,
	77,
	658,
	104,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM16 = {
	0x4,
	77,
	NULL,
	109,
	501,
	50,
	658,
	104,
	0,
	0,
	0,
	NULL,
	&MM14,
	&MM15,
	NULL,
	NULL,
};
static MENU MM17 = {
	0x40,
	0,
	"Remove Horizontal",
	111,
	501,
	104,
	658,
	131,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM18 = {
	0x40,
	0,
	"Remove Vertical",
	108,
	501,
	131,
	658,
	158,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM19 = {
	0x4,
	131,
	NULL,
	111,
	501,
	104,
	658,
	158,
	0,
	0,
	0,
	NULL,
	&MM17,
	&MM18,
	NULL,
	NULL,
};
static MENU MM20 = {
	0x404,
	104,
	NULL,
	0,
	501,
	50,
	658,
	158,
	0,
	0,
	0,
	NULL,
	&MM16,
	&MM19,
	NULL,
	NULL,
};
static MENU MM21 = {
	0x260,
	0,
	"Gravity Lines",
	101,
	356,
	43,
	498,
	67,
	0,
	0,
	0,
	NULL,
	&MM20,
	NULL,
	NULL,
	NULL,
};
static MENU MM22 = {
	0x40,
	0,
	"Gravity On",
	102,
	356,
	67,
	498,
	90,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM23 = {
	0x4,
	67,
	NULL,
	101,
	356,
	43,
	498,
	90,
	0,
	0,
	0,
	NULL,
	&MM21,
	&MM22,
	NULL,
	NULL,
};
static MENU MM24 = {
	0x4,
	43,
	NULL,
	97,
	356,
	20,
	498,
	90,
	-39,
	-15,
	0,
	NULL,
	&MM13,
	&MM23,
	NULL,
	NULL,
};
static MENU MM25 = {
	0x40,
	0,
	"Add Vertical",
	86,
	503,
	89,
	650,
	115,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM26 = {
	0x40,
	0,
	"Add Horizontal",
	72,
	503,
	115,
	650,
	141,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM27 = {
	0x4,
	115,
	NULL,
	86,
	503,
	89,
	650,
	141,
	0,
	0,
	0,
	NULL,
	&MM25,
	&MM26,
	NULL,
	NULL,
};
static MENU MM28 = {
	0x40,
	0,
	"Remove Vertical",
	118,
	503,
	141,
	650,
	167,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM29 = {
	0x40,
	0,
	"Remove Horizontal",
	104,
	503,
	167,
	650,
	193,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM30 = {
	0x4,
	167,
	NULL,
	118,
	503,
	141,
	650,
	193,
	0,
	0,
	0,
	NULL,
	&MM28,
	&MM29,
	NULL,
	NULL,
};
static MENU MM31 = {
	0x404,
	141,
	NULL,
	0,
	503,
	89,
	650,
	193,
	0,
	0,
	0,
	NULL,
	&MM27,
	&MM30,
	NULL,
	NULL,
};
static MENU MM32 = {
	0x260,
	0,
	"Connection Lines",
	106,
	356,
	90,
	498,
	114,
	0,
	0,
	0,
	NULL,
	&MM31,
	NULL,
	NULL,
	NULL,
};
static MENU MM33 = {
	0x40,
	0,
	"Connections On",
	107,
	356,
	114,
	498,
	137,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM34 = {
	0x40,
	0,
	"Dismiss Menu",
	68,
	356,
	137,
	498,
	161,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM35 = {
	0x4,
	137,
	NULL,
	107,
	356,
	114,
	498,
	161,
	0,
	0,
	0,
	NULL,
	&MM33,
	&MM34,
	NULL,
	NULL,
};
static MENU MM36 = {
	0x4,
	114,
	NULL,
	106,
	356,
	90,
	498,
	161,
	0,
	0,
	0,
	NULL,
	&MM32,
	&MM35,
	NULL,
	NULL,
};
static MENU MM37 = {
	0x404,
	90,
	NULL,
	0,
	356,
	20,
	498,
	161,
	-39,
	-15,
	0,
	NULL,
	&MM24,
	&MM36,
	NULL,
	NULL,
};
static MENU MM38 = {
	0x2240,
	329,
	"Layout",
	98,
	316,
	-1,
	395,
	35,
	0,
	0,
	0,
	NULL,
	&MM37,
	NULL,
	NULL,
	NULL,
};
static MENU MM39 = {
	0x8,
	316,
	NULL,
	99,
	236,
	-1,
	395,
	35,
	-5,
	-16,
	0,
	NULL,
	&MM9,
	&MM38,
	NULL,
	NULL,
};
static MENU MM40 = {
	0x40,
	0,
	"Save",
	83,
	469,
	19,
	539,
	44,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM41 = {
	0x40,
	0,
	"Load",
	76,
	469,
	44,
	539,
	70,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM42 = {
	0x40,
	0,
	"Suck",
	115,
	469,
	70,
	539,
	96,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM43 = {
	0x4,
	70,
	NULL,
	76,
	469,
	44,
	539,
	96,
	0,
	0,
	0,
	NULL,
	&MM41,
	&MM42,
	NULL,
	NULL,
};
static MENU MM44 = {
	0x404,
	44,
	NULL,
	0,
	469,
	19,
	539,
	96,
	-5,
	-16,
	0,
	NULL,
	&MM40,
	&MM43,
	NULL,
	NULL,
};
static MENU MM45 = {
	0x2260,
	0,
	"File",
	50,
	395,
	-1,
	474,
	35,
	0,
	0,
	0,
	NULL,
	&MM44,
	NULL,
	NULL,
	NULL,
};
static MENU MM46 = {
	0x40,
	0,
	"Cancel",
	105,
	556,
	13,
	626,
	40,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM47 = {
	0x40,
	0,
	"Confirm",
	81,
	556,
	40,
	626,
	67,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
static MENU MM48 = {
	0x404,
	40,
	NULL,
	0,
	556,
	13,
	626,
	67,
	2,
	-22,
	0,
	NULL,
	&MM46,
	&MM47,
	NULL,
	NULL,
};
static MENU MM49 = {
	0x2260,
	0,
	"Quit",
	103,
	474,
	-1,
	554,
	35,
	0,
	0,
	0,
	NULL,
	&MM48,
	NULL,
	NULL,
	NULL,
};
static MENU MM50 = {
	0x8,
	474,
	NULL,
	50,
	395,
	-1,
	554,
	35,
	0,
	0,
	0,
	NULL,
	&MM45,
	&MM49,
	NULL,
	NULL,
};
static MENU MM51 = {
	0x8,
	395,
	NULL,
	99,
	236,
	-1,
	554,
	35,
	0,
	0,
	0,
	NULL,
	&MM39,
	&MM50,
	NULL,
	NULL,
};
MENU menu_struct = {
	0x408,
	236,
	NULL,
	0,
	-1,
	-1,
	554,
	35,
	0,
	0,
	0,
	NULL,
	&MM8,
	&MM51,
	NULL,
	NULL,
};
