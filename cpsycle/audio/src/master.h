// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MASTER_H
#define psy_audio_MASTER_H

#include "custommachine.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Master
// 
// root node of Machines and mini mixer. Any machine that wants to produce
// sound must be connected to the master. The player copies the output
// buffer of the master to the current audio driver buffer. The master
// handles global volume changes redirected by the Sequencer and the
// connected input wires can be tweaked
//
// Structure:
//      psy_audio_Machine
//            ^
//            |
//      psy_audio_Master

typedef struct psy_audio_Master {
	psy_audio_CustomMachine custommachine;
	psy_audio_CustomMachineParam param_info;
	psy_audio_CustomMachineParam param_slider;
	psy_audio_CustomMachineParam param_level;	
	psy_dsp_amp_t volume;
	uintptr_t strobe;
	bool dostart;
} psy_audio_Master;

void psy_audio_master_init(psy_audio_Master* self, psy_audio_MachineCallback*);
const psy_audio_MachineInfo* psy_audio_master_info(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MASTER_H */
