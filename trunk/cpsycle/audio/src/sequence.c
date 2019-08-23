// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "sequence.h"
#include <stdlib.h>

static PatternNode* sequenceptr_next(SequencePtr* self);
static void sequenceptr_unget(SequencePtr* self);
static void sequence_reposition(Sequence* self);
static SequencePtr sequence_makeptr(Sequence* self, List* entries);

void sequence_init(Sequence* self, Patterns* patterns)
{
	self->entries = 0;
	self->patterns = patterns;
	self->editpos.patternnode = 0;
	self->editpos.sequence = 0;
	signal_init(&self->signal_editposchanged);
}

void sequence_dispose(Sequence* self)
{
	Track* ptr;
	List* next;

	ptr = self->entries;
	while (ptr) {
		next = ptr->next;
		free(ptr->node);
		ptr = next;
	}
	list_free(self->entries);
	signal_dispose(&self->signal_editposchanged);
}

SequenceEntry* sequence_insert(Sequence* self, SequencePtr position, int pattern)
{		
	SequenceEntry* entry = (SequenceEntry*) malloc(sizeof(SequenceEntry));
	entry->pattern = pattern;
	entry->offset = 0;
	if (self->entries) {
		Track* ptr = position.sequence;
		if (ptr) {			
			self->editpos = sequence_makeptr(self, list_insert(self->entries, position.sequence, entry));
			sequence_reposition(self);						
		}
	} else {		
		self->entries = list_create(entry);		
		self->editpos = sequence_makeptr(self, self->entries);
	}
	signal_emit(&self->signal_editposchanged, self, 0);
	return entry;
}


void sequence_remove(Sequence* self, SequencePtr position)
{				
	Track* ptr = position.sequence;					
	SequenceEntry* entry = (SequenceEntry*)ptr->node;
	ptr = list_remove(&self->entries, ptr);	
	if (self->entries != NULL) {
		sequence_reposition(self);	
		if (ptr) {
			self->editpos = sequence_begin(self, entry->offset);
		} else {
			self->editpos = sequence_last(self);
		}
	} else {
		self->editpos = sequence_makeptr(self, 0);
	}
	signal_emit(&self->signal_editposchanged, self, 0);
}

SequencePtr sequence_last(Sequence* self)
{
	SequencePtr ptr;
	ptr.patternnode = 0;		
	ptr.sequence = self->entries->tail;
	if (ptr.sequence) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) ptr.sequence->node;
		pattern = patterns_at(self->patterns, entry->pattern);		
		ptr.patternnode = pattern->events;
	}
	return ptr;	
}

void sequence_reposition(Sequence* self)
{
	float curroffset = 0.0f;
	Track* ptr = self->entries;	
	while (ptr) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) ptr->node;
		pattern = patterns_at(self->patterns, entry->pattern);
		entry->offset = curroffset;
		curroffset += pattern->length;
		ptr = ptr->next;
	}
}

unsigned int sequence_size(Sequence* self)
{
	Track* ptr;	
	unsigned int c = 0;

	ptr = self->entries;
	while (ptr) {		
		ptr = ptr->next;
		++c;
	}
	return c;
}

SequencePtr sequence_at(Sequence* self, unsigned int position)
{
	SequencePtr rv;	
	Track* ptr;	
	unsigned int c = 0;	
	
	rv.patternnode = 0;
	rv.sequence = 0;
	ptr = self->entries;
	while (ptr) {
		if (c == position) {			
			rv = sequence_makeptr(self, ptr);
			break;
		}
		++c;
		ptr = ptr->next;
	}
	return rv;
}

Track* sequence_at_offset(Sequence* self, float offset)
{
	float curroffset = 0.0f;
	Track* ptr = self->entries;	
	while (ptr) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) ptr->node;
		pattern = patterns_at(self->patterns, entry->pattern);
		if (offset >= curroffset && offset < curroffset + pattern->length) {
			break;
		}
		curroffset += pattern->length;
		ptr = ptr->next;
	}
	return ptr;
}

SequencePtr sequence_begin(Sequence* self, float pos)
{		
	return sequence_makeptr(self, sequence_at_offset(self, pos));	
}

void sequenceptr_inc(SequencePtr* self) {	
	if (self->patternnode) {		
		self->patternnode = self->patternnode->next;
		if (self->patternnode == NULL) {
			if (self->sequence->next) {
				SequenceEntry* entry;
				Pattern* pattern;
				self->sequence = self->sequence->next;			
				entry = (SequenceEntry*) self->sequence->node;
				pattern = patterns_at(self->patterns, entry->pattern);
				self->patternnode = pattern->events;
			}
		}
	}
}

void sequence_seteditposition(Sequence* self, SequencePtr position)
{
	self->editpos = position;	
	signal_emit(&self->signal_editposchanged, self, 0);
}

SequencePtr sequence_editposition(Sequence* self)
{
	return self->editpos;
}

SequencePtr sequence_makeptr(Sequence* self, List* entries)
{
	SequencePtr rv;
	Pattern* pPattern  = 0;	
	rv.patterns = self->patterns;
	rv.sequence = entries;
	if (entries) {
		SequenceEntry* entry = (SequenceEntry*) entries->node;
		pPattern = patterns_at(self->patterns, entry->pattern);
		rv.patternnode = pPattern->events;
	}		
	return rv;
}

PatternNode* sequenceptr_patternnode(SequencePtr* self)
{
	return self->patternnode;
}

SequenceEntry* sequenceptr_entry(SequencePtr* self)
{
	return self->sequence ? (SequenceEntry*) self->sequence->node : 0;
}
