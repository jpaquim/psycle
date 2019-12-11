// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pattern.h"
#include <stdlib.h>
#include <string.h>

int patterneditposition_equal(PatternEditPosition* lhs,
	PatternEditPosition* rhs)
{
	return 
		rhs->col == lhs->col &&
		rhs->track == lhs->track &&
		rhs->offset == lhs->offset &&
		rhs->pattern == lhs->pattern;
}

PatternEntry* patternentry_clone(PatternEntry* entry)
{
	PatternEntry* rv;
	if (entry) {			
		rv = (PatternEntry*) malloc(sizeof(PatternEntry));
		*rv = *entry;			
	} else {
		rv = 0;
	}
	return rv;
}

void pattern_init(Pattern* self)
{
	self->events = 0;
	self->length = 16;
	self->label = strdup("Untitled");	
	self->opcount = 0;
	self->maxsongtracks = 0;
}

void pattern_dispose(Pattern* self)
{
	PatternNode* p;	
	
	for (p = self->events; p != 0; p = p->next) {		
		free(p->entry);		
	}
	list_free(self->events);
	self->events = 0;
	free(self->label);
	self->label = 0;
}

void pattern_copy(Pattern* self, Pattern* src)
{	
	PatternNode* p;
	int opcount;

	opcount = self->opcount;
	pattern_dispose(self);
	pattern_init(self);	
	for (p = src->events; p != 0; p = p->next) {
		PatternEntry* srcentry;
		PatternEntry* entry;

		srcentry = (PatternEntry*) p->entry;
		entry = (PatternEntry*) malloc(sizeof(PatternEntry));
		memset(entry, 0, sizeof(PatternEntry));
		if (srcentry) {
			*entry = *srcentry;
		} else {
			memset(entry, 0, sizeof(PatternEntry));
		}
		list_append(&self->events, entry);
	}
	self->length = src->length;
	free(self->label);
	self->label = strdup(src->label);
	self->opcount = opcount + 1;
}

Pattern* pattern_alloc(void)
{
	return (Pattern*) malloc(sizeof(Pattern));
}

Pattern* pattern_allocinit(void)
{
	Pattern* rv;

	rv = pattern_alloc();
	if (rv) {
		pattern_init(rv);
	}
	return rv;
}

Pattern* pattern_clone(Pattern* self)
{	
	Pattern* rv;	

	rv = pattern_allocinit();
	if (rv) {
		pattern_copy(rv, self);
	}
	return rv;
}

void pattern_remove(Pattern* self, PatternNode* node)
{
	if (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		list_remove(&self->events, node);
		free(entry);		
		++self->opcount;
	}
}

PatternNode* pattern_insert(Pattern* self, PatternNode* prev, int track,
	psy_dsp_beat_t offset, const PatternEvent* event)
{
	PatternNode* rv;
	PatternEntry* entry;

	if (event) {
		entry = (PatternEntry*)malloc(sizeof(PatternEntry));
		entry->event = *event;
		entry->offset = offset;
		entry->delta = 0.f;
		entry->bpm = 0.f;
		entry->track = track;
		if (!self->events) {
			rv = self->events = list_create(entry);
		} else {	
			rv = list_insert(&self->events, prev, entry);		
		}	
		++self->opcount;
	}
	return rv;
}

void pattern_setevent(Pattern* self, PatternNode* node, const PatternEvent* event)
{
	if (node) {
		PatternEntry* entry;
			
		entry = (PatternEntry*) node->entry;
		if (event) {			
			entry->event = *event;
		} else {
			PatternEvent empty;

			patternevent_clear(&empty);
			entry->event = empty;
		}
		++self->opcount;
	}
}

PatternEvent pattern_event(Pattern* self, PatternNode* node)
{
	if (node) {
		return ((PatternEntry*)node->entry)->event;
	} else {
		PatternEvent empty;

		patternevent_clear(&empty);
		return empty;
	}	
}

PatternNode* pattern_greaterequal(Pattern* self, psy_dsp_beat_t offset)
{
	PatternNode* p;		
	p = self->events;
	while (p != 0) {
		PatternEntry* entry = (PatternEntry*)p->entry;
		if (entry->offset >= offset) {			
			break;
		}		
		p = p->next;
	}	
	return p;
}

PatternNode* pattern_findnode(Pattern* pattern, unsigned int track, float offset, 
	unsigned int subline, psy_dsp_beat_t bpl, PatternNode** prev)
{
	unsigned int currsubline = 0;
	int first = 1;
	PatternNode* node = pattern_greaterequal(pattern, offset);	
	if (node) {
		*prev = node->prev;
	} else {
		*prev = pattern_last(pattern);
	}	
	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			node = 0;
			break;
		}
		if (entry->track == 0 && !first) {
			++currsubline;				
		}
		if (subline < currsubline) {			
			node = 0;
			break;
		}
		if (entry->track > track && subline == currsubline) {			
			node = 0;
			break;
		}		
		if (entry->track == track && subline == currsubline) {
			break;
		}				
		*prev = node;		
		node = node->next;
		first = 0;
	}
	return node;
}

PatternNode* pattern_last(Pattern* self)
{	
	return self->events ? self->events->tail : 0;
}

void pattern_setlabel(Pattern* self, const char* text)
{
	if (self->label) {
		free(self->label);
		self->label = _strdup(text);
	}
	++self->opcount;
}

void pattern_setlength(Pattern* self, psy_dsp_beat_t length)
{
	self->length = length;
	++self->opcount;
}

int pattern_empty(Pattern* self)
{
	return self->events == 0;
}

unsigned int pattern_opcount(Pattern* self)
{
	return self->opcount;
}

void pattern_scale(Pattern* self, float factor)
{
	PatternNode* p;	
	
	for (p = self->events; p != 0; p = p->next) {		
		PatternEntry* entry = (PatternEntry*)p->entry;

		entry->offset *= factor;
	}
}

void pattern_blockremove(Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end)
{	
	PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);	
	while (p != 0) {			
		PatternEntry* entry;
		q = p->next;

		entry = (PatternEntry*) p->entry;
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

void pattern_blocktranspose(Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end, int offset)
{	
	PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);
	while (p != 0) {			
		PatternEntry* entry;
		q = p->next;

		entry = (PatternEntry*) p->entry;
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				if (entry->event.note < NOTECOMMANDS_RELEASE) {
					if (entry->event.note + offset < 0) {
						entry->event.note = 0;
					} else {
						entry->event.note += offset;
					}					
					if (entry->event.note >= NOTECOMMANDS_RELEASE) {
						entry->event.note = NOTECOMMANDS_RELEASE - 1;
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

void pattern_changemachine(Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end, int machine)
{
		PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);
	while (p != 0) {			
		PatternEntry* entry;
		q = p->next;

		entry = (PatternEntry*) p->entry;
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				entry->event.mach = machine;
			}
		} else {
			break;
		}	
		p = q;
	}
}

void pattern_changeinstrument(Pattern* self, PatternEditPosition begin, 
	PatternEditPosition end, int instrument)
{
	PatternNode* p;
	PatternNode* q;

	p = pattern_greaterequal(self, (psy_dsp_beat_t) begin.offset);
	while (p != 0) {			
		PatternEntry* entry;
		q = p->next;

		entry = (PatternEntry*) p->entry;
		if (entry->offset < end.offset) {
			if (entry->track >= begin.track && entry->track < end.track) {
				entry->event.inst = instrument;
			}
		} else {
			break;
		}	
		p = q;
	}
}

void pattern_setmaxsongtracks(Pattern* self, uintptr_t num)
{
	self->maxsongtracks = num;
}

uintptr_t pattern_maxsongtracks(Pattern* self)
{
	return self->maxsongtracks;
}
