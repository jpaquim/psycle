// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "sequence.h"
#include <stdlib.h>

static PatternNode* SequenceIterator_next(SequenceIterator* self);
static void SequenceIterator_unget(SequenceIterator* self);
static void sequence_reposition(Sequence* self);
static SequenceIterator sequence_makeiterator(Sequence* self, List* entries);

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
		free(ptr->entry);
		ptr = next;
	}
	list_free(self->entries);
	signal_dispose(&self->signal_editposchanged);
}

SequenceEntry* sequence_insert(Sequence* self, SequenceIterator position, int pattern)
{		
	SequenceEntry* entry = (SequenceEntry*) malloc(sizeof(SequenceEntry));
	entry->pattern = pattern;
	entry->offset = 0;
	if (self->entries) {
		Track* ptr = position.sequence;
		if (ptr) {			
			self->editpos = sequence_makeiterator(self, list_insert(&self->entries, position.sequence, entry));
			sequence_reposition(self);						
		}
	} else {		
		self->entries = list_create(entry);		
		self->editpos = sequence_makeiterator(self, self->entries);
	}
	signal_emit(&self->signal_editposchanged, self, 0);
	return entry;
}


void sequence_remove(Sequence* self, SequenceIterator position)
{				
	Track* ptr = position.sequence;					
	SequenceEntry* entry = (SequenceEntry*)ptr->entry;
	ptr = list_remove(&self->entries, ptr);	
	if (self->entries != NULL) {
		sequence_reposition(self);	
		if (ptr) {
			self->editpos = sequence_begin(self, entry->offset);
		} else {
			self->editpos = sequence_last(self);
		}
	} else {
		self->editpos = sequence_makeiterator(self, 0);
	}
	signal_emit(&self->signal_editposchanged, self, 0);
}

void sequence_clear(Sequence* self)
{
	Track* ptr;
	List* next;

	ptr = self->entries;
	while (ptr) {
		next = ptr->next;
		free(ptr->entry);
		ptr = next;
	}
	list_free(self->entries);
	self->entries = 0;
	self->editpos.patternnode = 0;
	self->editpos.sequence = 0;
}

SequenceIterator sequence_last(Sequence* self)
{
	SequenceIterator ptr;

	if (self->entries == 0) {
		return sequence_begin(self, 0);
	}
	ptr.patternnode = 0;		
	ptr.sequence = self->entries->tail;
	if (ptr.sequence) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) ptr.sequence->entry;
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {
			ptr.patternnode = pattern->events;
		} else {
			ptr.patternnode = 0;
		}
	}
	return ptr;	
}

void sequence_reposition(Sequence* self)
{
	float curroffset = 0.0f;
	Track* ptr = self->entries;	
	while (ptr) {
		Pattern* pattern;
		SequenceEntry* entry = (SequenceEntry*) ptr->entry;
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {
			entry->offset = curroffset;
			curroffset += pattern->length;
		} else {
			entry->offset = curroffset;
		}
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

SequenceIterator sequence_at(Sequence* self, unsigned int position)
{
	SequenceIterator rv;	
	Track* ptr;	
	unsigned int c = 0;	
	
	rv.patternnode = 0;
	rv.sequence = 0;
	ptr = self->entries;
	while (ptr) {
		if (c == position) {			
			rv = sequence_makeiterator(self, ptr);
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
		SequenceEntry* entry = (SequenceEntry*) ptr->entry;
		pattern = patterns_at(self->patterns, entry->pattern);
		if (pattern) {
			if (offset >= curroffset && offset < curroffset + pattern->length) {
				break;
			}
			curroffset += pattern->length;
		}
		ptr = ptr->next;
	}
	return ptr;
}

SequenceIterator sequence_begin(Sequence* self, float pos)
{		
	return sequence_makeiterator(self, sequence_at_offset(self, pos));	
}

void sequenceiterator_inc(SequenceIterator* self) {	
	if (self->patternnode) {		
		self->patternnode = self->patternnode->next;
		if (self->patternnode == NULL) {
			if (self->sequence->next) {
				SequenceEntry* entry;
				Pattern* pattern;
				self->sequence = self->sequence->next;			
				entry = (SequenceEntry*) self->sequence->entry;
				pattern = patterns_at(self->patterns, entry->pattern);
				self->patternnode = pattern->events;
			}
		}
	}
}

void sequence_seteditposition(Sequence* self, SequenceIterator position)
{
	self->editpos = position;	
	signal_emit(&self->signal_editposchanged, self, 0);
}

SequenceIterator sequence_editposition(Sequence* self)
{
	return self->editpos;
}

SequenceIterator sequence_makeiterator(Sequence* self, List* entries)
{
	SequenceIterator rv;
	Pattern* pPattern  = 0;	
	rv.patterns = self->patterns;
	rv.sequence = entries;
	if (entries) {
		SequenceEntry* entry = (SequenceEntry*) entries->entry;
		pPattern = patterns_at(self->patterns, entry->pattern);
		if (pPattern) {
			rv.patternnode = pPattern->events;
		} else {
			rv.patternnode = 0;
		}
	}		
	return rv;
}

PatternNode* sequenceiterator_patternnode(SequenceIterator* self)
{
	return self->patternnode;
}

SequenceEntry* sequenceiterator_entry(SequenceIterator* self)
{
	return self->sequence ? (SequenceEntry*) self->sequence->entry : 0;
}

PatternEntry* sequenceiterator_patternentry(SequenceIterator* self)
{
	return self->patternnode ? (PatternEntry*)(self->patternnode)->entry : 0;
}

float sequenceiterator_offset(SequenceIterator* self)
{	
	return sequenceiterator_patternentry(self)
		? sequenceiterator_entry(self)->offset +
		  sequenceiterator_patternentry(self)->offset
		: 0.f;	
}
