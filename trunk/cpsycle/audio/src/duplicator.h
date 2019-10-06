// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUPLICATOR_H)
#define DUPLICATOR_H

#include "machine.h"

#define NUMMACHINES 8

typedef struct {
	Machine machine;
	int macoutput[NUMMACHINES];
	int noteoffset[NUMMACHINES];
	int isticking;
} Duplicator;

void duplicator_init(Duplicator*, MachineCallback);

#endif
