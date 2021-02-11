// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_VIRTUALGENERATOR_H
#define psy_audio_VIRTUALGENERATOR_H

// local
#include "custommachine.h"
#include "instruments.h"

#ifdef __cplusplus
extern "C" {
#endif

// Virtual Generators
	
// In essence, a Virtual generator is an alias for a sampler number and
// instrument number. Virtual Generators have an index number from 81 to FE,
// and this index is associated to a pair of "Sampled instrument" and
// "Machine index".
//
// But a Virtual generator is not only an easier way to use a sampled
// instrument. It also offers another great feature: a volume column.
// Since the auxiliary(inst) column remains unused with a virtual generator
// (because the instrument index is already known), it is possible now to add a
// command here that will get translated as the 0Cxx command if the machine is
// a sampler, or as the 1Exx command, if the machine is Sampulse.
// (see Psycle Help v. 1.3.0)

// A VirtualGenerator works like the NoteDuplicator, delegates and translates
// the inst to volume column in VirtualGenerator::Seqtick to the real machine
// with help of the stored inst/machine pair
// Additional the buffer memory of the real machine will be returned by 
// VirtualGenerator::buffermemory. (Todo: return instrument voice buffer, not
// recorded separatly yet, of the sampler). The buffer memory is used in the
// scope monitors. (WireView, PatternTrackScopes)

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
