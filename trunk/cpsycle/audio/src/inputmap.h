// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INPUTS_H
#define psy_audio_INPUTS_H

#include <hashtbl.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_Table map;
	psy_Table cmdnames;
} psy_audio_Inputs;

void psy_audio_inputs_init(psy_audio_Inputs*);
void psy_audio_inputs_dispose(psy_audio_Inputs*);
void psy_audio_inputs_define(psy_audio_Inputs*, int input, int cmd, const char* cmdname);
int psy_audio_inputs_cmd(psy_audio_Inputs*, int cmd);
const char* psy_audio_inputs_cmdname(psy_audio_Inputs*, int cmd);

INLINE uintptr_t psy_audio_encodeinput(uintptr_t keycode, bool shift, bool ctrl)
{	
	return keycode | ((uintptr_t)shift << 8) | ((uintptr_t)ctrl << 9);
}

INLINE void psy_audio_decodeinput(uintptr_t input, uintptr_t* keycode, bool* shift, bool* ctrl)
{
	*keycode = input & 0xFF;
	*shift = ((input >> 8) & 0x01) == 0x01;
	*ctrl = ((input >> 9) & 0x01) == 0x01;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INPUTS_H */
