// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songio.h"

#include "psy2.h"
#include "psy3.h"
#include "song.h"
#include "machinefactory.h"
#include "wavsongio.h"
#include "xmdefs.h"
#include "xm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined DIVERSALIS__OS__POSIX
#include <errno.h>
#endif

static void psy_audio_songfile_createmaster(psy_audio_SongFile*);
static int psy_audio_songfile_errfile(psy_audio_SongFile* self);
static void psy_audio_songfile_machineuis_dispose(psy_audio_SongFile*);

void psy_audio_songfile_init(psy_audio_SongFile* self)
{
	psy_signal_init(&self->signal_output);
	psy_signal_init(&self->signal_warning);
	self->machinesoloed = -1;
	psy_table_init(&self->machineuis);
}

void psy_audio_songfile_dispose(psy_audio_SongFile* self)
{
	psy_signal_dispose(&self->signal_output);
	psy_signal_dispose(&self->signal_warning);	
	psy_audio_songfile_machineuis_dispose(self);
}

void psy_audio_songfile_machineuis_dispose(psy_audio_SongFile* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->machineuis);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_MachineUi* machineui;

		machineui = (psy_audio_MachineUi*)psy_tableiterator_value(&it);
		free(machineui);
	}
	psy_table_dispose(&self->machineuis);
}

int psy_audio_songfile_load(psy_audio_SongFile* self, const char* path)
{		
	PsyFile file;
	int status;

	status = PSY_OK;
	self->serr = 0;
	self->err = 0;
	self->warnings = 0;
	self->file = &file;
	self->path = path;	
	self->machinesoloed = -1;
	if (psy_table_size(&self->machineuis) > 0) {
		psy_audio_songfile_machineuis_dispose(self);
		psy_table_init(&self->machineuis);
	}
	psy_audio_songfile_message(self, "searching for ");
	psy_audio_songfile_message(self, path);
	psy_audio_songfile_message(self, "\n");
	if (psyfile_open(self->file, path)) {
		char header[20];
		char riff[5];
		
		psy_audio_songfile_message(self, "loading ");
		psy_audio_songfile_message(self, path);
		psy_audio_songfile_message(self, "\n");
		psy_audio_song_clear(self->song);		
		machines_startfilemode(&self->song->machines);
		psyfile_read(self->file, header, 8);
		header[8] = '\0';
		strncpy(riff, header, 4);
		riff[4] = '\0';
		psy_signal_emit(&self->song->signal_loadprogress, self->song, 1, 1);
		if (strcmp(header,"PSY3SONG") == 0) {
			if (status = psy_audio_psy3_load(self)) {
				psy_audio_songfile_errfile(self);
			}
			psyfile_close(self->file);
		} else
		if (strcmp(header,"PSY2SONG") == 0) {
			psy_audio_psy2_load(self);
			psyfile_close(self->file);
		} else 
		if (strcmp(riff, "RIFF") == 0) {			
			psyfile_read(&file, header, 8);
			header[8] = 0;
			if (strcmp(&header[0], "WAVEfmt ") == 0) {
				psyfile_close(self->file);		
				psy_audio_wav_songio_load(self);
			}
		} else {
			psyfile_read(self->file, header + 8, strlen(XM_HEADER) - 8);
			header[strlen(XM_HEADER)] = '\0';
			if (strcmp(header, XM_HEADER) == 0) {
				psy_audio_xm_load(self);
			} else {
				self->err = 2;
			}
			psyfile_close(self->file);
		}
		if (!machines_at(&self->song->machines, MASTER_INDEX)) {
			psy_audio_songfile_createmaster(self);
		}				
		psy_audio_machines_solo(&self->song->machines,
			self->machinesoloed);
		machines_endfilemode(&self->song->machines);		
	} else {
		status = psy_audio_songfile_errfile(self);
	}
	psy_signal_emit(&self->song->signal_loadprogress, self->song, 1, 0);
	return status;
}

int psy_audio_songfile_save(psy_audio_SongFile* self, const char* path)
{		
	int status;
	PsyFile file;

	status = PSY_OK;
	self->file = &file;
	self->err = 0;
	self->warnings = 0;
	self->path = path;
	if (psyfile_create(self->file, path, 1)) {				
		if (status = psy_audio_psy3_save(self)) {
			psy_audio_songfile_errfile(self);
		}
		psyfile_close(self->file);
	} else {
		return psy_audio_songfile_errfile(self);
	}
	return status;
}

void psy_audio_songfile_message(psy_audio_SongFile* self, const char* text)
{
	psy_signal_emit(&self->signal_output, self, 1, text);
}

void psy_audio_songfile_warn(psy_audio_SongFile* self, const char* text)
{
	psy_signal_emit(&self->signal_warning, self, 1, text);
}

void psy_audio_songfile_createmaster(psy_audio_SongFile* self)
{
	psy_audio_MachineUi* machineui;	

	machines_insertmaster(&self->song->machines,
		machinefactory_makemachine(self->song->machinefactory,
		MACH_MASTER, 0));	
	machineui = psy_audio_songfile_machineui(self, MASTER_INDEX);
	machineui->x = 320;
	machineui->y = 200;
}

int psy_audio_songfile_errfile(psy_audio_SongFile* self)
{
	self->err = errno;
	self->serr = strerror(self->err);
	return PSY_ERRFILE;
}

psy_audio_MachineUi* psy_audio_songfile_machineui(psy_audio_SongFile* self,
	uintptr_t index)
{
	psy_audio_MachineUi* rv;

	if (!psy_table_exists(&self->machineuis, index)) {
		rv = (psy_audio_MachineUi*)malloc(sizeof(psy_audio_MachineUi));
		if (rv) {
			memset(rv, 0, sizeof(psy_audio_MachineUi));		
			psy_table_insert(&self->machineuis, index, rv);
		}
	} else {
		rv = psy_table_at(&self->machineuis, index);
	}
	return rv;
}
