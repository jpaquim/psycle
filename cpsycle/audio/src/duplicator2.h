// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUPLICATOR2_H)
#define DUPLICATOR2_H

#include "machine.h"
#include "duplicatormap.h"

#define NUMMACHINES 8

typedef struct {
	Machine machine;
	int macoutput[NUMMACHINES];
	int noteoffset[NUMMACHINES];
	int isticking;
	char lowkey[256];
	char highkey[256];
	DuplicatorMap map;
} Duplicator2;

void duplicator2_init(Duplicator2*, MachineCallback);
const CMachineInfo* duplicator2_info(void);

#endif
