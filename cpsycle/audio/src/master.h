// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MASTER_H
#define psy_audio_MASTER_H

#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_audio_Machine machine;
	psy_audio_CustomMachineParam param_info;
	psy_audio_CustomMachineParam param_slider;
	psy_audio_CustomMachineParam param_level;
	psy_audio_Buffer memorybuffer;
	uintptr_t memorybuffersize;
	psy_dsp_amp_t volume;
} psy_audio_Master;

void master_init(psy_audio_Master* self, psy_audio_MachineCallback);
const psy_audio_MachineInfo* master_info(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MASTER_H */
