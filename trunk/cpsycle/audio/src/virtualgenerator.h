// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VIRTUALGENERATOR_H
#define psy_audio_VIRTUALGENERATOR_H

#include "custommachine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_VirtualGenerator {
	// inherits
	psy_audio_CustomMachine custommachine;	
	// internal data
	int32_t machine_index;
	int32_t instrument_index;
	psy_audio_IntMachineParam param_inst;
	psy_audio_IntMachineParam param_sampler;
} psy_audio_VirtualGenerator;

void psy_audio_virtualgenerator_init(psy_audio_VirtualGenerator*,
	psy_audio_MachineCallback*,
	uintptr_t macindex, uintptr_t instindex);
const psy_audio_MachineInfo* psy_audio_virtualgenerator_info(void);

void psy_audio_virtualgenerator_setinstrument(psy_audio_VirtualGenerator*,
	uintptr_t index);
uintptr_t psy_audio_virtualgenerator_instrumentindex(const
	psy_audio_VirtualGenerator*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_VIRTUALGENERATOR_H */
