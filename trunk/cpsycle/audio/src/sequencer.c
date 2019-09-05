// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "sequencer.h"
#include "pattern.h"

void sequencer_init(Sequencer* self)
{
	self->sequence = 0;
	self->pos = 0;
	self->samplerate = 44100;
	self->bpm = 125;	
	self->window = 0;
	self->events = 0;
}

void sequencer_connect(Sequencer* self, void* context, void (*callback)(void*, PatternNode*))
{
	self->context = context;
	self->callback = callback;
}

void sequencer_setposition(Sequencer* self, float position)
{
	list_free(self->events);
	self->pos = 0.0f;
	self->window = 0.0f;
	self->curr = sequence_begin(self->sequence, 0.0f);	
}

void sequencer_tick(Sequencer* self, float offset)
{		
	SequenceEntry* entry = sequenceiterator_entry(&self->curr);
	PatternNode* node;
	self->pos += self->window;
	self->window = offset;			
	list_free(self->events);
	self->events = 0;
	node = sequenceiterator_patternnode(&self->curr);
	
	while (node) {
		PatternEntry* patternentry = (PatternEntry*)node->entry;

		if (entry->offset + patternentry->offset >= self->pos && entry->offset + patternentry->offset < self->pos + self->window) {
			patternentry->delta = entry->offset - self->pos;
			if (!self->events) {
				self->events = list_create(patternentry);
			} else {
				list_append(self->events, patternentry);
			}
			sequenceiterator_inc(&self->curr);
			node = sequenceiterator_patternnode(&self->curr);
			entry = sequenceiterator_entry(&self->curr);
		} else {			
			break;
		}				
	}
}

SequenceIterator sequencer_curr(Sequencer* self)
{
	return self->curr;
}

