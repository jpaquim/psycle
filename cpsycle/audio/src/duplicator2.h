// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_DUPLICATOR2_H)
#define PSY_AUDIO_DUPLICATOR2_H

#include "custommachine.h"
#include "duplicatormap.h"

typedef struct {
	psy_audio_CustomMachine custommachine;	
	psy_audio_DuplicatorMap map;
	int isticking;	
} psy_audio_Duplicator2;

void psy_audio_duplicator2_init(psy_audio_Duplicator2*, MachineCallback);
const psy_audio_MachineInfo* psy_audio_duplicator2_info(void);
psy_audio_Machine* psy_audio_duplicator2_base(psy_audio_Duplicator2*);

#endif
