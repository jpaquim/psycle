// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pattern.h"

#include <list.h>

#include <stdlib.h>
#include <string.h>

int patterneditposition_equal(PatternEditPosition* lhs,
	PatternEditPosition* rhs)
{
	return 
		rhs->column == lhs->column &&
		rhs->digit == lhs->digit &&
		rhs->track == lhs->track &&
		rhs->offset == lhs->offset &&
		rhs->pattern == lhs->pattern;
}

void patternentry_init(psy_audio_PatternEntry* self)
{
	psy_audio_PatternEvent first;

	memset(self, 0, sizeof(psy_audio_PatternEntry));
	patternevent_clear(&first);
	patternentry_addevent(self,	&first);
}

void patternentry_init_all(psy_audio_PatternEntry* self,
	const psy_audio_PatternEvent* event,
	psy_dsp_beat_t offset,
	psy_dsp_beat_t delta,
	psy_dsp_beat_t bpm,
	uintptr_t track)	
{
	self->events = 0;
	patternentry_addevent(self,	event);	
	self->offset = offset;
	self->delta = delta;
	self->bpm = bpm;
	self->track = track;
}

void patternentry_dispose(psy_audio_PatternEntry* self)
{
	psy_List* p;

	for (p = self->events; p != 0; p = p->next) {
		free(p->entry);
	}
	psy_list_free(self->events);
	self->events = 0;
}

psy_audio_PatternEntry* patternentry_alloc(void)
{
	return (psy_audio_PatternEntry*) malloc(sizeof(psy_audio_PatternEntry));
}

psy_audio_PatternEntry* patternentry_allocinit(void)
{
	psy_audio_PatternEntry* rv;
	
	rv = patternentry_alloc();
	if (rv) {
		patternentry_init(rv);
	}
	return rv;
}

psy_audio_PatternEntry* patternentry_clone(psy_audio_PatternEntry* entry)
{
	psy_audio_PatternEntry* rv;
	if (entry) {
		psy_List* p;
		rv = patternentry_alloc();
		rv->bpm = entry->bpm;
		rv->delta = entry->delta;
		rv->offset = entry->offset;
		rv->track = entry->track;
		rv->events = 0;
		for (p = entry->events; p != 0; p = p->next) {
			psy_audio_PatternEvent* copy;

			copy = (psy_audio_PatternEvent*)
				malloc(sizeof(psy_audio_PatternEvent));
			*copy = *((psy_audio_PatternEvent*)p->entry);
			psy_list_append(&rv->events, copy);
		}		
	} else {
		rv = 0;
	}
	return rv;
}

psy_audio_PatternEvent* patternentry_front(psy_audio_PatternEntry* self)
{	
	return (psy_audio_PatternEvent*)(self->events->entry);
}

const psy_audio_PatternEvent* patternentry_front_const(
	const psy_audio_PatternEntry* self)
{
	return (const psy_audio_PatternEvent*)(self->events->entry);
}

void patternentry_addevent(psy_audio_PatternEntry* self,
	const psy_audio_PatternEvent* event)
{
	psy_audio_PatternEvent* copy;

	copy = (psy_audio_PatternEvent*)malloc(sizeof(psy_audio_PatternEvent));
	*copy = *event;
	psy_list_append(&self->events, copy);
}


void pattern_init(psy_audio_Pattern* self)
{
	self->events = 0;
	self->length = 16;
	self->label = strdup("Untitled");	
	self->opcount = 0;
	self->maxsongtracks = 0;
}

void pattern_dispose(psy_audio_Pattern* self)
{
	PatternNode* p;	
	
	for (p = self->events; p != 0; p = p->next) {
		patternentry_dispose((psy_audio_PatternEntry*) p->entry);
		free(p->entry);		
	}
	psy_list_free(self->events);
	self->events = 0;
	free(self->label);
	self->label = 0;
}

void pattern_copy(psy_audio_Pattern* self, psy_audio_Pattern* src)
{	
	PatternNode* p;
	int opcount;

	opcount = self->opcount;
	pattern_dispose(self);
	pattern_init(self);	
	for (p = src->events; p != 0; p = p->next) {
		psy_audio_PatternEntry* srcentry;
		psy_audio_PatternEntry* entry;

		srcentry = (psy_audio_PatternEntry*) p->entry;		
		entry = patternentry_clone(srcentry);		
		psy_list_append(&self->events, entry);
	}
	self->length = src->length;
	free(self->label);
	self->label = strdup(src->label);
	self->opcount = opcount + 1;
}

psy_audio_Pattern* pattern_alloc(void)
{
	return (psy_audio_Pattern*) malloc(sizeof(psy_audio_Pattern));
}

psy_audio_Pattern* pattern_allocinit(void)
{
	psy_audio_Pattern* rv;

	rv = pattern_alloc();
	if (rv) {
		pattern_init(rv);
	}
	return rv;
}

psy_audio_Pattern* pattern_clone(psy_audio_Pattern* self)
{	
	psy_audio_Pattern* rv;	

	rv = pattern_allocinit();
	if (rv) {
		pattern_copy(rv, self);
	}
	return rv;
}

void pattern_remove(psy_audio_Pattern* self, PatternNode* node)
{
	if (node) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)(node->entry);
		psy_list_remove(&self->events, node);
		patternentry_dispose(entry);
		free(entry);
		++self->opcount;
	}
}

PatternNode* pattern_insert(psy_audio_Pattern* self, PatternNode* prev,
	int track, psy_dsp_beat_t offset, const psy_audio_PatternEvent* event)
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

void pattern_setevent(psy_audio_Pattern* self, PatternNode* node,
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

psy_audio_PatternEvent pattern_event(psy_audio_Pattern* self, PatternNode* node)
{
	if (node) {
		return *patternentry_front(((psy_audio_PatternEntry*)node->entry));
	} else {
		psy_audio_PatternEvent empty;

		patternevent_clear(&empty);
		return empty;
	}	
}

PatternNode* pattern_greaterequal(psy_audio_Pattern* self, psy_dsp_beat_t offset)
{
	PatternNode* p;		
	p = self->events;
	while (p != 0) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;
		if (entry->offset >= offset) {			
			break;
		}		
		p = p->next;
	}	
	return p;
}

PatternNode* pattern_findnode(psy_audio_Pattern* pattern, unsigned int track, float offset, 
	psy_dsp_beat_t bpl, PatternNode** prev)
{		
	unsigned int currsubline = 0;
	unsigned int subline = 0;
	int first = 1;
	PatternNode* node = pattern_greaterequal(pattern, offset);	
	if (node) {
		*prev = node->prev;
	} else {
		*prev = pattern_last(pattern);
	}	
	while (node) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl || entry->track > track) {
			node = 0;
			break;
		}		
		if (entry->track == track) {
			break;
		}				
		*prev = node;		
		node = node->next;
		first = 0;
	}
	return node;
}

PatternNode* pattern_last(psy_audio_Pattern* self)
{	
	return self->events ? self->events->tail : 0;
}

void pattern_setlabel(psy_audio_Pattern* self, const char* text)
{
	if (self->label) {
		free(self->label);
		self->label = _strdup(text);
	}
	++self->opcount;
}

void pattern_setlength(psy_audio_Pattern* self, psy_dsp_beat_t length)
{
	self->length = length;
	++self->opcount;
}

psy_dsp_beat_t pattern_length(psy_audio_Pattern* self)
{
	return self->length;
}

int pattern_empty(psy_audio_Pattern* self)
{
	return self->events == 0;
}

unsigned int pattern_opcount(psy_audio_Pattern* self)
{
	return self->opcount;
}

void pattern_scale(psy_audio_Pattern* self, float factor)
{
	PatternNode* p;	
	
	for (p = self->events; p != 0; p = p->next) {		
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;

		entry->offset *= factor;
	}
}

void pattern_blockremove(psy_audio_Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end)
{	
	PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);	
	while (p != 0) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*) p->entry;
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				pattern_remove(self, p);
			}
		} else {
			break;
		}
		p = q;
	}
}

void pattern_blocktranspose(psy_audio_Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end, int offset)
{	
	PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);
	while (p != 0) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*) p->entry;
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

void pattern_changemachine(psy_audio_Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end, int machine)
{
		PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);
	while (p != 0) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*) p->entry;
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

void pattern_changeinstrument(psy_audio_Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end, int instrument)
{
	PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);
	while (p != 0) {			
		psy_audio_PatternEntry* entry;
		q = p->next;

		entry = (psy_audio_PatternEntry*) p->entry;
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

void pattern_setmaxsongtracks(psy_audio_Pattern* self, uintptr_t num)
{
	self->maxsongtracks = num;
}

uintptr_t pattern_maxsongtracks(psy_audio_Pattern* self)
{
	return self->maxsongtracks;
}
