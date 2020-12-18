// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_ENVELOPE_H
#define psy_dsp_ENVELOPE_H

#include "dsptypes.h"
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

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

psy_dsp_EnvelopePoint* psy_dsp_envelopepoint_alloc(void);

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make_all(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue,
	psy_dsp_amp_t maxvalue);

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value);

typedef enum {
	psy_dsp_ENVELOPETIME_SECONDS,
	psy_dsp_ENVELOPETIME_FT2FRAMES
} psy_dsp_EnvelopeTime;

typedef struct psy_dsp_EnvelopeSettings {
	psy_List* points;
	int sustainstage;
	psy_dsp_EnvelopeTime time_unit;
	char* str;
} psy_dsp_EnvelopeSettings;

void psy_dsp_envelopesettings_init(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_init_adsr(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_dispose(psy_dsp_EnvelopeSettings*);
void psy_dsp_envelopesettings_copy(psy_dsp_EnvelopeSettings* self,
	const psy_dsp_EnvelopeSettings* source);
/// Appends a new point at the end of the list.
/// note: be sure that the pointtime is the highest of the points
void psy_dsp_envelopesettings_append(psy_dsp_EnvelopeSettings*,
	psy_dsp_EnvelopePoint);
psy_dsp_EnvelopePoint psy_dsp_envelopesettings_at(const psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex);
void psy_dsp_envelopesettings_settimeandvalue(psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval);
void psy_dsp_envelopesettings_setvalue(psy_dsp_EnvelopeSettings*,
	uintptr_t pointindex, psy_dsp_amp_t pointval);
void psy_dsp_envelopesettings_setsustainstage(psy_dsp_EnvelopeSettings*,
	int stage);
const char* psy_dsp_envelopesettings_tostring(const psy_dsp_EnvelopeSettings*);

INLINE bool psy_dsp_envelopesettings_empty(const psy_dsp_EnvelopeSettings* self)
{
	return self->points == NULL;
}

typedef struct psy_dsp_Envelope {
	int rsvd;
	psy_dsp_EnvelopeSettings settings;	
	uintptr_t samplerate;	
	psy_List* currstage;
	psy_List* susstage;
	psy_dsp_amp_t startpeak;
	psy_dsp_amp_t value;
	psy_dsp_amp_t step;	
	uintptr_t samplecount;
	uintptr_t nexttime;
	bool susdone;	
} psy_dsp_Envelope;

void psy_dsp_envelope_init(psy_dsp_Envelope*);
void psy_dsp_envelope_init_adsr(psy_dsp_Envelope*);
void psy_dsp_envelope_dispose(psy_dsp_Envelope*);
void psy_dsp_envelope_reset(psy_dsp_Envelope*);
void psy_dsp_envelope_set_settings(psy_dsp_Envelope*,
	const psy_dsp_EnvelopeSettings*);
void psy_dsp_envelope_settimeandvalue(psy_dsp_Envelope*, uintptr_t pointindex,
	psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval);
void psy_dsp_envelope_setvalue(psy_dsp_Envelope*, uintptr_t pointindex,
	psy_dsp_amp_t pointval);
psy_dsp_EnvelopePoint psy_dsp_envelope_at(const psy_dsp_Envelope*,
	uintptr_t pointindex);
psy_List*  psy_dsp_envelope_begin(psy_dsp_Envelope*);
void psy_dsp_envelope_setsamplerate(psy_dsp_Envelope*, uintptr_t samplerate);
psy_dsp_amp_t psy_dsp_envelope_tick(psy_dsp_Envelope*);
void psy_dsp_envelope_start(psy_dsp_Envelope*);
void psy_dsp_envelope_stop(psy_dsp_Envelope*);
void psy_dsp_envelope_release(psy_dsp_Envelope*);

INLINE bool psy_dsp_envelope_releasing(psy_dsp_Envelope* self)
{
	return (self->settings.points && self->currstage == self->settings.points->tail);
}

INLINE bool psy_dsp_envelope_playing(psy_dsp_Envelope* self)
{	
	return self->currstage != NULL;	
}

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_ENVELOPE_H */
