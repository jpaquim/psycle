/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "trackercmds.h"
/* ui */
#include <uidef.h>
/* eventdriver */
#include "../../driver/eventdriver.h"
/* platform */
#include "../../detail/portable.h"

static void setcmdall(psy_Property*, int cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext);
static void setcmd(psy_Property*, int cmd, uint32_t keycode,
	const char* key, const char* shorttext);

void trackercmds_make(psy_Property* parent)
{
	psy_Property* cmds;

	assert(parent);

	cmds = psy_property_set_text(
		psy_property_append_section(parent, "tracker"),
		"Tracker");
	setcmd(cmds, CMD_NAVUP, psy_ui_KEY_UP, "navup", "up");
	setcmd(cmds, CMD_NAVDOWN, psy_ui_KEY_DOWN, "navdown", "down");
	setcmd(cmds, CMD_NAVLEFT, psy_ui_KEY_LEFT, "navleft", "left");
	setcmd(cmds, CMD_NAVRIGHT, psy_ui_KEY_RIGHT, "navright", "right");
	setcmd(cmds, CMD_NAVPAGEUP, psy_ui_KEY_PRIOR, "navpageup", "pageup");
	setcmd(cmds, CMD_NAVPAGEDOWN, psy_ui_KEY_NEXT, "navpagedown", "pagedown");
	setcmd(cmds, CMD_NAVTOP, psy_ui_KEY_HOME, "navtop", "track top");
	setcmd(cmds, CMD_NAVBOTTOM, psy_ui_KEY_END, "navbottom", "track bottom");
	setcmdall(cmds, CMD_COLUMNPREV, psy_ui_KEY_TAB, psy_SHIFT_ON, psy_CTRL_OFF,
		"columnprev", "prev col");
	setcmd(cmds, CMD_COLUMNNEXT, psy_ui_KEY_TAB, "columnnext", "next col");
	setcmd(cmds, CMD_ROWINSERT, psy_ui_KEY_INSERT, "rowinsert", "ins row");
	setcmd(cmds, CMD_ROWDELETE, psy_ui_KEY_BACK, "rowdelete", "del row");
	setcmd(cmds, CMD_ROWCLEAR, psy_ui_KEY_DELETE, "rowclear", "clr row");
	setcmdall(cmds, CMD_BLOCKSTART, psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	setcmdall(cmds, CMD_BLOCKEND, psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
	setcmdall(cmds, CMD_BLOCKUNMARK, psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	setcmdall(cmds, CMD_BLOCKCUT, psy_ui_KEY_X, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcut", "cut");
	setcmdall(cmds, CMD_BLOCKCOPY, psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcopy", "copy");
	setcmdall(cmds, CMD_BLOCKPASTE, psy_ui_KEY_V, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockpaste", "paste");
	setcmdall(cmds, CMD_BLOCKMIX, psy_ui_KEY_M, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockmix", "mix");
	setcmdall(cmds, CMD_BLOCKDELETE, psy_ui_KEY_X, psy_SHIFT_ON, psy_CTRL_ON,
		"blockdelete", "blkdel");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKINC,
		psy_ui_KEY_F12, psy_SHIFT_OFF, psy_CTRL_ON,
		"transposeblockinc", "Trsp+");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKDEC,
		psy_ui_KEY_F11, psy_SHIFT_OFF, psy_CTRL_ON,
		"transposeblockdec", "Trsp-");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKINC12,
		psy_ui_KEY_F12, psy_SHIFT_ON, psy_CTRL_ON,
		"transposeblockinc12", "Trsp+12");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKDEC12,
		psy_ui_KEY_F11, psy_SHIFT_ON, psy_CTRL_ON,
		"transposeblockdec12", "Trsp-12");

	setcmdall(cmds, CMD_SELECTALL, psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectall", "sel all");
	setcmdall(cmds, CMD_SELECTCOL, psy_ui_KEY_R, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectcol", "sel col");
	setcmdall(cmds, CMD_SELECTBAR, psy_ui_KEY_K, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectbar", "sel bar");
	setcmd(cmds, CMD_SELECTMACHINE, psy_ui_KEY_RETURN, "selectmachine",
		"Sel Mac/Ins");
	setcmdall(cmds, CMD_UNDO, psy_ui_KEY_Z, psy_SHIFT_OFF, psy_CTRL_ON,
		"undo", "undo");
	setcmdall(cmds, CMD_REDO, psy_ui_KEY_Z, psy_SHIFT_ON, psy_CTRL_ON,
		"redo", "redo");
	setcmd(cmds, CMD_DIGIT0, psy_ui_KEY_DIGIT0, "digit0", "0");
	setcmd(cmds, CMD_DIGIT1, psy_ui_KEY_DIGIT1, "digit1", "1");
	setcmd(cmds, CMD_DIGIT2, psy_ui_KEY_DIGIT2, "digit2", "2");
	setcmd(cmds, CMD_DIGIT3, psy_ui_KEY_DIGIT3, "digit3", "3");
	setcmd(cmds, CMD_DIGIT4, psy_ui_KEY_DIGIT4, "digit4", "4");
	setcmd(cmds, CMD_DIGIT5, psy_ui_KEY_DIGIT5, "digit5", "5");
	setcmd(cmds, CMD_DIGIT6, psy_ui_KEY_DIGIT6, "digit6", "6");
	setcmd(cmds, CMD_DIGIT7, psy_ui_KEY_DIGIT7, "digit7", "7");
	setcmd(cmds, CMD_DIGIT8, psy_ui_KEY_DIGIT8, "digit8", "8");
	setcmd(cmds, CMD_DIGIT9, psy_ui_KEY_DIGIT9, "digit9", "9");
	setcmd(cmds, CMD_DIGITA, psy_ui_KEY_A, "digitA", "A");
	setcmd(cmds, CMD_DIGITB, psy_ui_KEY_B, "digitB", "B");
	setcmd(cmds, CMD_DIGITC, psy_ui_KEY_C, "digitC", "C");
	setcmd(cmds, CMD_DIGITD, psy_ui_KEY_D, "digitD", "D");
	setcmd(cmds, CMD_DIGITE, psy_ui_KEY_E, "digitE", "E");
	setcmd(cmds, CMD_DIGITF, psy_ui_KEY_F, "digitF", "F");
}

/*
** Appends a property with shortcut defaults for the keyboard driver
** key		: cmd id used by the trackerview
** text		: "cmds.key" language dictionary key used by the translator
** shorttext: short description for the keyboard help view
** value	: encoded key shortcut (keycode/shift/ctrl)
*/
void setcmdall(psy_Property* cmds, int cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	assert(cmds);

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_set_hint(psy_property_set_text(psy_property_set_short_text(
		psy_property_set_id(psy_property_append_int(cmds, key,
			psy_audio_encodeinput(keycode, shift, ctrl, 0, 0), 0, 0),
			cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}

void setcmd(psy_Property* cmds, int cmd, uint32_t keycode,
	const char* key, const char* shorttext)
{
	assert(cmds);

	setcmdall(cmds, cmd, keycode, psy_SHIFT_OFF, psy_CTRL_OFF, key, shorttext);
}
