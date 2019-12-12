// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "cmdproperties.h"
#include "cmdsnotes.h"
#include "cmdsgeneral.h"
#include "inputmap.h"
#include <windows.h> // Virtual keycodes

static void cmdproperties_makenotes(Properties*);
static void cmdproperties_makegeneral(Properties*);

Properties* cmdproperties_create(void)
{
	Properties* rv;

	rv = properties_create();
	if (rv) {
		cmdproperties_makenotes(rv);
		cmdproperties_makegeneral(rv);
	}
	return rv;
}

void cmdproperties_makenotes(Properties* self)
{
	Properties* notes;
	Properties* p;
	
	notes = properties_createsection(self, "notes");	
	properties_settext(notes, "Notes");
	properties_setid(properties_append_int(notes, "cmd_note_c_0",
		encodeinput('Z', 0, 0), 0, 0), CMD_NOTE_C_0);
	properties_setid(properties_append_int(notes, "cmd_note_cs0",
		encodeinput('S', 0, 0), 0, 0), CMD_NOTE_CS0);
	properties_setid(properties_append_int(notes, "cmd_note_d_0",
		encodeinput('X', 0, 0), 0, 0), CMD_NOTE_D_0);
	properties_setid(properties_append_int(notes, "cmd_note_ds0",
		encodeinput('D', 0, 0), 0, 0), CMD_NOTE_DS0);
	properties_setid(properties_append_int(notes, "cmd_note_e_0",
		encodeinput('C', 0, 0), 0, 0), CMD_NOTE_E_0);
	properties_setid(properties_append_int(notes, "cmd_note_f_0",
		encodeinput('V', 0, 0), 0, 0), CMD_NOTE_F_0);
	properties_setid(properties_append_int(notes, "cmd_note_fs0",
		encodeinput('G', 0, 0), 0, 0), CMD_NOTE_FS0);
	properties_setid(properties_append_int(notes, "cmd_note_g_0",
		encodeinput('B', 0, 0), 0, 0), CMD_NOTE_G_0);
	properties_setid(properties_append_int(notes, "cmd_note_gs0",
		encodeinput('H', 0, 0), 0, 0), CMD_NOTE_GS0);
	properties_setid(properties_append_int(notes, "cmd_note_a_0",
		encodeinput('N', 0, 0), 0, 0), CMD_NOTE_A_0);
	properties_setid(properties_append_int(notes, "cmd_note_as0",
		encodeinput('J', 0, 0), 0, 0), CMD_NOTE_AS0);
	properties_setid(properties_append_int(notes, "cmd_note_b_0",
		encodeinput('M', 0, 0), 0, 0), CMD_NOTE_B_0);
	properties_setid(properties_append_int(notes, "cmd_note_c_1",
		encodeinput('Q', 0, 0), 0, 0), CMD_NOTE_C_1);
	properties_setid(properties_append_int(notes, "cmd_note_cs1",
		encodeinput('2', 0, 0), 0, 0), CMD_NOTE_CS1);
	properties_setid(properties_append_int(notes, "cmd_note_d_1",
		encodeinput('W', 0, 0), 0, 0), CMD_NOTE_D_1);
	properties_setid(properties_append_int(notes, "cmd_note_ds1",
		encodeinput('3', 0, 0), 0, 0), CMD_NOTE_DS1);
	properties_setid(properties_append_int(notes, "cmd_note_e_1",
		encodeinput('E', 0, 0), 0, 0), CMD_NOTE_E_1);
	properties_setid(properties_append_int(notes, "cmd_note_f_1",
		encodeinput('R', 0, 0), 0, 0), CMD_NOTE_F_1);
	properties_setid(properties_append_int(notes, "cmd_note_fs1",
		encodeinput('5', 0, 0), 0, 0), CMD_NOTE_FS1);
	properties_setid(properties_append_int(notes, "cmd_note_g_1",
		encodeinput('T', 0, 0), 0, 0), CMD_NOTE_G_1);
	properties_setid(properties_append_int(notes, "cmd_note_gs1",
		encodeinput('6', 0, 0), 0, 0), CMD_NOTE_GS1);
	properties_setid(properties_append_int(notes, "cmd_note_a_1",
		encodeinput('Y', 0, 0), 0, 0), CMD_NOTE_A_1);
	properties_setid(properties_append_int(notes, "cmd_note_as1",
		encodeinput('7', 0, 0), 0, 0), CMD_NOTE_AS1);
	properties_setid(properties_append_int(notes, "cmd_note_b_1",
		encodeinput('U', 0, 0), 0, 0), CMD_NOTE_B_1);
	properties_setid(properties_append_int(notes, "cmd_note_c_2",
		encodeinput('I', 0, 0), 0, 0), CMD_NOTE_C_2);
	properties_setid(properties_append_int(notes, "cmd_note_cs2",
		encodeinput('9', 0, 0), 0, 0), CMD_NOTE_CS2);
	properties_setid(properties_append_int(notes, "cmd_note_d_2",
		encodeinput('O', 0, 0), 0, 0), CMD_NOTE_D_2);
	properties_setid(properties_append_int(notes, "cmd_note_ds2",
		encodeinput('0', 0, 0), 0, 0), CMD_NOTE_DS2);
	properties_setid(properties_append_int(notes, "cmd_note_e_2",
		encodeinput('P', 0, 0), 0, 0), CMD_NOTE_E_2);
	properties_setid(properties_append_int(notes, "cmd_note_stop",
		encodeinput('1', 0, 0), 0, 0), CMD_NOTE_STOP);
	// special
	properties_setid(properties_append_int(notes, "cmd_note_tweakm",
		encodeinput(192, 0, 0), 0, 0), CMD_NOTE_TWEAKM);
	properties_setid(properties_append_int(notes, "cmd_note_tweaks",
		encodeinput(192, 0, 1), 0, 0), CMD_NOTE_TWEAKS);		
	for (p = notes->children; p != 0; p = properties_next(p)) {
		properties_sethint(p, PROPERTY_HINT_INPUT);
	}	
}

void cmdproperties_makegeneral(Properties* self)
{
	Properties* general;
	Properties* p;

	general = properties_createsection(self, "generalcmds");	
	properties_settext(general, "generalcmds");	
	properties_setid(properties_append_int(general, "cmd_editmachine",
		encodeinput(VK_F2, 0, 0), 0, 0), CMD_IMM_EDITMACHINE);
	properties_setid(properties_append_int(general, "cmd_editpattern",
		encodeinput(VK_F3, 0, 0), 0, 0), CMD_IMM_EDITPATTERN);
	properties_setid(properties_append_int(general, "cmd_addmachine",
		encodeinput(VK_F9, 0, 0), 0, 0), CMD_IMM_ADDMACHINE);
	properties_setid(properties_append_int(general, "cmd_playsong",
		encodeinput(VK_F5, 1, 0), 0, 0), CMD_IMM_PLAYSONG);
	properties_setid(properties_append_int(general, "cmd_playstart",
		encodeinput(VK_F5, 0, 0), 0, 0), CMD_IMM_PLAYSTART);
	properties_setid(properties_append_int(general, "cmd_playfrompos",
		encodeinput(VK_F7, 0, 0), 0, 0), CMD_IMM_PLAYFROMPOS);
	properties_setid(properties_append_int(general, "cmd_playstop",
		encodeinput(VK_F8, 0, 0), 0, 0), CMD_IMM_PLAYSTOP);
	properties_setid(properties_append_int(general, "cmd_songposdec",
		encodeinput(VK_LEFT, 1, 0), 0, 0), CMD_IMM_SONGPOSDEC);
	properties_setid(properties_append_int(general, "cmd_songposinc",
		encodeinput(VK_RIGHT, 1, 0), 0, 0), CMD_IMM_SONGPOSINC);
	properties_setid(properties_append_int(general, "cmd_maxpattern",
		encodeinput(VK_TAB, 0, 1), 0, 0), CMD_IMM_MAXPATTERN);
	properties_setid(properties_append_int(general, "cmd_infomachine",
		encodeinput(VK_RETURN, 1, 0), 0, 0), CMD_IMM_INFOMACHINE);
	properties_setid(properties_append_int(general, "cmd_editinstr",
		encodeinput(VK_F10, 0, 0), 0, 0), CMD_IMM_EDITINSTR);
	properties_setid(properties_append_int(general, "cmd_editinstr",
		encodeinput(VK_F10, 1, 0), 0, 0), CMD_IMM_EDITSAMPLE);
	properties_setid(properties_append_int(general, "cmd_editinstr",
		encodeinput(VK_F10, 0, 1), 0, 0), CMD_IMM_EDITWAVE);
	properties_setid(properties_append_int(general, "cmd_editinstr",
		encodeinput(VK_F1, 0, 0), 0, 0), CMD_IMM_HELP);
	properties_setid(properties_append_int(general, "cmd_editinstr",
		encodeinput(VK_F12, 0, 0), 0, 0), CMD_IMM_TERMINAL);	
	for (p = general->children; p != 0; p = properties_next(p)) {
		properties_sethint(p, PROPERTY_HINT_INPUT);
	}	
}
