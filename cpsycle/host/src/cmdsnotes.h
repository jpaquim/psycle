// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CMDSNOTES)
#define CMDNOTES

enum {
	CMD_NOTE_C_0,
	CMD_NOTE_CS0,
	CMD_NOTE_D_0,
	CMD_NOTE_DS0,
	CMD_NOTE_E_0,
	CMD_NOTE_F_0,
	CMD_NOTE_FS0,
	CMD_NOTE_G_0,
	CMD_NOTE_GS0,
	CMD_NOTE_A_0,
	CMD_NOTE_AS0,
	CMD_NOTE_B_0,
	CMD_NOTE_C_1, ///< 12
	CMD_NOTE_CS1,
	CMD_NOTE_D_1,
	CMD_NOTE_DS1,
	CMD_NOTE_E_1,
	CMD_NOTE_F_1,
	CMD_NOTE_FS1,
	CMD_NOTE_G_1,
	CMD_NOTE_GS1,
	CMD_NOTE_A_1,
	CMD_NOTE_AS1,
	CMD_NOTE_B_1,
	CMD_NOTE_C_2, ///< 24
	CMD_NOTE_CS2,
	CMD_NOTE_D_2,
	CMD_NOTE_DS2,
	CMD_NOTE_E_2,
	CMD_NOTE_F_2,
	CMD_NOTE_FS2,
	CMD_NOTE_G_2,
	CMD_NOTE_GS2,
	CMD_NOTE_A_2,
	CMD_NOTE_AS2,
	CMD_NOTE_B_2,
	CMD_NOTE_C_3, ///< 36
	CMD_NOTE_CS3,
	CMD_NOTE_D_3,
	CMD_NOTE_DS3,
	CMD_NOTE_E_3,
	CMD_NOTE_F_3,
	CMD_NOTE_FS3,
	CMD_NOTE_G_3,
	CMD_NOTE_GS3,
	CMD_NOTE_A_3,
	CMD_NOTE_AS3,
	CMD_NOTE_B_3,
	CMD_NOTE_STOP = 120,	///< NOTE STOP
	CMD_NOTE_TWEAKM = 121,	///< tweak
	//CMD_NOTE_TweakE = 122,	///< tweak effect. Old! No longer used.
	CMD_NOTE_MIDICC = 123,	///< Mcm Command (MIDI CC)
	CMD_NOTE_TWEAKS = 124,	///< tweak slide command
};

#endif
