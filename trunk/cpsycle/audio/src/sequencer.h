// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SEQUENCER_H)
#define SEQUENCER_H

#include "sequence.h"
#include <list.h>

typedef struct {
	Sequence* sequence;
	float position;
	float bpm;
	float window;
	unsigned int samplerate;
	List* currtrackiterators;	
	List* events;
	List* delayedevents;
} Sequencer;

void sequencer_init(Sequencer*);
void sequencer_dispose(Sequencer*);
void sequencer_tick(Sequencer*, float offset);
void sequencer_setposition(Sequencer*, float position);

#endif