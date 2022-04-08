/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "song.h"
/* local */
#include "constants.h"
#include "machinefactory.h"
/* platform */
#include "../../detail/portable.h"

/* psy_audio_SongProperties */
/* prototypes */
static void song_initproperties(psy_audio_Song*);
static void song_initmachines(psy_audio_Song*);
static void song_initpatterns(psy_audio_Song*);
static void song_initsequence(psy_audio_Song*);
static void song_initsignals(psy_audio_Song*);
static void song_disposesignals(psy_audio_Song*);
/* implementation */
void psy_audio_songproperties_init(psy_audio_SongProperties* self,
	const char* title,
	const char* credits,
	const char* comments)
{
	assert(self);

	self->title = psy_strdup(title);
	self->credits = psy_strdup(credits);
	self->comments = psy_strdup(comments);
	self->bpm = (psy_dsp_big_beat_t) 125.0;
	self->lpb = 4;	
	self->octave = 4;
	self->tpb = 24;
	self->extraticksperbeat = 0;
	self->samplerindex = 0x3E;
}

void psy_audio_songproperties_init_all(psy_audio_SongProperties* self,
	const char* title, const char* credits, const char* comments,
	int octave,
	uintptr_t lpb,
	int tpb,
	int extraticksperbeat,
	psy_dsp_big_beat_t bpm,
	uintptr_t sampler)
{
	assert(self);

	self->title = psy_strdup(title);
	self->credits = psy_strdup(credits);
	self->comments = psy_strdup(comments);
	self->lpb = lpb;	
	self->octave = octave;
	self->tpb = tpb;
	self->extraticksperbeat = extraticksperbeat;
	self->bpm = bpm;
	self->samplerindex = sampler;
}

void psy_audio_songproperties_dispose(psy_audio_SongProperties* self)
{
	assert(self);

	free(self->title);
	self->title = NULL;
	free(self->credits);
	self->credits = NULL;
	free(self->comments);
	self->comments = NULL;
}

void psy_audio_songproperties_copy(psy_audio_SongProperties* self, const psy_audio_SongProperties* other)
{
	assert(self);

	if (self != other) {
		psy_audio_songproperties_dispose(self);
		self->title = psy_strdup(other->title);
		self->credits = psy_strdup(other->credits);
		self->comments = psy_strdup(other->comments);
		self->bpm = other->bpm;
		self->lpb = other->lpb;
		self->octave = other->octave;
		self->tpb = other->tpb;
		self->extraticksperbeat = other->extraticksperbeat;		
	}
}

// Properties
void psy_audio_songproperties_settitle(psy_audio_SongProperties* self,
	const char* title)
{
	psy_strreset(&self->title, title);
}

void psy_audio_songproperties_setcomments(psy_audio_SongProperties* self,
	const char* comments)
{
	psy_strreset(&self->comments, comments);
}

void psy_audio_songproperties_setcredits(psy_audio_SongProperties* self,
	const char* credits)
{
	psy_strreset(&self->credits, credits);
}

/* psy_audio_Song */
void psy_audio_song_init(psy_audio_Song* self, psy_audio_MachineFactory*
	machinefactory)
{
	assert(self);

	self->machinefactory = machinefactory;
	psy_audio_songproperties_init(&self->properties, "Untitled", "Unnamed",
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
		psy_audio_machinefactory_makemachine(self->machinefactory,
			psy_audio_MASTER, NULL, psy_INDEX_INVALID));
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
	assert(self);

	psy_audio_sequence_init(&self->sequence, &self->patterns, &self->samples);		
	psy_audio_sequence_appendtrack(&self->sequence,
		psy_audio_sequencetrack_allocinit());		
	psy_audio_sequence_insert(&self->sequence,
		psy_audio_orderindex_make(0, 0), 0);
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

	psy_audio_songproperties_dispose(&self->properties);
	psy_audio_machines_dispose(&self->machines);
	psy_audio_sequence_dispose(&self->sequence);
	psy_audio_patterns_dispose(&self->patterns);	
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
	assert(self);
	
	psy_audio_song_dispose(self);
	free(self);	
}

void psy_audio_song_clear(psy_audio_Song* self)
{
	assert(self);

	psy_audio_sequence_clear(&self->sequence);
	psy_audio_patterns_clear(&self->patterns);
	psy_audio_machines_clear(&self->machines);
}

void psy_audio_song_setproperties(psy_audio_Song* self,
	const psy_audio_SongProperties* properties)
{	
	assert(self);

	psy_audio_songproperties_copy(&self->properties, properties);
}

void psy_audio_song_setbpm(psy_audio_Song* self, psy_dsp_big_beat_t bpm)
{
	assert(self);

	psy_audio_songproperties_setbpm(&self->properties, bpm);		
}

void psy_audio_song_insertvirtualgenerator(psy_audio_Song* self,
	uintptr_t virtual_inst, uintptr_t mac_idx, uintptr_t inst_idx)
{
	assert(self);

	/*
	** && mac != NULL && (mac->_type == psy_audio_SAMPLER ||
	**    mac->_type == psy_audio_XMSAMPLER))
	*/
	if (virtual_inst >= MAX_MACHINES && virtual_inst < MAX_VIRTUALINSTS) {
		psy_audio_Machine* machine;
		
		machine = psy_audio_machines_at(&self->machines, virtual_inst);
		if (machine) {
			psy_audio_machines_remove(&self->machines, virtual_inst, FALSE);
		}
		machine = psy_audio_machinefactory_makemachinefrompath(
			self->machinefactory, psy_audio_VIRTUALGENERATOR, NULL,
			mac_idx, inst_idx);
		if (machine) {
			psy_audio_machine_seteditname(machine, "Virtual Generator");
			psy_audio_machines_insert(&self->machines, virtual_inst, machine);
		}
	}
}
