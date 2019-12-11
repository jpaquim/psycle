// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MIXER_H)
#define MIXER_H

#include "custommachine.h"
#include <rms.h>

typedef struct {
	Table sendvols;
	float volume;
	float panning;
	float drymix;
	float gain;
	int mute;
	int dryonly;
	int wetonly;
	size_t inputslot;
	Buffer* buffer;	
} MixerChannel;

typedef struct {
	Table sendsto;
	unsigned char mastersend;
	psy_dsp_amp_t volume;
	psy_dsp_amp_t panning;
	int mute;
	size_t fxslot;
	Buffer* buffer;
	Machine* fx;
} ReturnChannel;

typedef struct {
	CustomMachine custommachine;
	Table inputs;
	Table sends;
	Table returns;
	int solocolumn;
	RMSVol masterrmsvol;
	MixerChannel master;
	uintptr_t slot;	
} Mixer;

void mixer_init(Mixer*, MachineCallback);
const MachineInfo* mixer_info(void);


#endif
