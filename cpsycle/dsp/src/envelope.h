// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_DSP_ENVELOPE_H)
#define PSY_DSP_ENVELOPE_H

#include "dsptypes.h"
#include <list.h>

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

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue,
	psy_dsp_amp_t maxvalue);

typedef struct psy_dsp_EnvelopeSettings {
	psy_List* points;
} psy_dsp_EnvelopeSettings;

void psy_dsp_envelopesettings_init(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_dispose(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_addpoint(psy_dsp_EnvelopeSettings*,
	psy_dsp_EnvelopePoint);

typedef struct psy_dsp_Envelope {
	int rsvd;
	psy_dsp_EnvelopeSettings settings;	
	unsigned int samplerate;
	int stage;
	int sustainstage;
	psy_List* currstage;
	psy_dsp_amp_t startpeak;
	psy_dsp_amp_t value;
	psy_dsp_amp_t step;
	int tickcount;
	psy_dsp_seconds_t nexttime; // unit frames
	int susdone;
} psy_dsp_Envelope;

void psy_dsp_envelope_init(psy_dsp_Envelope*);
void psy_dsp_envelope_dispose(psy_dsp_Envelope*);
void psy_dsp_envelope_reset(psy_dsp_Envelope*);
void psy_dsp_envelope_setsamplerate(psy_dsp_Envelope*, unsigned int samplerate);
psy_dsp_amp_t psy_dsp_envelope_tick(psy_dsp_Envelope*);
void psy_dsp_envelope_start(psy_dsp_Envelope*);
void psy_dsp_envelope_release(psy_dsp_Envelope*);

#endif
