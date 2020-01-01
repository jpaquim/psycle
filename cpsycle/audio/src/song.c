// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "song.h"
#include <stdlib.h>
#include <string.h>

static void song_initproperties(psy_audio_Song*);
static void song_initmachines(psy_audio_Song*);
static void song_initpatterns(psy_audio_Song*);
static void song_initsequence(psy_audio_Song*);
static void song_initsignals(psy_audio_Song*);
static void song_disposesignals(psy_audio_Song*);

void songproperties_init(SongProperties* self, const char* title,
	const char* credits, const char* comments)
{
	self->title = strdup(title);
	self->credits = strdup(credits);
	self->comments = strdup(comments);
	self->bpm = (psy_dsp_beat_t) 125.0;
	self->lpb = 4;
	self->tracks = 16;
	self->octave = 4;
	self->tpb = 24;
	self->extraticksperbeat = 0;
}

void songproperties_dispose(SongProperties* self)
{
	free(self->title);
	self->title = 0;
	free(self->credits);
	self->credits = 0;
	free(self->comments);
	self->comments = 0;
}

void songproperties_copy(SongProperties* self, const SongProperties* other)
{
	if (self != other) {
		songproperties_dispose(self);
		self->title = strdup(other->title);
		self->credits = strdup(other->credits);
		self->comments = strdup(other->comments);
		self->bpm = other->bpm;
		self->lpb = other->lpb;
		self->octave = other->octave;
		self->tpb = other->tpb;
		self->extraticksperbeat = other->extraticksperbeat;
		self->tracks = other->tracks;
	}
}

void song_init(psy_audio_Song* self, MachineFactory* machinefactory)
{		
	self->machinefactory = machinefactory;	
	songproperties_init(&self->properties, "Untitled", "Unnamed",
		"No Comments");	
	song_initmachines(self);
	song_initpatterns(self);
	song_initsequence(self);
	samples_init(&self->samples);
	instruments_init(&self->instruments);
	song_initsignals(self);
}

void song_initmachines(psy_audio_Song* self)
{
	machines_init(&self->machines);
	machines_insertmaster(&self->machines,
		machinefactory_makemachine(self->machinefactory, MACH_MASTER, 0));
}

void song_initpatterns(psy_audio_Song* self)
{
	patterns_init(&self->patterns);
	patterns_insert(&self->patterns, 0, pattern_allocinit());
}

void song_initsequence(psy_audio_Song* self)
{
	SequencePosition sequenceposition;
	sequence_init(&self->sequence, &self->patterns);	
	sequenceposition.track = 
		sequence_appendtrack(&self->sequence, sequencetrack_allocinit());
	sequenceposition.trackposition =
		sequence_begin(&self->sequence, sequenceposition.track, 0);
	sequence_insert(&self->sequence, sequenceposition, 0);
}

void song_initsignals(psy_audio_Song* self)
{
	psy_signal_init(&self->signal_loadprogress);
	psy_signal_init(&self->signal_saveprogress);
}

void song_dispose(psy_audio_Song* self)
{
	songproperties_dispose(&self->properties);
	machines_dispose(&self->machines);
	patterns_dispose(&self->patterns);
	sequence_dispose(&self->sequence);		
	samples_dispose(&self->samples);
	instruments_dispose(&self->instruments);	
	song_disposesignals(self);
}

void song_disposesignals(psy_audio_Song* self)
{
	psy_signal_dispose(&self->signal_loadprogress);
	psy_signal_dispose(&self->signal_saveprogress);
}

psy_audio_Song* song_alloc(void)
{
	return (psy_audio_Song*) malloc(sizeof(psy_audio_Song));
}

psy_audio_Song* song_allocinit(MachineFactory* machinefactory)
{
	psy_audio_Song* rv;

	rv = song_alloc();
	if (rv) {
		song_init(rv, machinefactory);
	}
	return rv;
}

void song_free(psy_audio_Song* self)
{
	if (self) {
		song_dispose(self);
		free(self);
	}
}

void song_clear(psy_audio_Song* self)
{
	sequence_clear(&self->sequence);
	patterns_clear(&self->patterns);
	machines_clear(&self->machines);
}

void song_setproperties(psy_audio_Song* self, const SongProperties* properties)
{	
	songproperties_copy(&self->properties, properties);
}
