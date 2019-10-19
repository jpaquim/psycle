// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_ADSR_H)
#define DSP_ADSR_H

#include "dsptypes.h"

typedef struct {
	amp_t value;
	beat_t time;
	amp_t minvalue;
	amp_t maxvalue;
	beat_t mintime;
	beat_t maxtime;	
} EnvelopePoint;

void envelopepoint_init(EnvelopePoint*, 
		seconds_t time, amp_t value,
		seconds_t mintime, seconds_t maxtime,
		amp_t minvalue, amp_t maxvalue);

typedef struct {	
	seconds_t attack;
	seconds_t decay;
	percent_t sustain;
	seconds_t release;
	seconds_t fastrelease;
} ADSRSettings;

void adsr_settings_init(ADSRSettings*, seconds_t, seconds_t, seconds_t, seconds_t);
void adsr_settings_initdefault(ADSRSettings*);
float adsr_settings_attack(ADSRSettings*);
void adsr_settings_setattack(ADSRSettings*, seconds_t);
float adsr_settings_decay(ADSRSettings*);
void adsr_settings_setdecay(ADSRSettings*, seconds_t);
float adsr_settings_sustain(ADSRSettings*);
void adsr_settings_setsustain(ADSRSettings*, percent_t);
float adsr_settings_release(ADSRSettings*);
void adsr_settings_setrelease(ADSRSettings*, seconds_t);
float adsr_settings_fastrelease(ADSRSettings*);
void adsr_settings_setfastrelease(ADSRSettings*, seconds_t);

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
	amp_t value;
	amp_t step;	
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
