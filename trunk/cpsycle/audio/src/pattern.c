// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pattern.h"

#include <list.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

static double hermitecurveinterpolate(int kf0, int kf1, int kf2,
	int kf3, int curposition, int maxposition, double tangmult, bool interpolation);

void psy_audio_patterneditposition_init(psy_audio_PatternEditPosition* self)
{
	self->track = 0;
	self->offset = 0;
	self->line = 0;
	self->column = 0;
	self->digit = 0;
	self->pattern = 0;
}

int psy_audio_patterneditposition_equal(psy_audio_PatternEditPosition* lhs,
	psy_audio_PatternEditPosition* rhs)
{
	return 
		rhs->column == lhs->column &&
		rhs->digit == lhs->digit &&
		rhs->track == lhs->track &&
		rhs->offset == lhs->offset &&
		rhs->pattern == lhs->pattern;
}

static void psy_audio_pattern_init_signals(psy_audio_Pattern*);
static void psy_audio_pattern_dispose_signals(psy_audio_Pattern*);

void psy_audio_pattern_init(psy_audio_Pattern* self)
{
	self->events = 0;
	self->length = 16;
	self->name = strdup("Untitled");	
	self->opcount = 0;
	self->maxsongtracks = 0;
	psy_audio_pattern_init_signals(self);
}

void psy_audio_pattern_init_signals(psy_audio_Pattern* self)
{
	psy_signal_init(&self->signal_namechanged);
	psy_signal_init(&self->signal_lengthchanged);
}

void psy_audio_pattern_dispose(psy_audio_Pattern* self)
{	
	psy_list_deallocate(&self->events,
		(psy_fp_disposefunc)patternentry_dispose);
	free(self->name);
	self->name = NULL;
	psy_audio_pattern_dispose_signals(self);
}

void psy_audio_pattern_dispose_signals(psy_audio_Pattern* self)
{
	psy_signal_dispose(&self->signal_namechanged);
	psy_signal_dispose(&self->signal_lengthchanged);
}

void psy_audio_pattern_copy(psy_audio_Pattern* self, psy_audio_Pattern* src)
{	
	PatternNode* p;
	int opcount;

	opcount = self->opcount;
	psy_audio_pattern_dispose(self);
	psy_audio_pattern_init(self);
	for (p = src->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* srcentry;
		psy_audio_PatternEntry* entry;

		srcentry = (psy_audio_PatternEntry*)psy_list_entry(p);
		entry = patternentry_clone(srcentry);
		psy_list_append(&self->events, entry);
	}
	self->length = src->length;
	free(self->name);
	self->name = strdup(src->name);
	self->opcount = opcount + 1;
}

psy_audio_Pattern* psy_audio_pattern_alloc(void)
{
	return (psy_audio_Pattern*) malloc(sizeof(psy_audio_Pattern));
}

psy_audio_Pattern* psy_audio_pattern_allocinit(void)
{
	psy_audio_Pattern* rv;

	rv = psy_audio_pattern_alloc();
	if (rv) {
		psy_audio_pattern_init(rv);
	}
	return rv;
}

psy_audio_Pattern* psy_audio_pattern_clone(psy_audio_Pattern* self)
{	
	psy_audio_Pattern* rv;	

	rv = psy_audio_pattern_allocinit();
	if (rv) {
		psy_audio_pattern_copy(rv, self);
	}
	return rv;
}

void psy_audio_pattern_remove(psy_audio_Pattern* self, PatternNode* node)
{
	if (node) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)(node->entry);
		psy_list_remove(&self->events, node);
		patternentry_dispose(entry);
		free(entry);
		++self->opcount;		
	}
}

PatternNode* psy_audio_pattern_insert(psy_audio_Pattern* self, PatternNode* prev,
	int track, psy_dsp_big_beat_t offset, const psy_audio_PatternEvent* event)
{
	PatternNode* rv;
	psy_audio_PatternEntry* entry;

	if (event) {
		entry = patternentry_alloc();
		patternentry_init_all(entry, event, offset, 0.f, 0.f, track);
		if (!self->events) {
			rv = self->events = psy_list_create(entry);
		} else {	
			rv = psy_list_insert(&self->events, prev, entry);		
		}	
		++self->opcount;
	}
	return rv;
}

void psy_audio_pattern_setevent(psy_audio_Pattern* self, PatternNode* node,
	const psy_audio_PatternEvent* event)
{
	if (node) {
		psy_audio_PatternEntry* entry;
			
		entry = (psy_audio_PatternEntry*) node->entry;
		if (event) {			
			*patternentry_front(entry) = *event;
		} else {
			psy_audio_PatternEvent empty;

			patternevent_clear(&empty);
			*patternentry_front(entry) = empty;
		}
		++self->opcount;
	}
}

psy_audio_PatternEvent psy_audio_pattern_event(psy_audio_Pattern* self, PatternNode* node)
{
	if (node) {
		return *patternentry_front(((psy_audio_PatternEntry*)node->entry));
	} else {
		psy_audio_PatternEvent empty;

		patternevent_clear(&empty);
		return empty;
	}	
}

PatternNode* psy_audio_pattern_greaterequal(psy_audio_Pattern* self, psy_dsp_big_beat_t offset)
{
	PatternNode* p;

	p = self->events;
	while (p != NULL) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset >= offset) {			
			break;
		}		
		psy_list_next(&p);
	}	
	return p;
}

PatternNode* psy_audio_pattern_findnode(psy_audio_Pattern* pattern, uintptr_t track,
	psy_dsp_big_beat_t offset, psy_dsp_big_beat_t bpl, PatternNode** prev)
{
	PatternNode* node;
	
	node = psy_audio_pattern_greaterequal(pattern, offset);
	if (node) {
		*prev = node->prev;
	} else {
		*prev = psy_audio_pattern_last(pattern);
	}	
	while (node) {
		psy_audio_PatternEntry* entry;
		
		entry = (psy_audio_PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl || entry->track > track) {
			node = NULL;
			break;
		}		
		if (entry->track == track) {
			break;
		}				
		*prev = node;		
		node = node->next;
	}
	return node;
}

PatternNode* psy_audio_pattern_last(psy_audio_Pattern* self)
{	
	return self->events ? self->events->tail : 0;
}

void psy_audio_pattern_setname(psy_audio_Pattern* self, const char* text)
{
	if (self->name) {
		free(self->name);
		self->name = strdup(text);
	}
	++self->opcount;
	psy_signal_emit(&self->signal_namechanged, self, 0);
}

void psy_audio_pattern_setlength(psy_audio_Pattern* self, psy_dsp_big_beat_t length)
{
	self->length = length;
	++self->opcount;
	psy_signal_emit(&self->signal_lengthchanged, self, 0);
}

void psy_audio_pattern_scale(psy_audio_Pattern* self, float factor)
{
	PatternNode* p;	
	
	for (p = self->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)psy_list_entry(p);

		entry->offset *= factor;
	}
}

void psy_audio_pattern_blockremove(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end)
{	
	PatternNode* p;
	PatternNode* q;

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				psy_audio_pattern_remove(self, p);
			}
		} else {
			break;
		}
		p = q;
	}
}

void psy_audio_pattern_blockinterpolatelinear(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end, psy_dsp_big_beat_t bpl)
{
	intptr_t startval;
	intptr_t endval;
	PatternNode* prev;
	PatternNode* node;

	node = psy_audio_pattern_findnode(self, begin.track, begin.line * bpl, bpl, &prev);
	startval = (node)
		? psy_audio_patternevent_tweakvalue(patternentry_front(node->entry))
		: 0;
	node = psy_audio_pattern_findnode(self, begin.track, (end.line - 1) * bpl, bpl, &prev);
	endval = (node)
		? psy_audio_patternevent_tweakvalue(patternentry_front(node->entry))
		: 0;
	psy_audio_pattern_blockinterpolaterange(self, begin, end, bpl, startval, endval);
}

void psy_audio_pattern_blockinterpolaterange(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end,
	psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval)
{
	uintptr_t line;
	float step;
	PatternNode* prev = 0;
	PatternNode* node;

	step = (endval - startval) / (float)(end.line - begin.line - 1);
	for (line = begin.line; line < end.line; ++line) {
		psy_dsp_big_beat_t offset;
		int value;

		offset = line * bpl;
		value = (int)((step * (line - begin.line) + startval));
		node = psy_audio_pattern_findnode(self, begin.track, offset, bpl, &prev);
		if (node) {
			psy_audio_patternevent_settweakvalue(patternentry_front(node->entry), value);
			++self->opcount;
		}
		else {
			psy_audio_PatternEvent ev;

			patternevent_clear(&ev);
			psy_audio_patternevent_settweakvalue(&ev, value);
			prev = psy_audio_pattern_insert(self, prev, begin.track, offset, &ev);
		}
	}
}

void psy_audio_pattern_blockinterpolaterangehermite(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end,
	psy_dsp_big_beat_t bpl, intptr_t startval, intptr_t endval)
{
	uintptr_t line;
	PatternNode* prev = 0;
	PatternNode* node;
	int val0 = 0;
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	int distance;

	val0 = startval;
	val1 = startval;
	val2 = endval;
	val3 = endval;
	distance = end.line - begin.line - 1;
	for (line = begin.line; line < end.line; ++line) {
		psy_dsp_big_beat_t offset;

		offset = line * bpl;
		node = psy_audio_pattern_findnode(self, begin.track, offset, bpl, &prev);
		if (node) {
			double curveval = hermitecurveinterpolate(val0, val1, val2, val3, line - begin.line, distance, 0, TRUE);
			psy_audio_patternevent_settweakvalue(patternentry_front(node->entry), (intptr_t)curveval);
			++self->opcount;
		} else {
			psy_audio_PatternEvent ev;
			double curveval = hermitecurveinterpolate(val0, val1, val2, val3, line - begin.line, distance, 0, TRUE);

			patternevent_clear(&ev);
			psy_audio_patternevent_settweakvalue(&ev, (intptr_t)curveval);
			prev = psy_audio_pattern_insert(self, prev, begin.track, offset, &ev);
		}
	}
}

double hermitecurveinterpolate(int kf0, int kf1, int kf2,
	int kf3, int curposition, int maxposition, double tangmult, bool interpolation)
{
	if (interpolation == TRUE)
	{
		double s = (double)curposition / (double)maxposition;
		double pws3 = pow(s, 3);
		double pws2 = pow(s, 2);
		double pws23 = 3 * pws2;
		double h1 = (2 * pws3) - pws23 + 1;
		double h2 = (-2 * pws3) + pws23;
		double h3 = pws3 - (2 * pws2) + s;
		double h4 = pws3 - pws2;

		double t1 = tangmult * (kf2 - kf0);
		double t2 = tangmult * (kf3 - kf1);

		return (h1 * kf1 + h2 * kf2 + h3 * t1 + h4 * t2);
	}
	else
	{
		return kf1;
	}
}

void psy_audio_pattern_blocktranspose(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end, int offset)
{	
	PatternNode* p;
	PatternNode* q;

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				if (patternentry_front(entry)->note < NOTECOMMANDS_RELEASE) {
					if (patternentry_front(entry)->note + offset < 0) {
						patternentry_front(entry)->note = 0;
					} else {
						patternentry_front(entry)->note += offset;
					}					
					if (patternentry_front(entry)->note >= NOTECOMMANDS_RELEASE) {
						patternentry_front(entry)->note = NOTECOMMANDS_RELEASE - 1;
					}
				}
			}
		} else {
			break;
		}	
		p = q;
	}
	++self->opcount;
}

void psy_audio_pattern_changemachine(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end, int machine)
{
	PatternNode* p;
	PatternNode* q;

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				patternentry_front(entry)->mach = machine;
			}
		} else {
			break;
		}	
		p = q;
	}
}

void psy_audio_pattern_changeinstrument(psy_audio_Pattern* self,
	psy_audio_PatternEditPosition begin,
	psy_audio_PatternEditPosition end, int instrument)
{
	PatternNode* p;
	PatternNode* q;

	p = psy_audio_pattern_greaterequal(self, (psy_dsp_big_beat_t) begin.offset);
	while (p != NULL) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				patternentry_front(entry)->inst = instrument;
			}
		} else {
			break;
		}	
		p = q;
	}
}

void psy_audio_pattern_setmaxsongtracks(psy_audio_Pattern* self, uintptr_t num)
{
	self->maxsongtracks = num;
}

uintptr_t psy_audio_pattern_maxsongtracks(psy_audio_Pattern* self)
{
	return self->maxsongtracks;
}

