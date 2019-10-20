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

/// Song hold everything comprising a "tracker module",
/// this include patterns, pattern sequence, machines and their initial 
/// parameters and coordinates, wavetables, ...

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

/// initializes a song
void song_init(Song*, MachineFactory*);
/// frees all memory used
void song_dispose(Song*);
/// allocates a song
///\return allocates a song
Song* song_alloc(void);
/// allocates and initializes a song
///\return allocates and initializes a song
Song* song_allocinit(MachineFactory*);
/// loads a song
///\return ui properties (coordinates, ...)
void song_load(Song*, const char* path, Properties** workspaceproperties);

#endif
