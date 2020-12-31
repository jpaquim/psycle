// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VIRTUALGENERATOR_H
#define psy_audio_VIRTUALGENERATOR_H

// local
#include "custommachine.h"
#include "instruments.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_VirtualGenerator {
	// inherits
	psy_audio_CustomMachine custommachine;	
	// internal data
	uintptr_t machine_index;
	// index subslot used as instrument index
	// instrument group is not used, because the sammpler defines it
	psy_audio_InstrumentIndex instrument_index;
	// parameters
	psy_audio_UIntPtrMachineParam param_inst;
	psy_audio_UIntPtrMachineParam param_sampler;
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
