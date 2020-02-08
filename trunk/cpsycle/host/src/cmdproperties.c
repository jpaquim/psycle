// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "cmdproperties.h"
#include "cmdsnotes.h"
#include "cmdsgeneral.h"
#include "inputmap.h"
#include "uidef.h"

static void cmdproperties_makenotes(psy_Properties*);
static void cmdproperties_makegeneral(psy_Properties*);

psy_Properties* cmdproperties_create(void)
{
	psy_Properties* rv;

	rv = psy_properties_create();
	if (rv) {
		cmdproperties_makenotes(rv);
		cmdproperties_makegeneral(rv);
	}
	return rv;
}

void cmdproperties_makenotes(psy_Properties* self)
{
	psy_Properties* notes;
	psy_Properties* p;
	
	notes = psy_properties_create_section(self, "notes");	
	psy_properties_settext(notes, "Notes");
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_c_0",
		encodeinput('Z', 0, 0), 0, 0), CMD_NOTE_C_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_cs0",
		encodeinput('S', 0, 0), 0, 0), CMD_NOTE_CS0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_d_0",
		encodeinput('X', 0, 0), 0, 0), CMD_NOTE_D_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_ds0",
		encodeinput('D', 0, 0), 0, 0), CMD_NOTE_DS0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_e_0",
		encodeinput('C', 0, 0), 0, 0), CMD_NOTE_E_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_f_0",
		encodeinput('V', 0, 0), 0, 0), CMD_NOTE_F_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_fs0",
		encodeinput('G', 0, 0), 0, 0), CMD_NOTE_FS0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_g_0",
		encodeinput('B', 0, 0), 0, 0), CMD_NOTE_G_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_gs0",
		encodeinput('H', 0, 0), 0, 0), CMD_NOTE_GS0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_a_0",
		encodeinput('N', 0, 0), 0, 0), CMD_NOTE_A_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_as0",
		encodeinput('J', 0, 0), 0, 0), CMD_NOTE_AS0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_b_0",
		encodeinput('M', 0, 0), 0, 0), CMD_NOTE_B_0);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_c_1",
		encodeinput('Q', 0, 0), 0, 0), CMD_NOTE_C_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_cs1",
		encodeinput('2', 0, 0), 0, 0), CMD_NOTE_CS1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_d_1",
		encodeinput('W', 0, 0), 0, 0), CMD_NOTE_D_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_ds1",
		encodeinput('3', 0, 0), 0, 0), CMD_NOTE_DS1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_e_1",
		encodeinput('E', 0, 0), 0, 0), CMD_NOTE_E_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_f_1",
		encodeinput('R', 0, 0), 0, 0), CMD_NOTE_F_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_fs1",
		encodeinput('5', 0, 0), 0, 0), CMD_NOTE_FS1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_g_1",
		encodeinput('T', 0, 0), 0, 0), CMD_NOTE_G_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_gs1",
		encodeinput('6', 0, 0), 0, 0), CMD_NOTE_GS1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_a_1",
		encodeinput('Y', 0, 0), 0, 0), CMD_NOTE_A_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_as1",
		encodeinput('7', 0, 0), 0, 0), CMD_NOTE_AS1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_b_1",
		encodeinput('U', 0, 0), 0, 0), CMD_NOTE_B_1);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_c_2",
		encodeinput('I', 0, 0), 0, 0), CMD_NOTE_C_2);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_cs2",
		encodeinput('9', 0, 0), 0, 0), CMD_NOTE_CS2);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_d_2",
		encodeinput('O', 0, 0), 0, 0), CMD_NOTE_D_2);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_ds2",
		encodeinput('0', 0, 0), 0, 0), CMD_NOTE_DS2);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_e_2",
		encodeinput('P', 0, 0), 0, 0), CMD_NOTE_E_2);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_stop",
		encodeinput('1', 0, 0), 0, 0), CMD_NOTE_STOP);
	// special
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_tweakm",
		encodeinput(192, 0, 0), 0, 0), CMD_NOTE_TWEAKM);
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_midicc",
		encodeinput(192, 1, 0), 0, 0), CMD_NOTE_MIDICC);	
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_tweaks",
		encodeinput(192, 0, 1), 0, 0), CMD_NOTE_TWEAKS);		
	for (p = notes->children; p != 0; p = psy_properties_next(p)) {
		psy_properties_sethint(p, PSY_PROPERTY_HINT_INPUT);
	}	
}

void cmdproperties_makegeneral(psy_Properties* self)
{
	psy_Properties* general;
	psy_Properties* p;

	general = psy_properties_create_section(self, "generalcmds");	
	psy_properties_settext(general, "generalcmds");	
	psy_properties_setid(psy_properties_append_int(general, "cmd_editmachine",
		encodeinput(psy_ui_KEY_F2, 0, 0), 0, 0), CMD_IMM_EDITMACHINE);
	psy_properties_setid(psy_properties_append_int(general, "cmd_editpattern",
		encodeinput(psy_ui_KEY_F3, 0, 0), 0, 0), CMD_IMM_EDITPATTERN);
	psy_properties_setid(psy_properties_append_int(general, "cmd_addmachine",
		encodeinput(psy_ui_KEY_F9, 0, 0), 0, 0), CMD_IMM_ADDMACHINE);
	psy_properties_setid(psy_properties_append_int(general, "cmd_playsong",
		encodeinput(psy_ui_KEY_F5, 1, 0), 0, 0), CMD_IMM_PLAYSONG);
	psy_properties_setid(psy_properties_append_int(general, "cmd_playstart",
		encodeinput(psy_ui_KEY_F5, 0, 0), 0, 0), CMD_IMM_PLAYSTART);
	psy_properties_setid(psy_properties_append_int(general, "cmd_playfrompos",
		encodeinput(psy_ui_KEY_F7, 0, 0), 0, 0), CMD_IMM_PLAYFROMPOS);
	psy_properties_setid(psy_properties_append_int(general, "cmd_playstop",
		encodeinput(psy_ui_KEY_F8, 0, 0), 0, 0), CMD_IMM_PLAYSTOP);
	psy_properties_setid(psy_properties_append_int(general, "cmd_songposdec",
		encodeinput(psy_ui_KEY_LEFT, 1, 0), 0, 0), CMD_IMM_SONGPOSDEC);
	psy_properties_setid(psy_properties_append_int(general, "cmd_songposinc",
		encodeinput(psy_ui_KEY_RIGHT, 1, 0), 0, 0), CMD_IMM_SONGPOSINC);
	psy_properties_setid(psy_properties_append_int(general, "cmd_maxpattern",
		encodeinput(psy_ui_KEY_TAB, 0, 1), 0, 0), CMD_IMM_MAXPATTERN);
	psy_properties_setid(psy_properties_append_int(general, "cmd_infomachine",
		encodeinput(psy_ui_KEY_RETURN, 1, 0), 0, 0), CMD_IMM_INFOMACHINE);
	psy_properties_setid(psy_properties_append_int(general, "cmd_editinstr",
		encodeinput(psy_ui_KEY_F10, 0, 0), 0, 0), CMD_IMM_EDITINSTR);
	psy_properties_setid(psy_properties_append_int(general, "cmd_editinstr",
		encodeinput(psy_ui_KEY_F10, 1, 0), 0, 0), CMD_IMM_EDITSAMPLE);
	psy_properties_setid(psy_properties_append_int(general, "cmd_editinstr",
		encodeinput(psy_ui_KEY_F10, 0, 1), 0, 0), CMD_IMM_EDITWAVE);
	psy_properties_setid(psy_properties_append_int(general, "cmd_editinstr",
		encodeinput(psy_ui_KEY_F1, 0, 0), 0, 0), CMD_IMM_HELP);
	psy_properties_setid(psy_properties_append_int(general, "cmd_editinstr",
		encodeinput(psy_ui_KEY_F12, 0, 0), 0, 0), CMD_IMM_TERMINAL);	
	for (p = general->children; p != 0; p = psy_properties_next(p)) {
		psy_properties_sethint(p, PSY_PROPERTY_HINT_INPUT);
	}	
}
