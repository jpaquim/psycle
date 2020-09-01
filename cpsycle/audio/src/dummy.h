// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_DUMMY_H
#define psy_audio_DUMMY_H

#include "custommachine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_audio_CustomMachine custommachine;
	int mode;
} psy_audio_DummyMachine;

void dummymachine_init(psy_audio_DummyMachine*, psy_audio_MachineCallback*);
const psy_audio_MachineInfo* dummymachine_info(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_DUMMY_H */
