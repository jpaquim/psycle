// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_ADSR_H)
#define DSP_ADSR_H


typedef struct {
	float value;
	float time;
	float minvalue;
	float maxvalue;
	float mintime;
	float maxtime;	
} EnvelopePoint;

void envelopepoint_init(EnvelopePoint*, float time, float value, float mintime,
	float maxtime, float minvalue, float maxvalue);

typedef struct {	
	float attack;
	float decay;
	float sustain;
	float release;
	float fastrelease;
} ADSRSettings;

void adsr_settings_init(ADSRSettings*, float, float, float, float);
void adsr_settings_initdefault(ADSRSettings*);
float adsr_settings_attack(ADSRSettings*);
void adsr_settings_setattack(ADSRSettings*, float);
float adsr_settings_decay(ADSRSettings*);
void adsr_settings_setdecay(ADSRSettings*, float);
float adsr_settings_sustain(ADSRSettings*);
void adsr_settings_setsustain(ADSRSettings*, float);
float adsr_settings_release(ADSRSettings*);
void adsr_settings_setrelease(ADSRSettings*, float);
float adsr_settings_fastrelease(ADSRSettings*);
void adsr_settings_setfastrelease(ADSRSettings*, float);

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
	float value;
	float step;	
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