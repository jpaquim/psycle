// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "sequencer.h"

void sequencer_init(Sequencer* self)
{
	self->sequence = 0;
	self->pos = 0;
	self->samplerate = 44100;
	self->bpm = 125;	
	self->window = 0;
}

void sequencer_connect(Sequencer* self, void* context, void (*callback)(void*, PatternNode*))
{
	self->context = context;
	self->callback = callback;
}

void sequencer_setposition(Sequencer* self, float position)
{
	self->pos = 0.0f;
	self->window = 0.0f;
	self->curr = sequence_begin(self->sequence, 0.0f);		
}

void sequencer_tick(Sequencer* self, float offset)
{		
	self->pos += self->window;
	self->window = offset;		
}

SequencePtr sequencer_curr(Sequencer* self)
{
	return self->curr;
}

void sequencer_enumerate(Sequencer* self, void* context, int slot, void (*callback)(void*, int, PatternNode*))
{
	PatternNode* node = sequenceptr_patternnode(&self->curr);
	SequenceEntry* entry = sequenceptr_entry(&self->curr);
	while (node) {
		PatternEntry* patternentry = (PatternEntry*)node->entry;
		if (entry->offset + patternentry->offset >= self->pos && entry->offset + patternentry->offset < self->pos + self->window) {
			callback(context, slot, node);
			sequenceptr_inc(&self->curr);
			node = sequenceptr_patternnode(&self->curr);
			entry = sequenceptr_entry(&self->curr);
		} else {			
			break;
		}				
	}

}