// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "noteinputs.h"
#include "cmdsnotes.h"

void InitNoteInputs(NoteInputs* self)
{	
	InitInputMap(&self->map);
	DefineInput(&self->map, 'Z', CMD_NOTE_C_0);
	DefineInput(&self->map, 'S', CMD_NOTE_CS0);
	DefineInput(&self->map, 'X', CMD_NOTE_D_0);
	DefineInput(&self->map, 'D', CMD_NOTE_DS0);
	DefineInput(&self->map, 'C', CMD_NOTE_E_0);
	DefineInput(&self->map, 'V', CMD_NOTE_F_0);
	DefineInput(&self->map, 'G', CMD_NOTE_FS0);
	DefineInput(&self->map, 'B', CMD_NOTE_G_0);
	DefineInput(&self->map, 'H', CMD_NOTE_GS0);
	DefineInput(&self->map, 'N', CMD_NOTE_A_0);
	DefineInput(&self->map, 'J', CMD_NOTE_AS0);
	DefineInput(&self->map, 'M', CMD_NOTE_B_0);			
	DefineInput(&self->map, 'Q', CMD_NOTE_C_1);
	DefineInput(&self->map, '2', CMD_NOTE_CS1);
	DefineInput(&self->map, 'W', CMD_NOTE_D_1);
	DefineInput(&self->map, '3', CMD_NOTE_DS1);
	DefineInput(&self->map, 'E', CMD_NOTE_E_1);
	DefineInput(&self->map, 'R', CMD_NOTE_F_1);
	DefineInput(&self->map, '5', CMD_NOTE_FS1);
	DefineInput(&self->map, 'T', CMD_NOTE_G_1);
	DefineInput(&self->map, '6', CMD_NOTE_GS1);
	DefineInput(&self->map, 'Y', CMD_NOTE_A_1);
	DefineInput(&self->map, '7', CMD_NOTE_AS1);
	DefineInput(&self->map, 'U', CMD_NOTE_B_1);			
	DefineInput(&self->map, 'I', CMD_NOTE_C_2);
	DefineInput(&self->map, '9', CMD_NOTE_CS2);
	DefineInput(&self->map, 'O', CMD_NOTE_D_2);
	DefineInput(&self->map, '0', CMD_NOTE_DS2);
	DefineInput(&self->map, 'P', CMD_NOTE_E_2);
	DefineInput(&self->map, '1', CMD_NOTE_STOP);
}
