// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONG_H)
#define SONG_H

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
	char* title;
	char* credits;
	char* comments;
	beat_t bpm;
	uintptr_t lpb;
	uintptr_t tracks;
	int octave;
	int tpb;
	int extraticksperbeat;
} SongProperties;

typedef struct Song {
	SongProperties properties;
	Machines machines;
	Patterns patterns;
	Sequence sequence;
	Samples samples;
	Instruments instruments;
	XMInstruments xminstruments;
	MachineFactory* machinefactory;
	Signal signal_loadprogress;
	Signal signal_saveprogress;
} Song;

/// initializes a song with a master and one sequence track/entry/pattern
void song_init(Song*, MachineFactory*);
/// frees all internal memory used by the songstruct
void song_dispose(Song*);
/// allocates a song
///\return allocates a song
Song* song_alloc(void);
/// allocates and initializes a song
///\return allocates and initializes a song
Song* song_allocinit(MachineFactory*);
/// disposes and deallocates the song
void song_free(Song*);
/// Clears the song completly (no master, no pattern, no sequence track/entry)
void song_clear(Song*);

#endif
