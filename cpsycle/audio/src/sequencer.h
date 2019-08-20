// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SEQUENCER_H)
#define SEQUENCER_H

#include "sequence.h"

typedef struct {
	Sequence* sequence;
	float pos;
	float bpm;
	float window;
	unsigned int samplerate;
	SequencePtr curr;
	void* context;
	void (*callback)(void*, PatternNode*);
} Sequencer;

void sequencer_init(Sequencer* self);
void sequencer_connect(Sequencer*, void* context, void (*callback)(void*, PatternNode*));
void sequencer_tick(Sequencer*, float offset);
void sequencer_enumerate(Sequencer*, void* context, int slot, void (*callback)(void*, int, PatternNode*));
void sequencer_setposition(Sequencer*, float position);
SequencePtr sequencer_curr(Sequencer*);

#endif