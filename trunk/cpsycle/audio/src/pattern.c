// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "pattern.h"
#include <stdlib.h>

void pattern_init(Pattern* self)
{
	self->events = 0;
	self->length = 16;
}

void pattern_free(Pattern* self)
{
	PatternNode* ptr;
	PatternNode* next;

	ptr = self->events;
	while (ptr != 0) {
		next = ptr->next;
		free (ptr);
		ptr = next;
	}
	list_free(self->events);
}

void pattern_write(Pattern* self, int track, float offset, PatternEvent event)
{
	PatternNode* ptr;	
	PatternNode* prev;
	PatternEntry* entry;
	
	prev = 0;
	ptr = self->events;
	while (ptr != 0) {
		PatternEntry* entry = (PatternEntry*) ptr->node;
		if (entry->offset > offset) {
			ptr = prev;
			break;
		} else
		if (entry->offset == offset) {						
			while (ptr != 0 && ((PatternEntry*)(ptr->node))->offset == offset && track >= ((PatternEntry*)(ptr->node))->track) {
				PatternEntry* entry = (PatternEntry*) ptr->node;
				if (entry->track == track) {
					entry->event = event;
					return;
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
		list_insert(self->events, prev, entry);		
	}
}

void pattern_remove(Pattern* self, int track, float offset)
{
	PatternNode* node = pattern_greaterequal(self, offset);
	if (node) {
		PatternEntry* entry = (PatternEntry*)(node->node);
		if (node && entry->offset < offset + 0.25) {
			list_remove(&self->events, node);
			free(entry);
		}
	}
}


PatternNode* pattern_greaterequal(Pattern* self, float offset)
{
	PatternNode* ptr;	
	
	ptr = self->events;
	while (ptr != 0) {
		PatternEntry* entry = (PatternEntry*)ptr->node;
		if (entry->offset >= offset) {			
			break;
		}		
		ptr = ptr->next;
	}	
	return ptr;
}
