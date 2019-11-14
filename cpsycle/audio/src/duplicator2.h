// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUPLICATOR2_H)
#define DUPLICATOR2_H

#include "custommachine.h"
#include "duplicatormap.h"

typedef struct {
	CustomMachine custommachine;
	short macoutput[16];
	short noteoffset[16];	
	short lowkey[16];
	short highkey[16];
	DuplicatorMap map;	
	int isticking;
} Duplicator2;

void duplicator2_init(Duplicator2*, MachineCallback);
const MachineInfo* duplicator2_info(void);

#endif
