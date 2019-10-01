// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "noteinputs.h"
#include "cmdsnotes.h"

void InitNoteInputs(NoteInputs* self)
{	
	inputs_init(self);
	inputs_define(self, encodeinput('Z', 0, 0), CMD_NOTE_C_0);
	inputs_define(self, encodeinput('S', 0, 0), CMD_NOTE_CS0);
	inputs_define(self, encodeinput('X', 0, 0), CMD_NOTE_D_0);
	inputs_define(self, encodeinput('D', 0, 0), CMD_NOTE_DS0);
	inputs_define(self, encodeinput('C', 0, 0), CMD_NOTE_E_0);
	inputs_define(self, encodeinput('V', 0, 0), CMD_NOTE_F_0);
	inputs_define(self, encodeinput('G', 0, 0), CMD_NOTE_FS0);
	inputs_define(self, encodeinput('B', 0, 0), CMD_NOTE_G_0);
	inputs_define(self, encodeinput('H', 0, 0), CMD_NOTE_GS0);
	inputs_define(self, encodeinput('N', 0, 0), CMD_NOTE_A_0);
	inputs_define(self, encodeinput('J', 0, 0), CMD_NOTE_AS0);
	inputs_define(self, encodeinput('M', 0, 0), CMD_NOTE_B_0);			
	inputs_define(self, encodeinput('Q', 0, 0), CMD_NOTE_C_1);
	inputs_define(self, encodeinput('2', 0, 0), CMD_NOTE_CS1);
	inputs_define(self, encodeinput('W', 0, 0), CMD_NOTE_D_1);
	inputs_define(self, encodeinput('3', 0, 0), CMD_NOTE_DS1);
	inputs_define(self, encodeinput('E', 0, 0), CMD_NOTE_E_1);
	inputs_define(self, encodeinput('R', 0, 0), CMD_NOTE_F_1);
	inputs_define(self, encodeinput('5', 0, 0), CMD_NOTE_FS1);
	inputs_define(self, encodeinput('T', 0, 0), CMD_NOTE_G_1);
	inputs_define(self, encodeinput('6', 0, 0), CMD_NOTE_GS1);
	inputs_define(self, encodeinput('Y', 0, 0), CMD_NOTE_A_1);
	inputs_define(self, encodeinput('7', 0, 0), CMD_NOTE_AS1);
	inputs_define(self, encodeinput('U', 0, 0), CMD_NOTE_B_1);			
	inputs_define(self, encodeinput('I', 0, 0), CMD_NOTE_C_2);
	inputs_define(self, encodeinput('9', 0, 0), CMD_NOTE_CS2);
	inputs_define(self, encodeinput('O', 0, 0), CMD_NOTE_D_2);
	inputs_define(self, encodeinput('0', 0, 0), CMD_NOTE_DS2);
	inputs_define(self, encodeinput('P', 0, 0), CMD_NOTE_E_2);
	inputs_define(self, encodeinput('1', 0, 0), CMD_NOTE_STOP);
}
