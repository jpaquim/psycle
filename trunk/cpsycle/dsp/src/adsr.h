// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_ADSR_H)
#define DSP_ADSR_H


typedef struct {	
	float attack;
	float decay;
	float sustain;
	float release;
} ADSREnvelopeSettings;

void adsr_envelopesettings_init(ADSREnvelopeSettings*, float, float, float, float);
void adsr_envelopesettings_initdefault(ADSREnvelopeSettings*);

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
	ADSREnvelopeSettings settings;
	EnvelopeStage stage;	
} ADSR;

void adsr_init(ADSR*);
void adsr_tick(ADSR*);
void adsr_start(ADSR*);
void adsr_release(ADSR*);

#endif