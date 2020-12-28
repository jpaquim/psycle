// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SONG_H
#define psy_audio_SONG_H

// local
#include "instruments.h"
#include "machines.h"
#include "patterns.h"
#include "samples.h"
#include "sequence.h"

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
	uintptr_t tpb;
	uintptr_t extraticksperbeat;
	// read-only
	char* title;
	char* credits;
	char* comments;
	psy_dsp_big_beat_t bpm;
} psy_audio_SongProperties;

void psy_audio_songproperties_init(psy_audio_SongProperties*, const char* title,
	const char* credits, const char* comments);
void psy_audio_songproperties_init_all(psy_audio_SongProperties*, const char* title,
	const char* credits, const char* comments,
	uintptr_t tracks,
	int octave,
	uintptr_t lpb,
	int tpb,
	int extraticksperbeat,
	psy_dsp_big_beat_t bpm);
void psy_audio_songproperties_copy(psy_audio_SongProperties*, const psy_audio_SongProperties* other);
void psy_audio_songproperties_dispose(psy_audio_SongProperties*);

/// Properties

INLINE void psy_audio_songproperties_setbpm(psy_audio_SongProperties* self,
	psy_dsp_big_beat_t bpm)
{
	assert(self);

	if (bpm < 32) {
		self->bpm = 32;
	} else if (bpm > 999) {
		self->bpm = 999;
	} else {
		self->bpm = bpm;
	}
}

INLINE psy_dsp_big_beat_t psy_audio_songproperties_bpm(const
	psy_audio_SongProperties* self)
{
	return self->bpm;
}

INLINE void psy_audio_songproperties_setlpb(psy_audio_SongProperties* self,
	uintptr_t lpb)
{
	self->lpb = lpb;
}

INLINE uintptr_t psy_audio_songproperties_lpb(const
	psy_audio_SongProperties* self)
{
	return self->lpb;
}

INLINE uintptr_t psy_audio_songproperties_numtracks(const
	psy_audio_SongProperties* self)
{
	return self->tracks;
}

INLINE const char* psy_audio_songproperties_title(const
	psy_audio_SongProperties* self)
{
	return self->title;
}

void psy_audio_songproperties_setcomments(psy_audio_SongProperties* self,
	const char* comments);

INLINE const char* psy_audio_songproperties_comments(const
	psy_audio_SongProperties* self)
{
	assert(self);

	return self->comments;
}

/// psy_audio_Song
typedef struct psy_audio_Song {
	// signals
	psy_Signal signal_loadprogress;
	psy_Signal signal_saveprogress;
	// public data
	psy_audio_SongProperties properties;
	psy_audio_Machines machines;
	psy_audio_Patterns patterns;
	psy_audio_Sequence sequence;
	psy_audio_Samples samples;
	psy_audio_Instruments instruments;	
	// references
	struct psy_audio_MachineFactory* machinefactory;
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
/// clears the song completly (no master, no pattern, no sequence track/entry)
void psy_audio_song_clear(psy_audio_Song*);
/// adds a virtual generator
void psy_audio_song_insertvirtualgenerator(psy_audio_Song*,
	uintptr_t virtual_inst, uintptr_t mac_idx, uintptr_t inst_idx);
/// getter of the song parts
/// return: Machines of song
INLINE psy_audio_Machines* psy_audio_song_machines(psy_audio_Song* self)
{
	assert(self);

	return &self->machines;
}
/// return: Patterns of song
INLINE psy_audio_Patterns* psy_audio_song_sequence(psy_audio_Song* self)
{
	assert(self);

	return &self->patterns;
}
/// return: Sequence of song
INLINE psy_audio_Sequence* psy_audio_song_(psy_audio_Song* self)
{
	assert(self);

	return &self->sequence;
}
/// return: Samples of song
INLINE psy_audio_Samples* psy_audio_song_samples(psy_audio_Song* self)
{
	assert(self);

	return &self->samples;
}
/// return: Instruments of song
INLINE psy_audio_Instruments* psy_audio_song_instruments(psy_audio_Song* self)
{
	assert(self);

	return &self->instruments;
}

/// Properties

/// set song properties
void psy_audio_song_setproperties(psy_audio_Song*, const psy_audio_SongProperties*);
/// return song title
INLINE const char* psy_audio_song_title(const psy_audio_Song* self)
{
	assert(self);

	return psy_audio_songproperties_title(&self->properties);
}

/// set song comments
INLINE void psy_audio_song_setcomments(psy_audio_Song* self,
	const char* comments)
{
	assert(self);

	psy_audio_songproperties_setcomments(&self->properties, comments);
}

/// return song comments
INLINE const char* psy_audio_song_comments(const psy_audio_Song* self)
{
	return psy_audio_songproperties_comments(&self->properties);
}

/// set song properties bpm
void psy_audio_song_setbpm(psy_audio_Song*, psy_dsp_big_beat_t bpm);
/// return song properties bpm
INLINE psy_dsp_big_beat_t psy_audio_song_bpm(const psy_audio_Song* self)
{
	assert(self);

	return psy_audio_songproperties_bpm(&self->properties);
}
/// set song properties lpb
INLINE void psy_audio_song_setlpb(psy_audio_Song* self, uintptr_t lpb)
{
	assert(self);

	psy_audio_songproperties_setlpb(&self->properties, lpb);
}
/// return song properties lpb
INLINE uintptr_t psy_audio_song_lpb(const psy_audio_Song* self)
{
	assert(self);

	return psy_audio_songproperties_lpb(&self->properties);
}
/// return song numtracks (pattern channels)
INLINE uintptr_t psy_audio_song_numtracks(const psy_audio_Song* self)
{
	assert(self);

	return psy_audio_songproperties_numtracks(&self->properties);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SONG_H */
