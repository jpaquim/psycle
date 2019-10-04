// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCER_H)
#define SEQUENCER_H

#include "sequence.h"
#include <list.h>

typedef struct {
	Sequence* sequence;
	beat_t position;
	beat_t bpm;
	beat_t window;
	unsigned int samplerate;
	List* currtrackiterators;	
	List* events;
	List* delayedevents;
} Sequencer;

void sequencer_init(Sequencer*, Sequence*);
void sequencer_dispose(Sequencer*);
void sequencer_reset(Sequencer*, Sequence*);
void sequencer_tick(Sequencer*, beat_t offset);
void sequencer_setposition(Sequencer*, beat_t position);

#endif