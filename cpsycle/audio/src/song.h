// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SONG_H
#define psy_audio_SONG_H

#include "machines.h"
#include "patterns.h"
#include "sequence.h"
#include "samples.h"
#include "instruments.h"

/// psy_audio_Song hold everything comprising a "tracker module",
/// this include patterns, pattern sequence, machines and their initial 
/// parameters and coordinates, wavetables, ...

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// public
	uintptr_t tracks;
	int octave;
	uintptr_t lpb;
	int tpb;
	int extraticksperbeat;	
	// read-only
	char* title;
	char* credits;
	char* comments;
	psy_dsp_big_beat_t bpm;
} SongProperties;

void songproperties_init(SongProperties*, const char* title,
	const char* credits, const char* comments);
void songproperties_copy(SongProperties*, const SongProperties* other);
void songproperties_dispose(SongProperties*);
void songproperties_setbpm(SongProperties*, psy_dsp_big_beat_t bpm);

INLINE psy_dsp_big_beat_t songproperties_bpm(SongProperties* self)
{
	return self->bpm;
}

INLINE uintptr_t songproperties_lpb(SongProperties* self)
{
	return self->lpb;
}

typedef struct psy_audio_Song {
	SongProperties properties;
	psy_audio_Machines machines;
	psy_audio_Patterns patterns;
	psy_audio_Sequence sequence;
	psy_audio_Samples samples;
	psy_audio_Instruments instruments;
	struct psy_audio_MachineFactory* machinefactory;
	psy_Signal signal_loadprogress;
	psy_Signal signal_saveprogress;
} psy_audio_Song;

/// initializes a song with a master and one sequence track/entry/pattern
void psy_audio_song_init(psy_audio_Song*, struct psy_audio_MachineFactory*);
/// frees all internal memory used by the songstruct
void psy_audio_song_dispose(psy_audio_Song*);
/// allocates a song
///\return allocates a song
psy_audio_Song* psy_audio_song_alloc(void);
/// allocates and initializes a song
///\return allocates and initializes a song
psy_audio_Song* psy_audio_song_allocinit(struct psy_audio_MachineFactory*);
/// calls dispose and deallocates memory allocated by song_alloc
void psy_audio_song_deallocate(psy_audio_Song*);
/// Clears the song completly (no master, no pattern, no sequence track/entry)
void psy_audio_song_clear(psy_audio_Song*);
/// set song properties
void psy_audio_song_setproperties(psy_audio_Song*, const SongProperties*);
/// return song properties title
INLINE const char* psy_audio_song_title(psy_audio_Song* self)
{
	return self->properties.title;
}
/// set song properties bpm
void psy_audio_song_setbpm(psy_audio_Song*, psy_dsp_big_beat_t bpm);
/// return song properties bpm
psy_dsp_big_beat_t psy_audio_song_bpm(psy_audio_Song*);
/// return song properties lpb
uintptr_t psy_audio_song_lpb(psy_audio_Song*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SONG_H */
