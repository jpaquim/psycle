// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SEQUENCER_H)
#define SEQUENCER_H

#include "sequence.h"
#include <list.h>

typedef struct {
	Sequence* sequence;
	float pos;
	float bpm;
	float window;
	unsigned int samplerate;
	SequenceIterator curr;
	void* context;
	void (*callback)(void*, PatternNode*);
	List* events;
} Sequencer;

void sequencer_init(Sequencer* self);
void sequencer_connect(Sequencer*, void* context, void (*callback)(void*, PatternNode*));
void sequencer_tick(Sequencer*, float offset);
void sequencer_setposition(Sequencer*, float position);
SequenceIterator sequencer_curr(Sequencer*);

#endif