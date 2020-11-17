// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songio.h"

#include "psy3loader.h"
#include "psy3saver.h"
#include "psy2.h"
#include "song.h"
#include "machinefactory.h"
#include "wavsongio.h"
#include "wire.h"
#include "xmdefs.h"
#include "xm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined DIVERSALIS__OS__POSIX
#include <errno.h>
#endif

static char load_filter[] =
"All Songs (*.psy *.xm *.it *.s3m *.mod *.wav)" "|*.psy;*.xm;*.it;*.s3m;*.mod;*.wav|"
"Songs (*.psy)"				        "|*.psy|"
"FastTracker II Songs (*.xm)"       "|*.xm|"
"Impulse Tracker Songs (*.it)"      "|*.it|"
"Scream Tracker Songs (*.s3m)"      "|*.s3m|"
"Original Mod Format Songs (*.mod)" "|*.mod|"
"Wav Format Songs (*.wav)"			"|*.wav";

static char save_filter[] =
"Songs (*.psy)|*.psy";

static void psy_audio_songfile_createmaster(psy_audio_SongFile*);
static int psy_audio_songfile_errfile(psy_audio_SongFile* self);

void psy_audio_songfile_init(psy_audio_SongFile* self)
{
	psy_signal_init(&self->signal_output);
	psy_signal_init(&self->signal_warning);
	self->machinesoloed = -1;
	psy_audio_legacywires_init(&self->legacywires);
}

void psy_audio_songfile_dispose(psy_audio_SongFile* self)
{
	psy_signal_dispose(&self->signal_output);
	psy_signal_dispose(&self->signal_warning);		
	psy_audio_legacywires_dispose(&self->legacywires);
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
		psy_audio_machines_startfilemode(&self->song->machines);
		psyfile_read(self->file, header, 8);
		header[8] = '\0';
		strncpy(riff, header, 4);
		riff[4] = '\0';
		psy_signal_emit(&self->song->signal_loadprogress, self->song, 1, 1);
		if (strcmp(header,"PSY3SONG") == 0) {
#ifdef PSYCLE_USE_PSY3
			psy_audio_PSY3Loader psy3loader;

			psy_audio_psy3loader_init(&psy3loader, self);
			if (status = psy_audio_psy3loader_load(&psy3loader)) {
				psy_audio_songfile_errfile(self);
			}
#else
			status = PSY_ERRFILEFORMAT;
#endif
			psyfile_close(self->file);
		} else if (strcmp(header,"PSY2SONG") == 0) {
			// PSY2SONG is the Sign of the file.
#ifdef PSYCLE_USE_PSY2
			psy_audio_psy2_load(self);
#else
			status = PSY_ERRFILEFORMAT;
#endif
			psyfile_close(self->file);
		} else if (strcmp(riff, "RIFF") == 0) {			
			psyfile_read(&file, header, 8);
			header[8] = 0;
			if (strcmp(&header[0], "WAVEfmt ") == 0) {
				psyfile_close(self->file);		
				psy_audio_wav_songio_load(self);
			}
		} else {
#ifdef PSYCLE_USE_XM
			psyfile_read(self->file, header + 8, strlen(XM_HEADER) - 8);
			header[strlen(XM_HEADER)] = '\0';
			if (strcmp(header, XM_HEADER) == 0) {
				psy_audio_xm_load(self);
			} else {
				psyfile_seek(self->file, 0);
				if (psy_audio_mod_isvalid(self)) {
					psyfile_seek(self->file, 0);
					psy_audio_mod_load(self);
				} else {
					self->err = 2;
				}
			}
#else
			status = PSY_ERRFILEFORMAT;
#endif
			psyfile_close(self->file);
		}
		if (!psy_audio_machines_at(&self->song->machines, psy_audio_MASTER_INDEX)) {
			psy_audio_songfile_createmaster(self);
		}				
		psy_audio_machines_solo(&self->song->machines,
			self->machinesoloed);		
		{
			psy_TableIterator it;
			// notify machines postload	
			for (it = psy_audio_machines_begin(&self->song->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_Machine* machine;

				machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
				psy_audio_machine_postload(machine, self, psy_tableiterator_key(&it));
			}
		}
		psy_audio_machines_endfilemode(&self->song->machines);		
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
		psy_audio_PSY3Saver psy3saver;

		psy_audio_psy3saver_init(&psy3saver, self);
		if (status = psy_audio_psy3saver_save(&psy3saver)) {
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
	psy_audio_machines_insertmaster(&self->song->machines,
		psy_audio_machinefactory_makemachine(self->song->machinefactory,
		MACH_MASTER, 0));	
}

int psy_audio_songfile_errfile(psy_audio_SongFile* self)
{
	self->err = errno;
	self->serr = strerror(self->err);
	return PSY_ERRFILE;
}

const char* psy_audio_songfile_loadfilter(void)
{
	return load_filter;
}

const char* psy_audio_songfile_savefilter(void)
{
	return save_filter;
}
