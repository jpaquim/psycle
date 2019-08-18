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
	self->curr = 0; // ((SequenceEntry*)(self->sequence->entries->node))->pattern->events;
}

void sequencer_tick(Sequencer* self, float offset)
{		
	self->pos += offset;	
}


void sequencer_enumerate(Sequencer* self, void* context, float offset,
	void (*callback)(void*, PatternNode*))
{
	while (self->curr && 
		self->curr->offset >= self->pos && self->curr->offset < self->pos + offset) {
		self->curr->delta = self->curr->offset - self->pos;
		callback(context, self->curr);
		self->curr = self->curr->next;
	}
}