// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "envelope.h"
#include <stdlib.h>

void calcstage(psy_dsp_Envelope* self,
	psy_dsp_amp_t peakstart, psy_dsp_amp_t peakend,
	psy_dsp_seconds_t time);

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
	self->points = 0;
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

// envelope
void psy_dsp_envelope_init(psy_dsp_Envelope* self)
{
	psy_dsp_envelopesettings_init(&self->settings);	
	self->samplerate = 44100;
	self->stage = 0;
	self->sustainstage = 0;
	self->currstage = 0;
	self->value = 0;
	self->step = 0;
	self->nexttime = 0;
	self->startpeak = 0;
	self->tickcount = 0;
    self->susdone = 0;
	psy_dsp_envelope_reset(self);
}

void psy_dsp_envelope_dispose(psy_dsp_Envelope* self)
{
	psy_dsp_envelopesettings_dispose(&self->settings);
}

void psy_dsp_envelope_reset(psy_dsp_Envelope* self)
{
	psy_dsp_EnvelopePoint* pt;

	self->susdone = 0;
	self->value = self->startpeak;
	self->currstage = self->settings.points;
	if (self->currstage) {
		pt = (psy_dsp_EnvelopePoint*) self->currstage->entry;
		calcstage(self, self->startpeak, pt->value, pt->time);
	}	
}

void psy_dsp_envelope_setsamplerate(psy_dsp_Envelope* self,
	uintptr_t samplerate)
{
	self->samplerate = samplerate;
}

psy_dsp_amp_t psy_dsp_envelope_tick(psy_dsp_Envelope* self)
{	
	psy_dsp_amp_t rv;

    if (self->currstage && self->nexttime == self->tickcount) {
		psy_dsp_EnvelopePoint* pt0;
		psy_dsp_EnvelopePoint* pt1;

		pt0 = (psy_dsp_EnvelopePoint*) self->currstage->entry;
		++self->stage;
		self->currstage = self->currstage->next;
		if (!self->currstage || (!self->susdone && 
			self->stage == self->sustainstage)) {
			self->value = pt0->value;
			self->step = 0;
			rv = self->value;
			return rv;
		}
		pt1 = (psy_dsp_EnvelopePoint*) self->currstage->entry;      
		calcstage(self, pt0->value, pt1->value, pt1->time);
		rv = self->value;
		self->value += self->step;      
		++self->tickcount;
	} else {
		rv = 0;
	}
	return rv;
}

void psy_dsp_envelope_start(psy_dsp_Envelope* self)
{
	psy_dsp_envelope_reset(self);
}

void psy_dsp_envelope_release(psy_dsp_Envelope* self)
{
	if (self->currstage) {    
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*) self->currstage->entry;
		self->susdone = 1;
		if (self->stage != self->sustainstage) {
			pt = (psy_dsp_EnvelopePoint*) self->currstage->tail->entry;
		}    
		calcstage(self, self->value, pt->value, pt->time);
	}
}

void calcstage(psy_dsp_Envelope* self,
	psy_dsp_amp_t peakstart, psy_dsp_amp_t peakend,
	psy_dsp_seconds_t time)
{
	self->nexttime = time * self->samplerate;
	self->value = (float) peakstart;
	self->tickcount = 0;
	self->step = (peakend - peakstart) / self->nexttime;
}
