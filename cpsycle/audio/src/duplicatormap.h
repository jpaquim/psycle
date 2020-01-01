// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUPLICATORMAP_H)
#define DUPLICATORMAP_H

#include "machine.h"

#define MAX_TRACKS 64
#define MAX_VIRTUALINSTS 256

typedef struct {
	// returns the allocated channel of the machine, for the channel (duplicator's channel) of this tick.
	int allocatedchans[MAX_TRACKS][MAX_VIRTUALINSTS]; //Not Using MAX_MACHINES because now there are the Virtual instruments
	// indicates if the channel of the specified machine is in use or not
	int availablechans[MAX_VIRTUALINSTS][MAX_TRACKS];//Not Using MAX_MACHINES because now there are the Virtual instruments	
} psy_audio_DuplicatorMap;

void duplicatormap_init(psy_audio_DuplicatorMap*);
void duplicatormap_dispose(psy_audio_DuplicatorMap*);
int duplicatormap_at(psy_audio_DuplicatorMap*, int channel, int machine);
void duplicatormap_allocate(psy_audio_DuplicatorMap*, int channel, int machine, int outputmachine);
void duplicatormap_remove(psy_audio_DuplicatorMap*, int channel, int machine, int outputmachine);

#endif
