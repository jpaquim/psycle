// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_ADSR_H)
#define DSP_ADSR_H

#include "dsptypes.h"

typedef struct {
	psy_dsp_amp_t value;
	psy_dsp_beat_t time;
	psy_dsp_amp_t minvalue;
	psy_dsp_amp_t maxvalue;
	psy_dsp_beat_t mintime;
	psy_dsp_beat_t maxtime;	
} EnvelopePoint;

void envelopepoint_init(EnvelopePoint*, 
		psy_dsp_seconds_t time,
		psy_dsp_amp_t value,
		psy_dsp_seconds_t mintime,
		psy_dsp_seconds_t maxtime,
		psy_dsp_amp_t minvalue,
		psy_dsp_amp_t maxvalue);

typedef struct {	
	psy_dsp_seconds_t attack;
	psy_dsp_seconds_t decay;
	psy_dsp_percent_t sustain;
	psy_dsp_seconds_t release;
	psy_dsp_seconds_t fastrelease;
} ADSRSettings;

void adsr_settings_init(ADSRSettings*, psy_dsp_seconds_t, psy_dsp_seconds_t,
	psy_dsp_seconds_t, psy_dsp_seconds_t);
void adsr_settings_initdefault(ADSRSettings*);
float adsr_settings_attack(ADSRSettings*);
void adsr_settings_setattack(ADSRSettings*, psy_dsp_seconds_t);
float adsr_settings_decay(ADSRSettings*);
void adsr_settings_setdecay(ADSRSettings*, psy_dsp_seconds_t);
float adsr_settings_sustain(ADSRSettings*);
void adsr_settings_setsustain(ADSRSettings*, psy_dsp_percent_t);
float adsr_settings_release(ADSRSettings*);
void adsr_settings_setrelease(ADSRSettings*, psy_dsp_seconds_t);
float adsr_settings_fastrelease(ADSRSettings*);
void adsr_settings_setfastrelease(ADSRSettings*, psy_dsp_seconds_t);

typedef enum
{
	ENV_OFF = 0,
	ENV_ATTACK = 1,
	ENV_DECAY = 2,
	ENV_SUSTAIN = 3,
	ENV_RELEASE = 4,
	ENV_FASTRELEASE = 5
} EnvelopeStage;

typedef struct {
	psy_dsp_amp_t value;
	psy_dsp_amp_t step;	
	unsigned int samplerate;
	ADSRSettings settings;
	EnvelopeStage stage;
} ADSR;

void adsr_initdefault(ADSR*, unsigned int samplerate);
void adsr_init(ADSR*, const ADSRSettings*, unsigned int samplerate);
void adsr_reset(ADSR*);
void adsr_setsamplerate(ADSR*, unsigned int samplerate);
void adsr_tick(ADSR*);
void adsr_start(ADSR*);
void adsr_release(ADSR*);

#endif
