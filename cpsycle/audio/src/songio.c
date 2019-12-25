// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songio.h"

#include "psy2.h"
#include "psy3.h"
#include "xm.h"
#include "xmdefs.h"

#include <string.h>

static void songfile_createmaster(psy_audio_SongFile*);

void psy_audio_songfile_init(psy_audio_SongFile* self)
{
	psy_signal_init(&self->signal_output);
	psy_signal_init(&self->signal_warning);	
	psy_signal_init(&self->signal_error);
}

void psy_audio_songfile_dispose(psy_audio_SongFile* self)
{
	psy_signal_dispose(&self->signal_output);
	psy_signal_dispose(&self->signal_warning);
	psy_signal_dispose(&self->signal_error);
}

void psy_audio_songfile_load(psy_audio_SongFile* self, const char* path)
{		
	PsyFile file;

	self->err = 0;
	self->warnings = 0;
	self->file = &file;	
	
	psy_audio_songfile_message(self, "searching for ");
	psy_audio_songfile_message(self, path);
	psy_audio_songfile_message(self, "\n");
	if (psyfile_open(self->file, path)) {
		char header[20];
//		SequencePosition position;
		
		psy_audio_songfile_message(self, "loading ");
		psy_audio_songfile_message(self, path);
		psy_audio_songfile_message(self, "\n");
		self->workspaceproperties = psy_properties_create();
		song_clear(self->song);		
		machines_startfilemode(&self->song->machines);
		psyfile_read(self->file, header, 8);
		header[8] = '\0';
		psy_signal_emit(&self->song->signal_loadprogress, self->song, 1, 1);
		if (strcmp(header,"PSY3SONG") == 0) {
			psy3_load(self);
		} else
		if (strcmp(header,"PSY2SONG") == 0) {
			psy2_load(self);
		} else {
			psyfile_read(self->file, header + 8, strlen(XM_HEADER) - 8);
			header[strlen(XM_HEADER)] = '\0';
			if (strcmp(header, XM_HEADER) == 0) {
				xm_load(self);
			} else {
				self->err = 2;
			}
		}
		if (!machines_at(&self->song->machines, MASTER_INDEX)) {
			songfile_createmaster(self);
		}		
		machines_endfilemode(&self->song->machines);
		psyfile_close(self->file);
		psy_audio_songfile_message(self, "ready\n");
	} else {
		psy_audio_songfile_error(self, "file not open error\n");		
		self->err = 1;
	}
	psy_signal_emit(&self->song->signal_loadprogress, self->song, 1, 0);
}

void psy_audio_songfile_save(psy_audio_SongFile* self, const char* path)
{	
	PsyFile file;

	self->file = &file;
	if (psyfile_create(self->file, path, 1)) {		
		psy3_save(self);
		psyfile_close(self->file);
	}
}

void psy_audio_songfile_message(psy_audio_SongFile* self, const char* text)
{
	psy_signal_emit(&self->signal_output, self, 1, text);
}

void psy_audio_songfile_warn(psy_audio_SongFile* self, const char* text)
{
	psy_signal_emit(&self->signal_warning, self, 1, text);
}

void psy_audio_songfile_error(psy_audio_SongFile* self, const char* text)
{
	psy_signal_emit(&self->signal_error, self, 1, text);
}

void songfile_createmaster(psy_audio_SongFile* self)
{
	psy_Properties* machines;	
	psy_Properties* machine;

	machines_insertmaster(&self->song->machines,
			machinefactory_makemachine(self->song->machinefactory,
			MACH_MASTER, 0));
	machines = psy_properties_findsection(self->workspaceproperties,
		"machines");
	if (!machines) {
		machines = psy_properties_create_section(
			self->workspaceproperties, "machines");
	}
	machine = psy_properties_create_section(machines, "machine");
	psy_properties_append_int(machine, "index", MASTER_INDEX, 0, 0);		
	psy_properties_append_int(machine, "x", 320, 0, 0);
	psy_properties_append_int(machine, "y", 200, 0, 0);
}

