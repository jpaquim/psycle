// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_CUSTOMMACHINE_H
#define psy_audio_CUSTOMMACHINE_H

#include "machine.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_CustomMachine
//	
// aim:	more specialized base class for "Machines", the audio producing
// elements, offering more default implementations than psy_audio_Machine
// Most plugins inherit from this class instead from Machine.

typedef struct {
	psy_audio_Machine machine;
	psy_dsp_amp_t pan;
	int ismuted;
	int isbypassed;
	char* editname;
	psy_audio_Buffer memorybuffer;
	uintptr_t memorybuffersize;
	uintptr_t slot;
	double x;
	double y;
} psy_audio_CustomMachine;

void psy_audio_custommachine_init(psy_audio_CustomMachine*, psy_audio_MachineCallback*);
void psy_audio_custommachine_dispose(psy_audio_CustomMachine*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_CUSTOMMACHINE_H */
