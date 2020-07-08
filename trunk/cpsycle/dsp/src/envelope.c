// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "envelope.h"
#include <stdlib.h>

#include "../../detail/trace.h"

// envelope point
void psy_dsp_envelopepoint_init(psy_dsp_EnvelopePoint* self, 
	psy_dsp_seconds_t time, psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime, psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue, psy_dsp_amp_t maxvalue)
{
	self->time = time;
	self->value = value;
	self->mintime = mintime;
	self->maxtime = maxtime;
	self->minvalue = minvalue;
	self->maxvalue = maxvalue;
}

psy_dsp_EnvelopePoint* psy_dsp_envelopepoint_alloc(void)
{
	return (psy_dsp_EnvelopePoint*)malloc(sizeof(psy_dsp_EnvelopePoint));
}

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value,
	psy_dsp_seconds_t mintime,
	psy_dsp_seconds_t maxtime,
	psy_dsp_amp_t minvalue,
	psy_dsp_amp_t maxvalue)
{
	psy_dsp_EnvelopePoint rv;

	rv.time = time;
	rv.value = value;
	rv.mintime = mintime;
	rv.maxtime = maxtime;
	rv.minvalue = minvalue;
	rv.maxvalue = maxvalue;

	return rv;
}

// envelope settings
void psy_dsp_envelopesettings_init(psy_dsp_EnvelopeSettings* self)
{
	self->points = NULL;
}

void psy_dsp_envelopesettings_dispose(psy_dsp_EnvelopeSettings* self)
{
	psy_list_free(self->points);
}

void psy_dsp_envelopesettings_addpoint(psy_dsp_EnvelopeSettings* self,
	psy_dsp_EnvelopePoint point)
{
	psy_dsp_EnvelopePoint* newpoint;

	newpoint = (psy_dsp_EnvelopePoint*) malloc(sizeof(psy_dsp_EnvelopePoint));
	if (newpoint) {		
		*newpoint = point;
		psy_list_append(&self->points, newpoint);
	}
}

// psy_dsp_Envelope
// prototypes
static void psy_dsp_envelope_startstage(psy_dsp_Envelope*);
static psy_dsp_amp_t psy_dsp_envelope_stagevalue(psy_dsp_Envelope*);

// implementation
void psy_dsp_envelope_init(psy_dsp_Envelope* self)
{
	psy_dsp_envelopesettings_init(&self->settings);	
	psy_dsp_envelope_reset(self);
}

void psy_dsp_envelope_dispose(psy_dsp_Envelope* self)
{
	psy_dsp_envelopesettings_dispose(&self->settings);
}

void psy_dsp_envelope_reset(psy_dsp_Envelope* self)
{
	self->samplerate = 44100;
	self->sustainstage = 0;
	self->value = 0.f;
	self->step = 0;
	self->nexttime = 0;
	self->startpeak = 0;
	self->samplecount = 0;
	self->susdone = FALSE;
	self->currstage = NULL;
	self->susstage = NULL;
}

void psy_dsp_envelope_setsamplerate(psy_dsp_Envelope* self,
	uintptr_t samplerate)
{
	self->samplerate = samplerate;
}

psy_dsp_amp_t psy_dsp_envelope_tick(psy_dsp_Envelope* self)
{
	if (!psy_dsp_envelope_playing(self)) {
		return 0.f;
	}
	if (self->currstage == self->susstage && !self->susdone) {
		return self->value;
	}
	if (self->nexttime == self->samplecount) {
		if (self->currstage->next == NULL ||
				(!self->susdone && self->currstage->next == self->susstage)) {						
			self->value = psy_dsp_envelope_stagevalue(self);
			self->step = 0;
			psy_list_next(&self->currstage);			
			return self->value;			
		}
		self->value = psy_dsp_envelope_stagevalue(self);
		psy_list_next(&self->currstage);				
		psy_dsp_envelope_startstage(self);
	}
	self->value += self->step;	
	++self->samplecount;
	return self->value;
}

void psy_dsp_envelope_start(psy_dsp_Envelope* self)
{	
	if (self->settings.points) {
		self->currstage = self->settings.points;
		self->susstage = psy_list_at(self->settings.points,
			self->sustainstage);
		self->value = self->startpeak;
		self->susdone = FALSE;
		psy_dsp_envelope_startstage(self);
	}
}

void psy_dsp_envelope_release(psy_dsp_Envelope* self)
{
	if (psy_dsp_envelope_playing(self)) {
		self->susdone = TRUE;
		if (self->currstage != self->susstage) {
			self->currstage = psy_list_last(self->settings.points);
		}	
		psy_dsp_envelope_startstage(self);
	}
}

void psy_dsp_envelope_startstage(psy_dsp_Envelope* self)
{
	if (self->currstage) {
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage);
		self->nexttime = (uintptr_t)(pt->time * self->samplerate);
		self->samplecount = 0;
		self->step = (pt->value - self->value) / self->nexttime;
	}
}

psy_dsp_amp_t psy_dsp_envelope_stagevalue(psy_dsp_Envelope* self)
{
	if (self->currstage) {
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage);
		return pt->value;
	}
	return 0.f;
}
