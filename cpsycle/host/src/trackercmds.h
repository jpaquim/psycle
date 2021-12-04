/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(TRACKERCMDS_H)
#define TRACKERCMDS_H

#include <properties.h>

enum {
	CMD_NAVUP = 1024,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_NAVPAGEUP,	/* < pgup */
	CMD_NAVPAGEDOWN,/* < pgdn */	
	CMD_NAVTOP,		/* < home */
	CMD_NAVBOTTOM,	/* < end */

	CMD_COLUMNPREV,	/* < tab */
	CMD_COLUMNNEXT,	/* < s-tab */

	CMD_BLOCKSTART,
	CMD_BLOCKEND,
	CMD_BLOCKUNMARK,
	CMD_BLOCKCUT,
	CMD_BLOCKCOPY,
	CMD_BLOCKPASTE,
	CMD_BLOCKMIX,

	CMD_ROWINSERT,
	CMD_ROWDELETE,
	CMD_ROWCLEAR,

	CMD_TRANSPOSEBLOCKINC,
	CMD_TRANSPOSEBLOCKDEC,
	CMD_TRANSPOSEBLOCKINC12,
	CMD_TRANSPOSEBLOCKDEC12,

	CMD_SELECTALL,
	CMD_SELECTCOL,
	CMD_SELECTBAR,

	CMD_SELECTMACHINE,	/* < Enter */
	CMD_UNDO,
	CMD_REDO,

	CMD_BLOCKDELETE,
	CMD_NAVPAGEUPKEYBOARD, /* < pgup keyboard */
	CMD_NAVPAGEDOWNKEYBOARD, /* < pgdn keyboard */

	CMD_DIGIT0,
	CMD_DIGIT1,
	CMD_DIGIT2,
	CMD_DIGIT3,
	CMD_DIGIT4,
	CMD_DIGIT5,
	CMD_DIGIT6,
	CMD_DIGIT7,
	CMD_DIGIT8,
	CMD_DIGIT9,
	CMD_DIGITA,
	CMD_DIGITB,
	CMD_DIGITC,
	CMD_DIGITD,
	CMD_DIGITE,
	CMD_DIGITF,

	CMD_COLUMN_0, /* Jump To Track (0..7) */
	CMD_COLUMN_1, /* FT2: ALT + Q .. I */
	CMD_COLUMN_2,
	CMD_COLUMN_3,
	CMD_COLUMN_4,
	CMD_COLUMN_5,
	CMD_COLUMN_6,
	CMD_COLUMN_7,
	CMD_COLUMN_8, /* Jump To Track(8..F) */
	CMD_COLUMN_9, /* have no key default settings atm */
	CMD_COLUMN_A, /* because FT2 key defaults conflict */
	CMD_COLUMN_B, /* with other psycle cmd-key mappings */
	CMD_COLUMN_C, /* (FT2 is: ALT + A .. K) */
	CMD_COLUMN_D,
	CMD_COLUMN_E,
	CMD_COLUMN_F
};

void trackercmds_make(psy_Property* parent);

#endif /* TRACKERCMDS_H */
