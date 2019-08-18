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
}

void pattern_write(Pattern* self, int track, float offset, PatternEvent event)
{
	PatternNode* ptr;	
	PatternNode* prev;
		
	prev = 0;
	ptr = self->events;
	while (ptr != 0) {
		if (ptr->offset > offset) {
			ptr = prev;
			break;
		} else
		if (ptr->offset == offset) {						
			while (ptr != 0 && ptr->offset == offset && track >= ptr->track) {
				if (ptr->track == track) {
					ptr->event = event;
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
	ptr = (PatternNode*)malloc(sizeof(PatternNode));
	ptr->event = event;
	ptr->offset = offset;
	ptr->delta = 0;
	ptr->next = 0;
	ptr->track = track;
	if (!self->events) {		
		self->events = ptr;		
	} else 
	if (prev != 0) {
		PatternNode* tmp;
		tmp = prev->next;
		prev->next = ptr;
		ptr->next = tmp;
	} else {
	   ptr->next = self->events;
	   self->events = ptr;
	}
}
