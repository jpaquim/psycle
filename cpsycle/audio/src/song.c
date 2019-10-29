// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "song.h"
#include "psy2.h"
#include "psy3.h"
#include <fileio.h>
#include <stdlib.h>
#include <string.h>

static void song_initproperties(Song*);
static void song_initmachines(Song*);
static void song_initpatterns(Song*);
static void song_initsequence(Song*);
static void song_initsignals(Song*);
static void song_disposesignals(Song*);

void song_init(Song* self, MachineFactory* machinefactory)
{		
	self->machinefactory = machinefactory;	
	song_initproperties(self);
	song_initmachines(self);
	song_initpatterns(self);
	song_initsequence(self);
	samples_init(&self->samples);
	instruments_init(&self->instruments);
	xminstruments_init(&self->xminstruments);
	song_initsignals(self);
}

void song_initproperties(Song* self)
{
	self->properties = properties_create();
	properties_append_string(self->properties, "title", "Untitled");	
	properties_append_string(self->properties, "credits", "Unnamed");
	properties_append_string(self->properties, "comments", "No Comments");
	properties_append_double(self->properties, "bpm", 125.0, 0, 999);
	properties_append_int(self->properties, "lpb", 4, 1, 999);
	properties_append_int(self->properties, "tracks", 16, 1, 64);
}

void song_initmachines(Song* self)
{
	machines_init(&self->machines);
	machines_insertmaster(&self->machines,
		machinefactory_make(self->machinefactory, MACH_MASTER, 0));
}

void song_initpatterns(Song* self)
{
	patterns_init(&self->patterns);
	patterns_insert(&self->patterns, 0, pattern_allocinit());
}

void song_initsequence(Song* self)
{
	SequencePosition sequenceposition;
	sequence_init(&self->sequence, &self->patterns);	
	sequenceposition.track = 
		sequence_appendtrack(&self->sequence, sequencetrack_allocinit());
	sequenceposition.trackposition =
		sequence_begin(&self->sequence, sequenceposition.track, 0);
	sequence_insert(&self->sequence, sequenceposition, 0);
}

void song_initsignals(Song* self)
{
	signal_init(&self->signal_loadprogress);
	signal_init(&self->signal_saveprogress);
}

void song_dispose(Song* self)
{
	properties_free(self->properties);
	self->properties = 0;	
	machines_dispose(&self->machines);
	patterns_dispose(&self->patterns);
	sequence_dispose(&self->sequence);		
	samples_dispose(&self->samples);
	instruments_dispose(&self->instruments);
	xminstruments_dispose(&self->xminstruments);	
	song_disposesignals(self);
}

void song_disposesignals(Song* self)
{
	signal_dispose(&self->signal_loadprogress);
	signal_dispose(&self->signal_saveprogress);
}

Song* song_alloc(void)
{
	return (Song*) malloc(sizeof(Song));
}

Song* song_allocinit(MachineFactory* machinefactory)
{
	Song* rv;

	rv = song_alloc();
	if (rv) {
		song_init(rv, machinefactory);
	}
	return rv;
}

void song_free(Song* self)
{
	if (self) {
		song_dispose(self);
		free(self);
	}
}

void song_load(Song* self, const char* path, Properties** workspaceproperties)
{	
	PsyFile file;

	if (psyfile_open(&file, path)) {
		char header[9];				

		*workspaceproperties = 0;
		sequence_clear(&self->sequence);		
		patterns_clear(&self->patterns);
		machines_startfilemode(&self->machines);
		psyfile_read(&file, header, 8);
		header[8] = '\0';
		*workspaceproperties = properties_create();
		signal_emit(&self->signal_loadprogress, self, 1, 1);
		if (strcmp(header,"PSY3SONG")==0) {						
			psy3_load(self, &file, header, *workspaceproperties);			
		} else
		if (strcmp(header,"PSY2SONG")==0) {
			psy2_load(self, &file, header, *workspaceproperties);
		} else {

		}
		machines_endfilemode(&self->machines);
		psyfile_close(&file);
		signal_emit(&self->signal_loadprogress, self, 1, 0);
	}
}

void song_save(Song* self, const char* path, Properties* workspaceproperties)
{
	PsyFile file;

	if (psyfile_create(&file, path, 1)) {
		psy3_save(self, &file, workspaceproperties);
	}
}
