// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "song.h"

#include "machinefactory.h"

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"

static void song_initproperties(psy_audio_Song*);
static void song_initmachines(psy_audio_Song*);
static void song_initpatterns(psy_audio_Song*);
static void song_initsequence(psy_audio_Song*);
static void song_initsignals(psy_audio_Song*);
static void song_disposesignals(psy_audio_Song*);

void songproperties_init(SongProperties* self, const char* title,
	const char* credits, const char* comments)
{
	assert(self);

	self->title = psy_strdup(title);
	self->credits = psy_strdup(credits);
	self->comments = psy_strdup(comments);
	self->bpm = (psy_dsp_big_beat_t) 125.0;
	self->lpb = 4;
	self->tracks = 16;
	self->octave = 4;
	self->tpb = 24;
	self->extraticksperbeat = 0;
}

void songproperties_init_all(SongProperties* self, const char* title,
	const char* credits, const char* comments,
	uintptr_t tracks,
	int octave,
	uintptr_t lpb,
	int tpb,
	int extraticksperbeat,
	psy_dsp_big_beat_t bpm)
{
	assert(self);

	self->title = psy_strdup(title);
	self->credits = psy_strdup(credits);
	self->comments = psy_strdup(comments);
	self->lpb = lpb;
	self->tracks = tracks;
	self->octave = octave;
	self->tpb = tpb;
	self->extraticksperbeat = extraticksperbeat;
	self->bpm = bpm;
}

void songproperties_dispose(SongProperties* self)
{
	assert(self);

	free(self->title);
	self->title = NULL;
	free(self->credits);
	self->credits = NULL;
	free(self->comments);
	self->comments = NULL;
}

void songproperties_copy(SongProperties* self, const SongProperties* other)
{
	assert(self);

	if (self != other) {
		songproperties_dispose(self);
		self->title = psy_strdup(other->title);
		self->credits = psy_strdup(other->credits);
		self->comments = psy_strdup(other->comments);
		self->bpm = other->bpm;
		self->lpb = other->lpb;
		self->octave = other->octave;
		self->tpb = other->tpb;
		self->extraticksperbeat = other->extraticksperbeat;
		self->tracks = other->tracks;
	}
}

void songproperties_setbpm(SongProperties* self, psy_dsp_big_beat_t bpm)
{
	assert(self);

	if (bpm < 32) {
		self->bpm = 32;
	} else
	if (bpm > 999) {
		self->bpm = 999;
	} else {
		self->bpm = bpm;
	}	
}

void psy_audio_song_init(psy_audio_Song* self, psy_audio_MachineFactory*
	machinefactory)
{
	assert(self);

	self->machinefactory = machinefactory;	
	songproperties_init(&self->properties, "Untitled", "Unnamed",
		"No Comments");	
	song_initmachines(self);
	song_initpatterns(self);
	song_initsequence(self);
	psy_audio_samples_init(&self->samples);
	psy_audio_instruments_init(&self->instruments);
	song_initsignals(self);
}

void song_initmachines(psy_audio_Song* self)
{
	assert(self);

	psy_audio_machines_init(&self->machines);
	psy_audio_machines_insertmaster(&self->machines,
		psy_audio_machinefactory_makemachine(self->machinefactory, MACH_MASTER,
			0));
}

void song_initpatterns(psy_audio_Song* self)
{
	assert(self);

	psy_audio_patterns_init(&self->patterns);
	psy_audio_patterns_insert(&self->patterns, 0,
		psy_audio_pattern_allocinit());
}

void song_initsequence(psy_audio_Song* self)
{
	psy_audio_SequencePosition sequenceposition;

	assert(self);

	psy_audio_sequence_init(&self->sequence, &self->patterns);	
	sequenceposition.track = 
		psy_audio_sequence_appendtrack(&self->sequence,
			psy_audio_sequencetrack_allocinit());
	sequenceposition.trackposition =
		psy_audio_sequence_begin(&self->sequence, sequenceposition.track, 0);
	psy_audio_sequence_insert(&self->sequence, sequenceposition, 0);
}

void song_initsignals(psy_audio_Song* self)
{
	assert(self);

	psy_signal_init(&self->signal_loadprogress);
	psy_signal_init(&self->signal_saveprogress);
}

void psy_audio_song_dispose(psy_audio_Song* self)
{
	assert(self);

	songproperties_dispose(&self->properties);
	psy_audio_machines_dispose(&self->machines);
	psy_audio_patterns_dispose(&self->patterns);
	psy_audio_sequence_dispose(&self->sequence);		
	psy_audio_samples_dispose(&self->samples);
	psy_audio_instruments_dispose(&self->instruments);
	song_disposesignals(self);
}

void song_disposesignals(psy_audio_Song* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_loadprogress);
	psy_signal_dispose(&self->signal_saveprogress);
}

psy_audio_Song* psy_audio_song_alloc(void)
{
	return (psy_audio_Song*)malloc(sizeof(psy_audio_Song));
}

psy_audio_Song* psy_audio_song_allocinit(psy_audio_MachineFactory*
	machinefactory)
{
	psy_audio_Song* rv;

	rv = psy_audio_song_alloc();
	if (rv) {
		psy_audio_song_init(rv, machinefactory);
	}
	return rv;
}

void psy_audio_song_deallocate(psy_audio_Song* self)
{
	if (self) {
		psy_audio_song_dispose(self);
		free(self);
	}
}

void psy_audio_song_clear(psy_audio_Song* self)
{
	assert(self);

	psy_audio_sequence_clear(&self->sequence);
	psy_audio_patterns_clear(&self->patterns);
	psy_audio_machines_clear(&self->machines);
}

void psy_audio_song_setproperties(psy_audio_Song* self,
	const SongProperties* properties)
{	
	assert(self);

	songproperties_copy(&self->properties, properties);
}

void psy_audio_song_setbpm(psy_audio_Song* self, psy_dsp_big_beat_t bpm)
{
	assert(self);

	songproperties_setbpm(&self->properties, bpm);		
}

psy_dsp_big_beat_t psy_audio_song_bpm(psy_audio_Song* self)
{
	assert(self);

	return songproperties_bpm(&self->properties);
}

uintptr_t psy_audio_song_lpb(psy_audio_Song* self)
{
	assert(self);

	return songproperties_lpb(&self->properties);
}
