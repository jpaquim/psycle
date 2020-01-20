// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_INPUTS_H
#define psy_audio_INPUTS_H

#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_Table map;	
} Inputs;

void inputs_init(Inputs*);
void inputs_dispose(Inputs*);
void inputs_define(Inputs*, int input, int cmd);
int inputs_cmd(Inputs*, int input);

unsigned int encodeinput(unsigned int keycode, int shift, int ctrl);
void decodeinput(unsigned int input, unsigned int* keycode, int* shift, int* ctrl);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_INPUTS_H */
