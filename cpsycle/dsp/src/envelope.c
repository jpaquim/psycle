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

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make_all(
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

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make(
	psy_dsp_seconds_t time,
	psy_dsp_amp_t value)
{
	psy_dsp_EnvelopePoint rv;

	rv.time = time;
	rv.value = value;
	rv.mintime = (psy_dsp_amp_t)0.f;
	rv.maxtime = (psy_dsp_amp_t)0.f;
	rv.minvalue = (psy_dsp_amp_t)0.f;
	rv.maxvalue = (psy_dsp_amp_t)1.f;

	return rv;
}

// envelope settings
void psy_dsp_envelopesettings_init(psy_dsp_EnvelopeSettings* self)
{
	self->points = NULL;
	self->sustainstage = 0;
	self->time_unit = psy_dsp_ENVELOPETIME_SECONDS;
	self->str = NULL;
}

// adsr envelope settings
void psy_dsp_envelopesettings_init_adsr(psy_dsp_EnvelopeSettings* self)
{
	psy_dsp_envelopesettings_init(self);
	// start attack
	psy_dsp_envelopesettings_append(self,
		psy_dsp_envelopepoint_make(0.f, 0.f));
	// start decay
	psy_dsp_envelopesettings_append(self,
		psy_dsp_envelopepoint_make_all(0.005f, 1.f, 0.f, 5.f, 1.f, 1.f));
	// start release
	psy_dsp_envelopesettings_append(self,
		psy_dsp_envelopepoint_make_all(0.01f, 1.f, 0.f, 5.f, 0.f, 1.f));
	// end release
	psy_dsp_envelopesettings_append(self,
		psy_dsp_envelopepoint_make_all(0.015f, 0.f, 0.f, 5.f, 0.f, 0.f));
	psy_dsp_envelopesettings_setsustainstage(self, 2);
}

void psy_dsp_envelopesettings_dispose(psy_dsp_EnvelopeSettings* self)
{
	psy_list_free(self->points);
	self->points = NULL;
	free(self->str);
	self->str = NULL;
}

void psy_dsp_envelopesettings_copy(psy_dsp_EnvelopeSettings* self,
	const psy_dsp_EnvelopeSettings* source)
{
	const psy_List* pts_src;
	psy_dsp_envelopesettings_dispose(self);

	for (pts_src = source->points; pts_src != NULL; psy_list_next_const(&pts_src)) {
		const psy_dsp_EnvelopePoint* pt_src;

		pt_src = (psy_dsp_EnvelopePoint*)psy_list_entry_const(pts_src);
		psy_dsp_envelopesettings_append(self, psy_dsp_envelopepoint_make_all(
				pt_src->time, pt_src->value,
				pt_src->mintime, pt_src->maxtime,
				pt_src->minvalue, pt_src->maxvalue));
	}
	self->sustainstage = source->sustainstage;
}

void psy_dsp_envelopesettings_append(psy_dsp_EnvelopeSettings* self,
	psy_dsp_EnvelopePoint point)
{
	psy_dsp_EnvelopePoint* newpoint;

	newpoint = (psy_dsp_EnvelopePoint*)malloc(sizeof(psy_dsp_EnvelopePoint));
	if (newpoint) {		
		*newpoint = point;
		psy_list_append(&self->points, newpoint);
	}
}

psy_dsp_EnvelopePoint psy_dsp_envelopesettings_at(const psy_dsp_EnvelopeSettings* self,
	uintptr_t pointindex)
{
	if (self->points) {
		const psy_List* node;

		node = psy_list_at_const(self->points, pointindex);
		if (node) {			
			return *((psy_dsp_EnvelopePoint*)psy_list_entry_const(node));
		}
	}
	return psy_dsp_envelopepoint_make(0.f, 0.f);
}

void psy_dsp_envelopesettings_settimeandvalue(psy_dsp_EnvelopeSettings* self,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval)
{
	if (self->points) {
		psy_List* node;

		node = psy_list_at(self->points, pointindex);
		if (node) {
			psy_dsp_EnvelopePoint* pt;
			psy_dsp_seconds_t shift;

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(node);
			shift = pointtime - pt->time;
			pt->time = pointtime;
			pt->value = pointval;				
			psy_list_next(&node);
			for (; node != NULL; psy_list_next(&node)) {
				pt = (psy_dsp_EnvelopePoint*)psy_list_entry(node);
				pt->time += shift;
			}
		}
	}
}

void psy_dsp_envelopesettings_setvalue(psy_dsp_EnvelopeSettings* self,
	uintptr_t pointindex, psy_dsp_amp_t pointval)
{
	if (self->points) {
		psy_List* node;

		node = psy_list_at(self->points, pointindex);
		if (node) {
			psy_dsp_EnvelopePoint* pt;			

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(node);
			pt->value = pointval;			
		}
	}
}

void psy_dsp_envelopesettings_setsustainstage(psy_dsp_EnvelopeSettings* self,
	int stage)
{
	self->sustainstage = stage;
}

const char* psy_dsp_envelopesettings_tostring(const psy_dsp_EnvelopeSettings* self)
{	
	char valuestr[20];
	char* values;
	psy_List* p;

	free(((psy_dsp_EnvelopeSettings*)self)->str);
	((psy_dsp_EnvelopeSettings*)self)->str = NULL;
	
	if (!self->points) {
		return NULL;
	}
	psy_snprintf(valuestr, 20, "%i ", (int)self->sustainstage);
	values = strdup(valuestr);
	for (p = self->points; p != NULL; psy_list_next(&p)) {
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)psy_list_entry(p);
		if (p->next) {
			psy_snprintf(valuestr, 20, "%f %f ", (float)pt->time, (float)pt->value);
		} else {
			psy_snprintf(valuestr, 20, "%f %f", (float)pt->time, (float)pt->value);
		}
		values = psy_strcat_realloc(values, valuestr);
	}
	psy_strreset(&((psy_dsp_EnvelopeSettings*)self)->str, values);
	free(values);
	return self->str;
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

void psy_dsp_envelope_init_adsr(psy_dsp_Envelope* self)
{	
	psy_dsp_envelopesettings_init_adsr(&self->settings);
	psy_dsp_envelope_reset(self);	
}

void psy_dsp_envelope_dispose(psy_dsp_Envelope* self)
{	
	psy_dsp_envelopesettings_dispose(&self->settings);	
}

void psy_dsp_envelope_reset(psy_dsp_Envelope* self)
{
	self->samplerate = 44100;	
	self->value = 0.f;
	self->step = 0;
	self->nexttime = 0;
	self->startpeak = 0;
	self->samplecount = 0;
	self->susdone = FALSE;
	self->currstage = NULL;
	self->susstage = NULL;
}

void psy_dsp_envelope_set_settings(psy_dsp_Envelope* self,
	const psy_dsp_EnvelopeSettings* settings)
{
	psy_dsp_envelopesettings_dispose(&self->settings);	
	psy_dsp_envelopesettings_copy(&self->settings, settings);	
	self->susstage = psy_list_at(self->settings.points,
		self->settings.sustainstage);
}

void psy_dsp_envelope_settimeandvalue(psy_dsp_Envelope* self, uintptr_t pointindex,
	psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval)
{
	psy_dsp_envelopesettings_settimeandvalue(&self->settings,
		pointindex, pointtime, pointval);
}

void psy_dsp_envelope_setvalue(psy_dsp_Envelope* self, uintptr_t pointindex,
	psy_dsp_amp_t pointval)
{
	psy_dsp_envelopesettings_setvalue(&self->settings,
		pointindex, pointval);
}

psy_dsp_EnvelopePoint psy_dsp_envelope_at(const psy_dsp_Envelope* self, uintptr_t pointindex)
{
	return psy_dsp_envelopesettings_at(&self->settings, pointindex);
}

psy_List* psy_dsp_envelope_begin(psy_dsp_Envelope* self)
{
	return self->settings.points;
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
			self->settings.sustainstage);
		self->value = self->startpeak;
		self->susdone = FALSE;
		psy_dsp_envelope_startstage(self);
	}
}

void psy_dsp_envelope_stop(psy_dsp_Envelope* self)
{
	if (self->settings.points) {
		self->currstage = NULL;		
		self->value = self->startpeak;
		self->susdone = FALSE;		
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
