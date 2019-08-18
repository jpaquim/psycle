// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "sequence.h"
#include <stdlib.h>


void sequence_init(Sequence* self, Patterns* patterns)
{
	self->entries = 0;
	self->patterns = patterns;
}

void sequence_dispose(Sequence* self)
{
	List* ptr;
	List* next;

	ptr = self->entries;
	while (ptr) {
		next = ptr->next;
		free(ptr->node);
		ptr = next;
	}
	list_free(self->entries);
}

void sequence_insert(Sequence* self, float offset, int pattern)
{	
	SequenceEntry* entry = (SequenceEntry*) malloc(sizeof(SequenceEntry));
	entry->pattern = pattern;
	entry->offset = offset;

	if (!self->entries) {
		self->entries = list_create();
		self->entries->node = entry;
	} else {
		list_append(self->entries, entry);
	}	
}

SequenceEntry* sequence_append(Sequence* self, int pattern)
{	
	SequenceEntry* entry = 0;
	if (self->entries) {
		List* ptr = self->entries->tail;		
		if (ptr) {
			Pattern* lastpattern;
			SequenceEntry* entry = (SequenceEntry*) ptr->node;
			lastpattern = patterns_at(self->patterns, entry->pattern);
			sequence_insert(self, entry->offset + lastpattern->length, pattern);
		}
	} else {
		sequence_insert(self, 0, pattern);
	}
	return entry;
}

unsigned int sequence_size(Sequence* self)
{
	List* ptr;	
	unsigned int c = 0;

	ptr = self->entries;
	while (ptr) {		
		ptr = ptr->next;
		++c;
	}
	return c;
}

SequenceEntry* sequence_at(Sequence* self, unsigned int position)
{
	SequenceEntry* entry = 0;
	List* ptr;	
	unsigned int c = 0;

	ptr = self->entries;
	while (ptr) {
		if (c == position) {
			entry = (SequenceEntry*) ptr->node;
			break;
		}
		++c;
		ptr = ptr->next;
	}
	return entry;
}
