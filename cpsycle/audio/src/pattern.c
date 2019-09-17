// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "pattern.h"
#include <stdlib.h>
#include <string.h>

void pattern_init(Pattern* self)
{
	self->events = 0;
	self->length = 16;
	self->label = strdup("Untitled");
}

Pattern* pattern_clone(Pattern* self)
{	
	Pattern* rv;
	PatternNode* p;	

	rv = (Pattern*) malloc(sizeof(Pattern));
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
		if (rv->events) {						
			list_append(rv->events, rventry);
		} else {
			rv->events = list_create(rventry);
		}
	}
	rv->length = self->length;
	rv->label = strdup(self->label);
	return rv;
}

void pattern_free(Pattern* self)
{
	PatternNode* p;
	PatternNode* next;

	free(self->label);
	self->label = 0;
	p = self->events;
	while (p != 0) {
		next = p->next;
		free (p);
		p = next;
	}
	list_free(self->events);	
}

PatternEntry* pattern_write(Pattern* self, int track, float offset, PatternEvent event)
{
	PatternNode* ptr;	
	PatternNode* prev;
	PatternEntry* entry;
	
	prev = 0;
	ptr = self->events;
	while (ptr != 0) {
		PatternEntry* entry = (PatternEntry*) ptr->entry;
		if (entry->offset > offset) {
			ptr = prev;
			break;
		} else
		if (entry->offset == offset) {						
			while (ptr != 0 && ((PatternEntry*)(ptr->entry))->offset == offset && track >= ((PatternEntry*)(ptr->entry))->track) {
				PatternEntry* entry = (PatternEntry*) ptr->entry;
				if (entry->track == track) {
					entry->event = event;
					return entry;
				}
				prev = ptr;
				ptr = ptr->next;
			}
			break;
		}
		prev = ptr;					
		ptr = ptr->next;
	}		
	entry = (PatternEntry*)malloc(sizeof(PatternEntry));
	entry->event = event;
	entry->offset = offset;
	entry->delta = 0;
	entry->track = track;

	if (!self->events) {
		self->events = list_create(entry);
	} else {	
		list_insert(&self->events, prev, entry);		
	}
	return entry;
}

void pattern_remove(Pattern* self, PatternNode* node)
{
	if (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		list_remove(&self->events, node);
		free(entry);		
	}
}

PatternNode* pattern_insert(Pattern* self, PatternNode* prev, int track, float offset, PatternEvent* event)
{
	PatternNode* rv;
	PatternEntry* entry;

	entry = (PatternEntry*)malloc(sizeof(PatternEntry));
	entry->event = *event;
	entry->offset = offset;
	entry->delta = 0;
	entry->track = track;
	if (!self->events) {
		rv = self->events = list_create(entry);
	} else {	
		rv = list_insert(&self->events, prev, entry);		
	}
	return rv;
}

PatternNode* pattern_greaterequal(Pattern* self, float offset, PatternNode** prev)
{
	PatternNode* p;	
	*prev = 0;
	p = self->events;
	while (p != 0) {
		PatternEntry* entry = (PatternEntry*)p->entry;
		if (entry->offset >= offset) {			
			break;
		}	
		*prev = p;
		p = p->next;
	}	
	return p;
}

void pattern_setlabel(Pattern* self, const char* text)
{
	if (self->label) {
		free(self->label);
		self->label = strdup(text);
	}
}

void pattern_setlength(Pattern* self, float length)
{
	self->length = length;
}