// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIXER_H
#define psy_audio_MIXER_H

#include "custommachine.h"
#include <rms.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_Table sendvols;
	float volume;
	float panning;
	float drymix;
	float gain;
	int mute;
	int dryonly;
	int wetonly;
	size_t inputslot;
	psy_audio_Buffer* buffer;
	psy_dsp_amp_t volumedisplay;	
} psy_audio_MixerChannel;

typedef struct {
	psy_Table sendsto;
	unsigned char mastersend;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	int mute;
	size_t fxslot;
	psy_audio_Buffer* buffer;
	psy_audio_Machine* fx;
} psy_audio_ReturnChannel;

typedef struct {
	psy_audio_CustomMachine custommachine;
	psy_Table inputs;
	psy_Table sends;
	psy_Table returns;
	int solocolumn;	
	psy_audio_MixerChannel master;
	psy_dsp_amp_t mastervolumedisplay;
	uintptr_t slot;	
	psy_Table rms;
} psy_audio_Mixer;

void mixer_init(psy_audio_Mixer*, MachineCallback);
const psy_audio_MachineInfo* mixer_info(void);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MIXER_H */
