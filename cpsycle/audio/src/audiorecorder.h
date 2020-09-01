// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_AUDIORECORDER_H
#define psy_audio_AUDIORECORDER_H

#include "custommachine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_audio_AudioRecorder {
	psy_audio_CustomMachine custommachine;
	char drivername[32];
	int _captureidx;
	bool _initialized;
	float _gainvol;
	float* pleftorig;
	float* prightorig;
} psy_audio_AudioRecorder;

void psy_audio_audiorecorder_init(psy_audio_AudioRecorder* self,
	psy_audio_MachineCallback*);
const psy_audio_MachineInfo* psy_audio_audiorecorder_info(void);

INLINE psy_audio_Machine* psy_audio_audiorecorder_base(psy_audio_AudioRecorder*
	self)
{
	return &(self->custommachine.machine);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_AUDIORECORDER_H */
