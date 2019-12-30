// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_DSP_ADSR_H)
#define PSY_DSP_ADSR_H

#include "dsptypes.h"

typedef struct {
	psy_dsp_amp_t value;
	psy_dsp_beat_t time;
	psy_dsp_amp_t minvalue;
	psy_dsp_amp_t maxvalue;
	psy_dsp_beat_t mintime;
	psy_dsp_beat_t maxtime;	
} psy_dsp_EnvelopePoint;

void psy_dsp_envelopepoint_init(psy_dsp_EnvelopePoint*, 
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
} psy_dsp_ADSRSettings;

void adsr_settings_init(psy_dsp_ADSRSettings*, psy_dsp_seconds_t, psy_dsp_seconds_t,
	psy_dsp_seconds_t, psy_dsp_seconds_t);
void adsr_settings_initdefault(psy_dsp_ADSRSettings*);
float adsr_settings_attack(psy_dsp_ADSRSettings*);
void adsr_settings_setattack(psy_dsp_ADSRSettings*, psy_dsp_seconds_t);
float adsr_settings_decay(psy_dsp_ADSRSettings*);
void adsr_settings_setdecay(psy_dsp_ADSRSettings*, psy_dsp_seconds_t);
float adsr_settings_sustain(psy_dsp_ADSRSettings*);
void adsr_settings_setsustain(psy_dsp_ADSRSettings*, psy_dsp_percent_t);
float adsr_settings_release(psy_dsp_ADSRSettings*);
void adsr_settings_setrelease(psy_dsp_ADSRSettings*, psy_dsp_seconds_t);
float adsr_settings_fastrelease(psy_dsp_ADSRSettings*);
void adsr_settings_setfastrelease(psy_dsp_ADSRSettings*, psy_dsp_seconds_t);

typedef enum
{
	ENV_OFF = 0,
	ENV_ATTACK = 1,
	ENV_DECAY = 2,
	ENV_SUSTAIN = 3,
	ENV_RELEASE = 4,
	ENV_FASTRELEASE = 5
} psy_dsp_EnvelopeStage;

typedef struct {
	psy_dsp_amp_t value;
	psy_dsp_amp_t step;	
	unsigned int samplerate;
	psy_dsp_ADSRSettings settings;
	psy_dsp_EnvelopeStage stage;
} psy_dsp_ADSR;

void psy_dsp_adsr_initdefault(psy_dsp_ADSR*, unsigned int samplerate);
void psy_dsp_adsr_init(psy_dsp_ADSR*, const psy_dsp_ADSRSettings*, unsigned int samplerate);
void psy_dsp_adsr_reset(psy_dsp_ADSR*);
void psy_dsp_adsr_setsamplerate(psy_dsp_ADSR*, unsigned int samplerate);
void psy_dsp_adsr_tick(psy_dsp_ADSR*);
void psy_dsp_adsr_start(psy_dsp_ADSR*);
void psy_dsp_adsr_release(psy_dsp_ADSR*);

#endif
