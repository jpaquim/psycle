/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "envelope.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static const psy_dsp_seconds_t defaultfastrelease = 0.003f;

/* envelope point */
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
	rv.maxtime = (psy_dsp_amp_t)65535.f;
	rv.minvalue = (psy_dsp_amp_t)0.f;
	rv.maxvalue = (psy_dsp_amp_t)1.f;

	return rv;
}

psy_dsp_EnvelopePoint psy_dsp_envelopepoint_make_start(void)
{
	psy_dsp_EnvelopePoint rv;

	rv.time = (psy_dsp_seconds_t)0.f;
	rv.value = (psy_dsp_amp_t)0.f;
	rv.mintime = (psy_dsp_seconds_t)0.f;
	rv.maxtime = (psy_dsp_seconds_t)0.f;
	rv.minvalue = (psy_dsp_amp_t)0.f;
	rv.maxvalue = (psy_dsp_amp_t)0.f;

	return rv;
}

/* envelope settings */
void psy_dsp_envelope_init(psy_dsp_Envelope* self)
{
	assert(self);

	self->enabled = FALSE;
	self->carry = FALSE;	
	self->sustainbegin = psy_INDEX_INVALID;
	self->sustainend = psy_INDEX_INVALID;
	self->loopstart = psy_INDEX_INVALID;
	self->loopend = psy_INDEX_INVALID;
	self->timemode = psy_dsp_ENVELOPETIME_SECONDS;	
	self->str = NULL;
	self->points = NULL;
}

/* adsr envelope settings */
void psy_dsp_envelope_init_adsr(psy_dsp_Envelope* self)
{
	assert(self);

	psy_dsp_envelope_init(self);
	/* start attack */
	psy_dsp_envelope_append(self, psy_dsp_envelopepoint_make_start());
	/* start decay */
	psy_dsp_envelope_append(self,
		psy_dsp_envelopepoint_make_all(0.005f, 1.f, 0.f, 65535.f, 1.f, 1.f));
	/* start release */
	psy_dsp_envelope_append(self,
		psy_dsp_envelopepoint_make_all(0.01f, 1.f, 0.f, 65535.f, 0.f, 1.f));
	/* end release */
	psy_dsp_envelope_append(self,
		psy_dsp_envelopepoint_make_all(0.015f, 0.f, 0.f, 65535.f, 0.f, 0.f));
	psy_dsp_envelope_setsustainbegin(self, 2);
	psy_dsp_envelope_setsustainend(self, 2);	
}

void psy_dsp_envelope_init_dispose(psy_dsp_Envelope* self)
{
	assert(self);

	psy_list_deallocate(&self->points, (psy_fp_disposefunc)NULL);	
	free(self->str);
	self->str = NULL;
}

void psy_dsp_envelope_init_copy(psy_dsp_Envelope* self,
	const psy_dsp_Envelope* source)
{
	const psy_List* pts_src;

	assert(self);

	psy_dsp_envelope_init_dispose(self);
	for (pts_src = source->points; pts_src != NULL; psy_list_next_const(&pts_src)) {
		const psy_dsp_EnvelopePoint* pt_src;

		pt_src = (psy_dsp_EnvelopePoint*)psy_list_entry_const(pts_src);
		psy_dsp_envelope_append(self, psy_dsp_envelopepoint_make_all(
			pt_src->time, pt_src->value,
			pt_src->mintime, pt_src->maxtime,
			pt_src->minvalue, pt_src->maxvalue));
	}
	self->enabled = source->enabled;
	self->carry = source->carry;	
	self->sustainbegin = source->sustainbegin;
	self->sustainend = source->sustainend;
	self->loopstart = source->loopstart;
	self->loopend = source->loopend;
	self->timemode = source->timemode;
	self->str = NULL; /* don't copy tostring cache */
}

void psy_dsp_envelope_append(psy_dsp_Envelope* self,
	psy_dsp_EnvelopePoint point)
{
	psy_dsp_EnvelopePoint* newpoint;

	assert(self);

	newpoint = (psy_dsp_EnvelopePoint*)malloc(sizeof(psy_dsp_EnvelopePoint));
	if (newpoint) {		
		*newpoint = point;
		psy_list_append(&self->points, newpoint);
	}
}

void psy_dsp_envelope_delete(psy_dsp_Envelope* self,
	uintptr_t pointindex)
{	
	psy_List* node;

	assert(self);

	node = psy_list_at(self->points, pointindex);
	if (node) {
		free(psy_list_entry(node));
		psy_list_remove(&self->points, node);		
	}
}

void psy_dsp_envelope_clear(psy_dsp_Envelope* self)
{
	assert(self);

	psy_list_deallocate(&self->points, (psy_fp_disposefunc)NULL);
	self->sustainbegin = psy_INDEX_INVALID;
	self->sustainend = psy_INDEX_INVALID;
	self->loopstart = psy_INDEX_INVALID;
	self->loopend = psy_INDEX_INVALID;
	free(self->str);
	self->str = NULL;
}

psy_dsp_EnvelopePoint psy_dsp_envelope_at(const psy_dsp_Envelope* self,
	uintptr_t pointindex)
{
	assert(self);

	if (self->points) {
		const psy_List* node;

		node = psy_list_at_const(self->points, pointindex);
		if (node) {			
			return *((psy_dsp_EnvelopePoint*)psy_list_entry_const(node));
		}
	}
	return psy_dsp_envelopepoint_make(-1.f, 0.f);
}

void psy_dsp_envelope_settimeandvalue(psy_dsp_Envelope* self,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval)
{
	assert(self);

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

void psy_dsp_envelope_settime(psy_dsp_Envelope* self,
	uintptr_t pointindex, psy_dsp_seconds_t pointtime)
{
	assert(self);

	if (self->points) {
		psy_List* node;

		node = psy_list_at(self->points, pointindex);
		if (node) {
			psy_dsp_EnvelopePoint* pt;
			psy_dsp_seconds_t shift;

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(node);
			shift = pointtime - pt->time;
			pt->time = pointtime;			
			psy_list_next(&node);
			for (; node != NULL; psy_list_next(&node)) {
				pt = (psy_dsp_EnvelopePoint*)psy_list_entry(node);
				pt->time += shift;
			}
		}
	}
}


void psy_dsp_envelope_setvalue(psy_dsp_Envelope* self,
	uintptr_t pointindex, psy_dsp_amp_t pointval)
{
	assert(self);

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

const char* psy_dsp_envelope_tostring(const psy_dsp_Envelope* self)
{	
	char valuestr[20];
	char* values;
	psy_List* p;

	assert(self);

	free(((psy_dsp_Envelope*)self)->str);
	((psy_dsp_Envelope*)self)->str = NULL;
	
	if (!self->points) {
		return NULL;
	}
	psy_snprintf(valuestr, 20, "%i ", (int)self->sustainbegin);
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
	psy_strreset(&((psy_dsp_Envelope*)self)->str, values);
	free(values);
	return self->str;
}

void psy_dsp_envelope_setadsr(psy_dsp_Envelope* self,
	psy_dsp_seconds_t attack,
	psy_dsp_seconds_t decay,
	psy_dsp_amp_t sustain,
	psy_dsp_seconds_t release)
{
	psy_dsp_envelope_clear(self);	
	/* start attack */
	psy_dsp_envelope_append(self, psy_dsp_envelopepoint_make_start());
	/* start decay */
	psy_dsp_envelope_append(self,
		psy_dsp_envelopepoint_make_all(attack, 1.f, 0.f, 65535.f, 1.f, 1.f));
	/* start release */
	psy_dsp_envelope_append(self,
		psy_dsp_envelopepoint_make_all(attack + decay, sustain, 0.f, 65535.f, 0.f, 1.f));
	/* end release */
	psy_dsp_envelope_append(self,
		psy_dsp_envelopepoint_make_all(attack + decay + release, 0.f, 0.f, 65535.f, 0.f, 0.f));
	psy_dsp_envelope_setsustainbegin(self, 2);
	psy_dsp_envelope_setsustainend(self, 2);
}

/* psy_dsp_EnvelopeController */
/* prototypes */
static void psy_dsp_envelope_startstage(psy_dsp_EnvelopeController*);
static psy_dsp_amp_t psy_dsp_envelope_stagevalue(psy_dsp_EnvelopeController*);
/* implementation */
void psy_dsp_envelopecontroller_init(psy_dsp_EnvelopeController* self)
{
	psy_dsp_envelope_init(&self->settings);	
	psy_dsp_envelopecontroller_reset(self);
}

void psy_dsp_envelopecontroller_init_envelope(psy_dsp_EnvelopeController* self,
	const psy_dsp_Envelope* envelope)
{
	psy_dsp_envelope_init(&self->settings);
	psy_dsp_envelopecontroller_reset(self);
	psy_dsp_envelopecontroller_set_settings(self, envelope);
}

void psy_dsp_envelopecontroller_init_adsr(psy_dsp_EnvelopeController* self)
{	
	psy_dsp_envelope_init_adsr(&self->settings);
	psy_dsp_envelopecontroller_reset(self);	
}

void psy_dsp_envelopecontroller_dispose(psy_dsp_EnvelopeController* self)
{	
	psy_dsp_envelope_init_dispose(&self->settings);	
}

void psy_dsp_envelopecontroller_reset(psy_dsp_EnvelopeController* self)
{
	self->samplerate = 44100;
	self->bpm = 125;
	self->tpb = 24;
	self->value = 0.f;
	self->step = 0;
	self->nexttime = 0;
	self->startpeak = 0;
	self->samplecount = 0;
	self->susdone = FALSE;	
	self->currstage = NULL;	
	self->susbeginstage = NULL;
	self->susendstage = NULL;
	self->fastrelease = FALSE;		
}

void psy_dsp_envelopecontroller_set_settings(psy_dsp_EnvelopeController* self,
	const psy_dsp_Envelope* settings)
{
	psy_dsp_envelope_init_dispose(&self->settings);	
	psy_dsp_envelope_init_copy(&self->settings, settings);	
	self->susbeginstage = psy_list_at(self->settings.points,
		self->settings.sustainbegin);
	self->susendstage = psy_list_at(self->settings.points,
		self->settings.sustainend);
}

void psy_dsp_envelopecontroller_settimeandvalue(psy_dsp_EnvelopeController* self, uintptr_t pointindex,
	psy_dsp_seconds_t pointtime, psy_dsp_amp_t pointval)
{
	psy_dsp_envelope_settimeandvalue(&self->settings,
		pointindex, pointtime, pointval);
}

void psy_dsp_envelopecontroller_setvalue(psy_dsp_EnvelopeController* self, uintptr_t pointindex,
	psy_dsp_amp_t pointval)
{
	psy_dsp_envelope_setvalue(&self->settings,
		pointindex, pointval);
}

psy_dsp_EnvelopePoint psy_dsp_envelopecontroller_at(const
	psy_dsp_EnvelopeController* self, uintptr_t pointindex)
{
	return psy_dsp_envelope_at(&self->settings, pointindex);
}

psy_List* psy_dsp_envelopecontroller_begin(psy_dsp_EnvelopeController* self)
{
	return self->settings.points;
}

void psy_dsp_envelopecontroller_setsamplerate(psy_dsp_EnvelopeController* self,
	psy_dsp_big_hz_t samplerate)
{
	self->samplerate = samplerate;
}

void psy_dsp_envelopecontroller_updatespeed(psy_dsp_EnvelopeController* self, uintptr_t tpb, double bpm)
{
	self->tpb = (int)tpb;
	self->bpm = bpm;
}

psy_dsp_amp_t psy_dsp_envelopecontroller_tick(psy_dsp_EnvelopeController* self)
{
	if (!psy_dsp_envelopecontroller_playing(self)) {
		self->fastrelease = FALSE;
		return 0.f;
	}

	if (self->currstage == self->susbeginstage && !self->susdone) {
		return self->value;
	}
	if (self->nexttime == self->samplecount) {
		if (self->currstage->next == NULL ||
				(!self->susdone && self->currstage->next == self->susbeginstage)) {
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

psy_dsp_amp_t psy_dsp_envelopecontroller_tick_ps1(psy_dsp_EnvelopeController* self)
{
	psy_dsp_EnvelopePoint* pt;

	if (!psy_dsp_envelopecontroller_playing(self)) {
		self->fastrelease = FALSE;
		return 0.f;
	}

	if (self->currstage == self->susbeginstage && !self->susdone) {
		return self->value;
	}
	if (self->currstage && self->currstage->next) {
		pt = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage->next);
	} else {
		return 0.f;
	}
	if ((self->step == 0) || (self->step > 0 && self->value > pt->value) || 
		(self->step < 0 && self->value < pt->value)) {
		if (self->currstage->next == NULL ||
			(!self->susdone && self->currstage->next == self->susbeginstage)) {
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

void psy_dsp_envelopecontroller_start(psy_dsp_EnvelopeController* self)
{	
	if (self->settings.points) {
		self->currstage = self->settings.points;
		self->susbeginstage = psy_list_at(self->settings.points,
			self->settings.sustainbegin);
		self->susendstage = psy_list_at(self->settings.points,
			self->settings.sustainend);
		self->value = self->startpeak;
		self->susdone = FALSE;
		self->step = 0;
		psy_dsp_envelope_startstage(self);
	}
}

void psy_dsp_envelopecontroller_stop(psy_dsp_EnvelopeController* self)
{
	if (self->settings.points) {
		self->currstage = NULL;		
		self->value = self->startpeak;
		self->susdone = FALSE;		
	}
}

void psy_dsp_envelopecontroller_release(psy_dsp_EnvelopeController* self)
{
	if (psy_dsp_envelopecontroller_playing(self)) {
		self->susdone = TRUE;
		if (self->currstage != self->susbeginstage) {
			self->currstage = psy_list_last(self->settings.points);
		}	
		psy_dsp_envelope_startstage(self);
	}
}

void psy_dsp_envelopecontroller_fastrelease(psy_dsp_EnvelopeController* self)
{		
	if (psy_dsp_envelopecontroller_playing(self)) {
		self->susdone = TRUE;
		if (self->currstage != self->susbeginstage) {
			self->currstage = psy_list_last(self->settings.points);
		}
		if (self->currstage) {
			psy_dsp_EnvelopePoint* pt;

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage);
			self->nexttime = (uintptr_t)(defaultfastrelease * self->samplerate);
			self->samplecount = 0;
			self->step = (self->value) / self->nexttime;
			self->fastrelease = TRUE;
		}
	}
}

void psy_dsp_envelope_startstage(psy_dsp_EnvelopeController* self)
{
	if (self->currstage) {		
		psy_dsp_EnvelopePoint* pt;
		float prevtime;
		
		if (self->currstage->prev) {
			psy_dsp_EnvelopePoint* pt_prev;

			pt_prev = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage->prev);
			prevtime = pt_prev->time;
		} else {
			prevtime = 0;
		}
		pt = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage);
		if (self->settings.timemode == psy_dsp_ENVELOPETIME_TICK) {
			psy_dsp_big_hz_t srdeviation;

			srdeviation = (self->samplerate * 60) / (self->bpm * self->tpb);
			self->nexttime = (uintptr_t)((pt->time - prevtime) * srdeviation);
		} else {
			self->nexttime = (uintptr_t)((pt->time - prevtime) * self->samplerate);
		}
		self->samplecount = 0; 
		if (self->nexttime > 0) {
			self->step = (pt->value - self->value) / self->nexttime;
		} else {
			self->step = 0;
		}
	}
}

psy_dsp_amp_t psy_dsp_envelope_stagevalue(psy_dsp_EnvelopeController* self)
{
	if (self->currstage) {
		psy_dsp_EnvelopePoint* pt;

		pt = (psy_dsp_EnvelopePoint*)psy_list_entry(self->currstage);
		return pt->value;
	}
	return 0.f;
}
