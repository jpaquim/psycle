// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(CUSTOMMACHINE_H)
#define CUSTOMMACHINE_H

#include "machine.h"

typedef struct {
	psy_audio_Machine machine;
	psy_dsp_amp_t pan;
	int ismuted;
	int isbypassed;
	char* editname;
	psy_audio_Buffer memorybuffer;
	uintptr_t memorybuffersize;
	uintptr_t slot;
} psy_audio_CustomMachine;

void custommachine_init(psy_audio_CustomMachine*, MachineCallback);
void custommachine_dispose(psy_audio_CustomMachine*);

#endif
