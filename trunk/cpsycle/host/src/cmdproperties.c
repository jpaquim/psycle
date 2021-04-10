// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "cmdproperties.h"
#include "cmdsgeneral.h"
#include "uidef.h"
#include "trackerview.h"
#include "pianoroll.h"

#include <cmdsnotes.h>

static void cmdproperties_makenotes(psy_Property*);
static void cmdproperties_makegeneral(psy_Property*);
static void cmdproperties_setinput(psy_Property*);

psy_Property* cmdproperties_create(void)
{
	psy_Property* rv;

	rv = psy_property_allocinit_key("cmds");
	if (rv) {
		cmdproperties_makenotes(rv);
		cmdproperties_makegeneral(rv);
		maketrackercmds(rv);
		pianoroll_makecmds(rv);
	}
	return rv;
}

void cmdproperties_makenotes(psy_Property* self)
{
	psy_Property* notes;	
	
	notes = psy_property_append_section(self, "notes");	
	psy_property_settext(notes, "Notes");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_c_0",
		psy_audio_encodeinput('Z', 0, 0, 0), 0, 0), CMD_NOTE_C_0),
		"C0");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_cs0",
		psy_audio_encodeinput('S', 0, 0, 0), 0, 0), CMD_NOTE_CS0),
		"C#0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_d_0",
		psy_audio_encodeinput('X', 0, 0, 0), 0, 0), CMD_NOTE_D_0),
		"D0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_ds0",
		psy_audio_encodeinput('D', 0, 0, 0), 0, 0), CMD_NOTE_DS0),
		"D#0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_e_0",
		psy_audio_encodeinput('C', 0, 0, 0), 0, 0), CMD_NOTE_E_0),
		"E0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_f_0",
		psy_audio_encodeinput('V', 0, 0, 0), 0, 0), CMD_NOTE_F_0),
		"F0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_fs0",
		psy_audio_encodeinput('G', 0, 0, 0), 0, 0), CMD_NOTE_FS0),
		"F#0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_g_0",
		psy_audio_encodeinput('B', 0, 0, 0), 0, 0), CMD_NOTE_G_0),
		"G0");
	psy_property_settext(
	psy_property_setid(psy_property_append_int(notes, "cmd_note_gs0",
		psy_audio_encodeinput('H', 0, 0, 0), 0, 0), CMD_NOTE_GS0),
		"G#0");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_a_0",
			psy_audio_encodeinput('N', 0, 0, 0), 0, 0), CMD_NOTE_A_0),
		"A0");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_as0",
			psy_audio_encodeinput('J', 0, 0, 0), 0, 0), CMD_NOTE_AS0),
		"A#0");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_b_0",
			psy_audio_encodeinput('M', 0, 0, 0), 0, 0), CMD_NOTE_B_0),
		"B0");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_c_1",
			psy_audio_encodeinput('Q', 0, 0, 0), 0, 0), CMD_NOTE_C_1),
		"C1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_cs1",
			psy_audio_encodeinput('2', 0, 0, 0), 0, 0), CMD_NOTE_CS1),
		"C#1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_d_1",
			psy_audio_encodeinput('W', 0, 0, 0), 0, 0), CMD_NOTE_D_1),
		"D1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_ds1",
			psy_audio_encodeinput('3', 0, 0, 0), 0, 0), CMD_NOTE_DS1),
		"D#1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_e_1",
			psy_audio_encodeinput('E', 0, 0, 0), 0, 0), CMD_NOTE_E_1),
		"E1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_f_1",
			psy_audio_encodeinput('R', 0, 0, 0), 0, 0), CMD_NOTE_F_1),
		"F1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_fs1",
			psy_audio_encodeinput('5', 0, 0, 0), 0, 0), CMD_NOTE_FS1),
		"F#1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_g_1",
			psy_audio_encodeinput('T', 0, 0, 0), 0, 0), CMD_NOTE_G_1),
		"G1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_gs1",
			psy_audio_encodeinput('6', 0, 0, 0), 0, 0), CMD_NOTE_GS1),
		"G#1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_a_1",
			psy_audio_encodeinput('Y', 0, 0, 0), 0, 0), CMD_NOTE_A_1),
		"A1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_as1",
			psy_audio_encodeinput('7', 0, 0, 0), 0, 0), CMD_NOTE_AS1),
		"A#1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_b_1",
			psy_audio_encodeinput('U', 0, 0, 0), 0, 0), CMD_NOTE_B_1),
		"B1");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_c_2",
			psy_audio_encodeinput('I', 0, 0, 0), 0, 0), CMD_NOTE_C_2),
		"C2");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_cs2",
			psy_audio_encodeinput('9', 0, 0, 0), 0, 0), CMD_NOTE_CS2),
		"C#2");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_d_2",
			psy_audio_encodeinput('O', 0, 0, 0), 0, 0), CMD_NOTE_D_2),
		"D2");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_ds2",
			psy_audio_encodeinput('0', 0, 0, 0), 0, 0), CMD_NOTE_DS2),
		"D#2");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_e_2",
			psy_audio_encodeinput('P', 0, 0, 0), 0, 0), CMD_NOTE_E_2),
		"E2");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_stop",
			psy_audio_encodeinput('1', 0, 0, 0), 0, 0), CMD_NOTE_STOP),
		"off");
	// special
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_tweakm",
			psy_audio_encodeinput(192, 0, 0, 0), 0, 0), CMD_NOTE_TWEAKM),
		"twk");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_midicc",
			psy_audio_encodeinput(192, 1, 0, 0), 0, 0), CMD_NOTE_MIDICC),
		"mcm");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(notes, "cmd_note_tweaks",
			psy_audio_encodeinput(192, 0, 1, 0), 0, 0), CMD_NOTE_TWEAKS),
		"tws");
	cmdproperties_setinput(notes);
}

void cmdproperties_makegeneral(psy_Property* self)
{
	psy_Property* general;

	general = psy_property_settext(
		psy_property_append_section(self, "general"),
		"General");	
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_help",
			psy_audio_encodeinput(psy_ui_KEY_F1, 0, 0, 0), 0, 0), CMD_IMM_HELP),
		"hlp"), "cmds.help");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_helpshortcut",
			psy_audio_encodeinput(psy_ui_KEY_F1, 1, 0, 0), 0, 0), CMD_IMM_HELPSHORTCUT),
		"kbdhlp"), "cmds.helpshortcut");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editmachine",
			psy_audio_encodeinput(psy_ui_KEY_F2, 0, 0, 0), 0, 0), CMD_IMM_EDITMACHINE),
		"mac"), "cmds.editmachine"
	);
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editpattern",
			psy_audio_encodeinput(psy_ui_KEY_F3, 0, 0, 0), 0, 0), CMD_IMM_EDITPATTERN),
		"pat"), "cmds.editpattern");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_addmachine",
			psy_audio_encodeinput(psy_ui_KEY_F9, 0, 0, 0), 0, 0), CMD_IMM_ADDMACHINE),
		"addmac"), "cmds.addmachine");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_playsong",
			psy_audio_encodeinput(psy_ui_KEY_F5, 1, 0, 0), 0, 0), CMD_IMM_PLAYSONG),
		"playa"), "cmds.playsong");	
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_playrowtrack",
			psy_audio_encodeinput(psy_ui_KEY_DIGIT4, 0, 0, 0), 0, 0), CMD_IMM_PLAYROWTRACK),
		"playt"), "cmds.playrowtrack");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_playrowpattern",
			psy_audio_encodeinput(psy_ui_KEY_DIGIT8, 0, 0, 0), 0, 0), CMD_IMM_PLAYROWPATTERN),
		"playr"), "cmds.playrowpattern");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_playstart",
			psy_audio_encodeinput(psy_ui_KEY_F5, 0, 0, 0), 0, 0), CMD_IMM_PLAYSTART),
		"plays"), "cmds.playstart");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_playfrompos",
			psy_audio_encodeinput(psy_ui_KEY_F7, 0, 0, 0), 0, 0), CMD_IMM_PLAYFROMPOS),
		"playp"), "cmds.playfrompos");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(general, "cmd_playstop",
			psy_audio_encodeinput(psy_ui_KEY_F8, 0, 0, 0), 0, 0), CMD_IMM_PLAYSTOP),
		"cmds.playstop");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_patterndec",
			psy_audio_encodeinput(psy_ui_KEY_DOWN, 1, 0, 0), 0, 0), CMD_IMM_PATTERNDEC),
		"decpat"), "cmds.patterndec");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_patterninc",
			psy_audio_encodeinput(psy_ui_KEY_UP, 1, 0, 0), 0, 0), CMD_IMM_PATTERNINC),
		"incpat"), "cmds.patterninc");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_songposdec",
			psy_audio_encodeinput(psy_ui_KEY_LEFT, 1, 0, 0), 0, 0), CMD_IMM_SONGPOSDEC),
		"decseqp"), "cmds.songposdec");	
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_songposinc",
			psy_audio_encodeinput(psy_ui_KEY_RIGHT, 1, 0, 0), 0, 0), CMD_IMM_SONGPOSINC),
		"incseqp"), "cmds.songposinc");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_followsong",
			psy_audio_encodeinput(psy_ui_KEY_F, psy_SHIFT_OFF, psy_CTRL_ON, 0), 0, 0),
			CMD_IMM_FOLLOWSONG),
		"follow"), "followsong");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_maxpattern",
			psy_audio_encodeinput(psy_ui_KEY_TAB, 0, 1, 0), 0, 0), CMD_IMM_MAXPATTERN),
		"maxpat"), "cmds.maxpattern");	
	psy_property_settext(
		psy_property_setid(psy_property_append_int(general, "cmd_infopattern",
			psy_audio_encodeinput(psy_ui_KEY_RETURN, 0, 1, 0), 0, 0), CMD_IMM_INFOPATTERN),
		"cmds.infopattern");
	psy_property_settext(
		psy_property_setid(psy_property_append_int(general, "cmd_infomachine",
			psy_audio_encodeinput(psy_ui_KEY_RETURN, 1, 0, 0), 0, 0), CMD_IMM_INFOMACHINE),
		"cmds.infomachine");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editinstr",			
			psy_audio_encodeinput(psy_ui_KEY_F10, 0, 0, 0), 0, 0), CMD_IMM_EDITINSTR),
		"inst"), "cmds.editinstr");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editsample",
			psy_audio_encodeinput(psy_ui_KEY_F10, 1, 0, 0), 0, 0), CMD_IMM_EDITSAMPLE),
		"smpl"), "cmds.editsample");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editwave",
		psy_audio_encodeinput(psy_ui_KEY_F10, 0, 1, 0), 0, 0), CMD_IMM_EDITWAVE),
		"wav"), "cmds.editwave");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_terminal",
			psy_audio_encodeinput(psy_ui_KEY_F12, 0, 0, 0), 0, 0), CMD_IMM_TERMINAL),
		"term"), "cmds.terminal");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_instrdec",
			psy_audio_encodeinput(psy_ui_KEY_DOWN, 0, 1, 0), 0, 0), CMD_IMM_INSTRDEC),
		"inst-1"), "cmds.instrdec");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_instrinc",
			psy_audio_encodeinput(psy_ui_KEY_UP, 0, 1, 0), 0, 0), CMD_IMM_INSTRINC),
		"inst+1"), "cmds.instrinc");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_settings",
			psy_audio_encodeinput(psy_ui_KEY_S, 0, 0, 1), 0, 0), CMD_IMM_SETTINGS),
		"config"), "cmds.settings");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_enableaudio",
			psy_audio_encodeinput(psy_ui_KEY_A, 0, 0, 1), 0, 0), CMD_IMM_ENABLEAUDIO),
		"enableaudio"), "cmds.enableaudio");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_loadsong",
			psy_audio_encodeinput(psy_ui_KEY_O, 0, 1, 0), 0, 0), CMD_IMM_LOADSONG),
		"loadsong"), "cmds.loadsong");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_savesong",
			psy_audio_encodeinput(psy_ui_KEY_S, 0, 1, 0), 0, 0), CMD_IMM_SAVESONG),
		"savesong"), "cmds.savesong");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editquantizedec",
			psy_audio_encodeinput(219, 0, 0, 0), 0, 0), CMD_EDT_EDITQUANTIZEDEC),
		"lineskip-1"), "cmds.editquantizedec");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_editquantizeinc",
			psy_audio_encodeinput(221, 0, 0, 0), 0, 0), CMD_EDT_EDITQUANTIZEINC),
		"lineskip+1"), "cmds.editquantizedec");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_0",
			psy_audio_encodeinput(psy_ui_KEY_Q, 0, 0, 1), 0, 0), CMD_COLUMN_0),
		"column0"), "cmds.column0");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_1",
			psy_audio_encodeinput(psy_ui_KEY_W, 0, 0, 1), 0, 0), CMD_COLUMN_1),
		"column1"), "cmds.column1");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_2",
			psy_audio_encodeinput(psy_ui_KEY_E, 0, 0, 1), 0, 0), CMD_COLUMN_2),
		"column2"), "cmds.column2");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_3",
			psy_audio_encodeinput(psy_ui_KEY_R, 0, 0, 1), 0, 0), CMD_COLUMN_3),
		"column3"), "cmds.column3");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_4",
			psy_audio_encodeinput(psy_ui_KEY_T, 0, 0, 1), 0, 0), CMD_COLUMN_4),
		"column4"), "cmds.column4");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_5",
			psy_audio_encodeinput(psy_ui_KEY_Y, 0, 0, 1), 0, 0), CMD_COLUMN_5),
		"column5"), "cmds.column5");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_6",
			psy_audio_encodeinput(psy_ui_KEY_U, 0, 0, 1), 0, 0), CMD_COLUMN_6),
		"column6"), "cmds.column6");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_7",
			psy_audio_encodeinput(psy_ui_KEY_I, 0, 0, 1), 0, 0), CMD_COLUMN_7),
		"column7"), "cmds.column7");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_8",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_8),
		"column8"), "cmds.column8");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_9",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_9),
		"column9"), "cmds.column9");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_A",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_A),
		"columnA"), "cmds.columnA");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_B",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_B),
		"columnB"), "cmds.columnB");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_C",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_C),
		"columnC"), "cmds.columnC");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_D",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_D),
		"columnD"), "cmds.columnD");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_E",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_E),
		"columnE"), "cmds.columnE");
	psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(general, "cmd_column_F",
			psy_audio_encodeinput(0, 0, 0, 0), 0, 0), CMD_COLUMN_F),
		"columnF"), "cmds.columnF");
	cmdproperties_setinput(general);
}

void cmdproperties_setinput(psy_Property* properties)
{
	if (properties) {
		psy_List* p;

		for (p = psy_property_begin(properties); p != NULL; psy_list_next(&p)) {
			psy_property_sethint((psy_Property*)psy_list_entry(p),
				PSY_PROPERTY_HINT_SHORTCUT);
		}
	}
}
