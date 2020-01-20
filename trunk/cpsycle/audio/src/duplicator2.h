// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_DUPLICATOR2_H
#define psy_audio_DUPLICATOR2_H

#include "custommachine.h"
#include "duplicatormap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_audio_CustomMachine custommachine;	
	psy_audio_DuplicatorMap map;
	int isticking;	
} psy_audio_Duplicator2;

void psy_audio_duplicator2_init(psy_audio_Duplicator2*, MachineCallback);
const psy_audio_MachineInfo* psy_audio_duplicator2_info(void);

INLINE psy_audio_Machine* psy_audio_duplicator2_base(psy_audio_Duplicator2* self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_DUPLICATOR2_H */
