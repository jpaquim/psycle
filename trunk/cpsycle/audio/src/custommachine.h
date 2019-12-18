// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(CUSTOMMACHINE_H)
#define CUSTOMMACHINE_H

#include "machine.h"

typedef struct {
	Machine machine;
	psy_dsp_amp_t pan;
	int ismuted;
	int isbypassed;
	char* editname;
	Buffer memorybuffer;
	uintptr_t memorybuffersize;
} CustomMachine;

void custommachine_init(CustomMachine*, MachineCallback);
void custommachine_dispose(CustomMachine*);

#endif
