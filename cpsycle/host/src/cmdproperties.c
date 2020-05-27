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
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_c_0",
		psy_audio_encodeinput('Z', 0, 0), 0, 0), CMD_NOTE_C_0),
		"C0");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_cs0",
		psy_audio_encodeinput('S', 0, 0), 0, 0), CMD_NOTE_CS0),
		"C#0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_d_0",
		psy_audio_encodeinput('X', 0, 0), 0, 0), CMD_NOTE_D_0),
		"D0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_ds0",
		psy_audio_encodeinput('D', 0, 0), 0, 0), CMD_NOTE_DS0),
		"D#0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_e_0",
		psy_audio_encodeinput('C', 0, 0), 0, 0), CMD_NOTE_E_0),
		"E0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_f_0",
		psy_audio_encodeinput('V', 0, 0), 0, 0), CMD_NOTE_F_0),
		"F0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_fs0",
		psy_audio_encodeinput('G', 0, 0), 0, 0), CMD_NOTE_FS0),
		"F#0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_g_0",
		psy_audio_encodeinput('B', 0, 0), 0, 0), CMD_NOTE_G_0),
		"G0");
	psy_properties_settext(
	psy_properties_setid(psy_properties_append_int(notes, "cmd_note_gs0",
		psy_audio_encodeinput('H', 0, 0), 0, 0), CMD_NOTE_GS0),
		"G#0");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_a_0",
			psy_audio_encodeinput('N', 0, 0), 0, 0), CMD_NOTE_A_0),
		"A0");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_as0",
			psy_audio_encodeinput('J', 0, 0), 0, 0), CMD_NOTE_AS0),
		"A#0");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_b_0",
			psy_audio_encodeinput('M', 0, 0), 0, 0), CMD_NOTE_B_0),
		"B0");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_c_1",
			psy_audio_encodeinput('Q', 0, 0), 0, 0), CMD_NOTE_C_1),
		"C1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_cs1",
			psy_audio_encodeinput('2', 0, 0), 0, 0), CMD_NOTE_CS1),
		"C#1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_d_1",
			psy_audio_encodeinput('W', 0, 0), 0, 0), CMD_NOTE_D_1),
		"D1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_ds1",
			psy_audio_encodeinput('3', 0, 0), 0, 0), CMD_NOTE_DS1),
		"D#1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_e_1",
			psy_audio_encodeinput('E', 0, 0), 0, 0), CMD_NOTE_E_1),
		"E1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_f_1",
			psy_audio_encodeinput('R', 0, 0), 0, 0), CMD_NOTE_F_1),
		"F1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_fs1",
			psy_audio_encodeinput('5', 0, 0), 0, 0), CMD_NOTE_FS1),
		"F#1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_g_1",
			psy_audio_encodeinput('T', 0, 0), 0, 0), CMD_NOTE_G_1),
		"G1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_gs1",
			psy_audio_encodeinput('6', 0, 0), 0, 0), CMD_NOTE_GS1),
		"G#1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_a_1",
			psy_audio_encodeinput('Y', 0, 0), 0, 0), CMD_NOTE_A_1),
		"A1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_as1",
			psy_audio_encodeinput('7', 0, 0), 0, 0), CMD_NOTE_AS1),
		"A#1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_b_1",
			psy_audio_encodeinput('U', 0, 0), 0, 0), CMD_NOTE_B_1),
		"B1");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_c_2",
			psy_audio_encodeinput('I', 0, 0), 0, 0), CMD_NOTE_C_2),
		"C2");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_cs2",
			psy_audio_encodeinput('9', 0, 0), 0, 0), CMD_NOTE_CS2),
		"C#2");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_d_2",
			psy_audio_encodeinput('O', 0, 0), 0, 0), CMD_NOTE_D_2),
		"D2");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_ds2",
			psy_audio_encodeinput('0', 0, 0), 0, 0), CMD_NOTE_DS2),
		"D#2");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_e_2",
			psy_audio_encodeinput('P', 0, 0), 0, 0), CMD_NOTE_E_2),
		"E2");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_stop",
			psy_audio_encodeinput('1', 0, 0), 0, 0), CMD_NOTE_STOP),
		"off");
	// special
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_tweakm",
			psy_audio_encodeinput(192, 0, 0), 0, 0), CMD_NOTE_TWEAKM),
		"twk");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_midicc",
			psy_audio_encodeinput(192, 1, 0), 0, 0), CMD_NOTE_MIDICC),
		"mcm");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(notes, "cmd_note_tweaks",
			psy_audio_encodeinput(192, 0, 1), 0, 0), CMD_NOTE_TWEAKS),
		"tws");
	for (p = notes->children; p != NULL; p = psy_properties_next(p)) {
		psy_properties_sethint(p, PSY_PROPERTY_HINT_INPUT);
	}	
}

void cmdproperties_makegeneral(psy_Properties* self)
{
	psy_Properties* general;
	psy_Properties* p;

	general = psy_properties_create_section(self, "generalcmds");	
	psy_properties_settext(general, "generalcmds");	
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_help",
			psy_audio_encodeinput(psy_ui_KEY_F1, 0, 0), 0, 0), CMD_IMM_HELP),
		"hlp"), "Help");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_helpshortcut",
			psy_audio_encodeinput(psy_ui_KEY_F1, 1, 0), 0, 0), CMD_IMM_HELPSHORTCUT),
		"kbdhlp"), "Kbd Help");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_editmachine",
			psy_audio_encodeinput(psy_ui_KEY_F2, 0, 0), 0, 0), CMD_IMM_EDITMACHINE),
		"mac"), "Machines"
	);
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_editpattern",
			psy_audio_encodeinput(psy_ui_KEY_F3, 0, 0), 0, 0), CMD_IMM_EDITPATTERN),
		"pat"), "Patterns");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_addmachine",
			psy_audio_encodeinput(psy_ui_KEY_F9, 0, 0), 0, 0), CMD_IMM_ADDMACHINE),
		"addmac"), "Add Machine");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_playsong",
			psy_audio_encodeinput(psy_ui_KEY_F5, 1, 0), 0, 0), CMD_IMM_PLAYSONG),
		"playa"), "Play Song");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_playstart",
			psy_audio_encodeinput(psy_ui_KEY_F5, 0, 0), 0, 0), CMD_IMM_PLAYSTART),
		"plays"), "Play Start");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_playfrompos",
			psy_audio_encodeinput(psy_ui_KEY_F7, 0, 0), 0, 0), CMD_IMM_PLAYFROMPOS),
		"playp"), "Play Pos");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_playstop",
			psy_audio_encodeinput(psy_ui_KEY_F8, 0, 0), 0, 0), CMD_IMM_PLAYSTOP),
		"stop");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_songposdec",
			psy_audio_encodeinput(psy_ui_KEY_LEFT, 1, 0), 0, 0), CMD_IMM_SONGPOSDEC),
		"decseqp"), "Seqpos dec");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_songposinc",
			psy_audio_encodeinput(psy_ui_KEY_RIGHT, 1, 0), 0, 0), CMD_IMM_SONGPOSINC),
		"incseqp"), "Seqpos Inc");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_maxpattern",
			psy_audio_encodeinput(psy_ui_KEY_TAB, 0, 1), 0, 0), CMD_IMM_MAXPATTERN),
		"maxpat"), "Max Pattern");
	psy_properties_settext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_infomachine",
			psy_audio_encodeinput(psy_ui_KEY_RETURN, 1, 0), 0, 0), CMD_IMM_INFOMACHINE),
		"gear");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_editinstr",			
			psy_audio_encodeinput(psy_ui_KEY_F10, 0, 0), 0, 0), CMD_IMM_EDITINSTR),
		"inst"), "Instruments");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_editsample",
			psy_audio_encodeinput(psy_ui_KEY_F10, 1, 0), 0, 0), CMD_IMM_EDITSAMPLE),
		"smpl"), "Samples");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_editwave",
		psy_audio_encodeinput(psy_ui_KEY_F10, 0, 1), 0, 0), CMD_IMM_EDITWAVE),
		"wav"), "Wave Edit");
	psy_properties_settext(psy_properties_setshorttext(
		psy_properties_setid(psy_properties_append_int(general, "cmd_terminal",
			psy_audio_encodeinput(psy_ui_KEY_F12, 0, 0), 0, 0), CMD_IMM_TERMINAL),
		"term"), "Terminal");
	for (p = general->children; p != NULL; p = psy_properties_next(p)) {
		psy_properties_sethint(p, PSY_PROPERTY_HINT_INPUT);
	}	
}
