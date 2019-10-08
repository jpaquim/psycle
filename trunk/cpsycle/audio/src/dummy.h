// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUMMY_H)
#define DUMMY_H

#include "machine.h"

typedef struct {
	Machine machine;
	int mode;
} DummyMachine;

void dummymachine_init(DummyMachine* self, MachineCallback);
const CMachineInfo* dummymachine_info(void);

#endif
