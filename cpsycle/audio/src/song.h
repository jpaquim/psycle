// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(SONG_H)
#define SONG_H

#include <properties.h>
#include "machines.h"
#include "patterns.h"
#include "sequence.h"
#include "samples.h"
#include "instruments.h"
#include "xminstruments.h"
#include "machinefactory.h"

typedef struct {
	Properties* properties;
	Machines machines;
	Patterns patterns;
	Sequence sequence;
	Samples samples;
	Instruments instruments;
	XMInstruments xminstruments;
	MachineFactory* machinefactory;
} Song;

void song_init(Song* self, MachineFactory*);
void song_dispose(Song* self);
void song_load(Song* self, const char* path,
			   Properties** workspaceproperties);

#endif
