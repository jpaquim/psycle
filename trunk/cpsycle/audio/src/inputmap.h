// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_INPUTS_H)
#define PSY_AUDIO_INPUTS_H

#include <hashtbl.h>

typedef struct {
	psy_Table map;	
} Inputs;

void inputs_init(Inputs*);
void inputs_dispose(Inputs*);
void inputs_define(Inputs*, int input, int cmd);
int inputs_cmd(Inputs*, int input);

unsigned int encodeinput(unsigned int keycode, int shift, int ctrl);
void decodeinput(unsigned int input, unsigned int* keycode, int* shift, int* ctrl);

#endif
