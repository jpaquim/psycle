// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUPLICATOR_H)
#define DUPLICATOR_H

#include "custommachine.h"
#include "duplicatormap.h"

#define NUMMACHINES 8

typedef struct {
	psy_audio_CustomMachine custommachine;
	short macoutput[NUMMACHINES];
	short noteoffset[NUMMACHINES];
	int isticking;
	psy_audio_DuplicatorMap map;	
} psy_audio_Duplicator;

void duplicator_init(psy_audio_Duplicator*, MachineCallback);
const psy_audio_MachineInfo* duplicator_info(void);

#endif
