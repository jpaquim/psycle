// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pattern.h"
#include <stdlib.h>
#include <string.h>

PatternEntry* patternentry_clone(PatternEntry* entry)
{
	PatternEntry* rv;
	if (entry) {			
		rv = (PatternEntry*)malloc(sizeof(PatternEntry));
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
	self->label = _strdup("Untitled");	
	self->opcount = 0;
}

void pattern_dispose(Pattern* self)
{
	PatternNode* p;	
	
	for (p = self->events; p != 0; p = p->next) {		
		free (p->entry);		
	}
	list_free(self->events);
	self->events = 0;
	free(self->label);
	self->label = 0;
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
	PatternNode* p;	

	rv = pattern_alloc();
	rv->events = 0;			
	for (p = self->events; p != 0; p = p->next) {
		PatternEntry* entry;
		PatternEntry* rventry;

		entry = (PatternEntry*)p->entry;
		rventry = (PatternEntry*)malloc(sizeof(PatternEntry));
		if (entry) {
			*rventry = *entry;
		} else {
			memset(rventry, 0, sizeof(PatternEntry));
		}		
		list_append(&rv->events, rventry);		
	}
	rv->length = self->length;
	rv->label = strdup(self->label);
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
	beat_t offset, const PatternEvent* event)
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

PatternNode* pattern_greaterequal(Pattern* self, beat_t offset)
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

void pattern_setlength(Pattern* self, beat_t length)
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
