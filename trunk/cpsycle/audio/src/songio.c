// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songio.h"
// local
#include "exs24loader.h"
#include "itmodule2.h"
#include "machinefactory.h"
#include "midiloader.h"
#include "midisongexport.h"
#include "psy3loader.h"
#include "psy3saver.h"
#include "psy2.h"
#include "song.h"
#include "wavsongio.h"
#include "wire.h"
#include "xmdefs.h"
#include "xmsongexport.h"
#include "xmsongloader.h"
// platform
#include "../../detail/portable.h"

#if defined DIVERSALIS__OS__POSIX
#include <errno.h>
#endif

// song/module filter
static char load_filters[] =
"All Songs (*.psy *.xm *.it *.s3m *.mod *.wav *.mid)" "|*.psy;*.xm;*.it;*.s3m;*.mod;*.wav;*.mid|"
"Songs (*.psy)"				        "|*.psy|"
"FastTracker II Songs (*.xm)"       "|*.xm|"
"Impulse Tracker Songs (*.it)"      "|*.it|"
"Scream Tracker Songs (*.s3m)"      "|*.s3m|"
"Original Mod Format Songs (*.mod)" "|*.mod|"
"Wav Format Songs (*.wav)"			"|*.wav|"
"Midi Standard Songs (*.mid)"	    "|*.mid|";
static const char save_filters[] = "Songs (*.psy)|*.psy";
// instrument filter
static const char instloadfilter[] = "All Instruments and samples|*.psins;*.xi;*.iti;*.wav;*.aif;*.aiff;*.its;*.s3i;*.8svx;*.16sv;*.svx;*.iff;*.exs|Psycle Instrument (*.psins)|*.psins|XM Instruments (*.xi)|*.xi|IT Instruments (*.iti)|*.iti|Wav (PCM) Files (*.wav)|*.wav|Apple AIFF (PCM) Files (*.aif)|*.aif;*.aiff|ST3 Samples (*.s3i)|*.s3i|IT2 Samples (*.its)|*.its|Amiga IFF/SVX Samples (*.svx)|*.8svx;*.16sv;*.svx;*.iff;*.exs|All files (*.*)|*.*||";
static const char instsavefilter[] = "Instrument (*.psins)|*.psins";

void psy_audio_songfile_init(psy_audio_SongFile* self)
{
	psy_signal_init(&self->signal_output);
	psy_signal_init(&self->signal_warning);
	self->machinesoloed = -1;
	self->legacywires = NULL;
	self->path = NULL;	
}

void psy_audio_songfile_dispose(psy_audio_SongFile* self)
{
	free(self->path);
	self->path = NULL;
	psy_signal_dispose(&self->signal_output);
	psy_signal_dispose(&self->signal_warning);		
}

int psy_audio_songfile_load(psy_audio_SongFile* self, const char* filename)
{		
	PsyFile file;
	int status;

	status = PSY_OK;
	self->serr = 0;
	self->err = 0;
	self->warnings = 0;
	self->file = &file;	
	psy_strreset(&self->path, filename);
	self->machinesoloed = -1;	
	psy_audio_songfile_message(self, "searching for ");
	psy_audio_songfile_message(self, self->path);
	psy_audio_songfile_message(self, "\n");
	if (psyfile_open(self->file, filename)) {
		char header[20];
		char riff[5];
		
		psy_audio_songfile_message(self, "loading ");
		psy_audio_songfile_message(self, self->path);
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
			psy_audio_psy3loader_dispose(&psy3loader);
#else
			status = PSY_ERRFILEFORMAT;
#endif			
		} else if (strcmp(header,"PSY2SONG") == 0) {
			// PSY2SONG is the Sign of the file.
#ifdef PSYCLE_USE_PSY2
			PSY2Loader psy2loader;

			psy2loader_init(&psy2loader, self);
			psy2loader_load(&psy2loader);
			psy2loader_dispose(&psy2loader);
#else
			status = PSY_ERRFILEFORMAT;
#endif			
		} else if (strcmp(riff, "RIFF") == 0) {			
			psyfile_read(&file, header, 8);
			header[8] = 0;
			if (strcmp(&header[0], "WAVEfmt ") == 0) {
				psyfile_close(self->file);		
				psy_audio_wav_songio_load(self);
			}
		} else if (strcmp(riff, "IMPM") == 0) {
			ITModule2 itmodule;

			psyfile_seek(self->file, 0);
			itmodule2_init(&itmodule, self);
			if (status = itmodule2_load(&itmodule)) {
				psy_audio_songfile_errfile(self);
			}
			itmodule2_dispose(&itmodule);			
		} else if (strcmp(riff, "MThd") == 0) {
			MidiLoader midifile;

			psyfile_seek(self->file, 0);
			midiloader_init(&midifile, self);			
			if (status = midiloader_load(&midifile)) {			
				psy_audio_songfile_errfile(self);
			}
			midiloader_dispose(&midifile);			
		} else {
#ifdef PSYCLE_USE_XM
			psyfile_read(self->file, header + 8, (uint32_t)(strlen(XM_HEADER) - 8));
			header[strlen(XM_HEADER)] = '\0';
			if (strcmp(header, XM_HEADER) == 0) {
				XMSongLoader loader;

				psyfile_seek(self->file, 0);
				xmsongloader_init(&loader, self);
				if (status = xmsongloader_load(&loader)) {
					psy_audio_songfile_errfile(self);
				}
				xmsongloader_dispose(&loader);				
			} else {
				MODSongLoader loader;

				psyfile_seek(self->file, 0);
				modsongloader_init(&loader, self);
				if (status = modsongloader_load(&loader)) {
					psy_audio_songfile_errfile(self);
				}				
				modsongloader_dispose(&loader);				
			}
#else
			status = PSY_ERRFILEFORMAT;
#endif			
		}
		psyfile_close(self->file);
		if (!psy_audio_machines_at(&self->song->machines, psy_audio_MASTER_INDEX)) {
			psy_audio_songfile_createmaster(self);
		}
		psy_audio_machines_solo(&self->song->machines,
			self->machinesoloed);
		psy_audio_machines_endfilemode(&self->song->machines);		
	} else {
		status = psy_audio_songfile_errfile(self);
	}
	psy_signal_emit(&self->song->signal_loadprogress, self->song, 1, 0);
	return status;
}

int psy_audio_songfile_save(psy_audio_SongFile* self, const char* filename)
{		
	int status;
	PsyFile file;	

	status = PSY_OK;
	self->file = &file;
	self->err = 0;
	self->warnings = 0;
	psy_strreset(&self->path, filename);	
	if (psyfile_create(self->file, self->path, 1)) {
		psy_audio_PSY3Saver psy3saver;

		psy_audio_psy3saver_init(&psy3saver, self);
		if (status = psy_audio_psy3saver_save(&psy3saver)) {
			psy_audio_songfile_errfile(self);
		}
		psy_audio_psy3saver_dispose(&psy3saver);
		psyfile_close(self->file);
	} else {
		return psy_audio_songfile_errfile(self);
	}
	return status;
}

int psy_audio_songfile_exportmodule(psy_audio_SongFile* self, const char* filename)
{
	int status;
	PsyFile file;

	status = PSY_OK;
	self->file = &file;
	self->err = 0;
	self->warnings = 0;
	psy_strreset(&self->path, filename);
	if (psyfile_create(self->file, self->path, 1)) {
		XMSongExport moduleexport;

		xmsongexport_init(&moduleexport, self);
		if (status = xmsongexport_exportsong(&moduleexport)) {
			psy_audio_songfile_errfile(self);
		}
		xmsongexport_dispose(&moduleexport);
		psyfile_close(self->file);
	} else {
		return psy_audio_songfile_errfile(self);
	}
	return status;
}

int psy_audio_songfile_exportmidifile(psy_audio_SongFile* self, const char* filename)
{
	int status;
	PsyFile file;

	status = PSY_OK;
	self->file = &file;
	self->err = 0;
	self->warnings = 0;
	psy_strreset(&self->path, filename);
	if (psyfile_create(self->file, self->path, 1)) {
		psy_audio_MidiSongExport moduleexport;

		psy_audio_midisongexport_init(&moduleexport, self);
		if (status = psy_audio_midisongexport_save(&moduleexport)) {
			psy_audio_songfile_errfile(self);
		}
		psy_audio_midisongexport_dispose(&moduleexport);
		psyfile_close(self->file);
	} else {
		return psy_audio_songfile_errfile(self);
	}
	return status;
}

int psy_audio_songfile_saveinstrument(psy_audio_SongFile* self, const char* filename,
	psy_audio_Instrument* instr)
{
	int status;
	PsyFile file;

	status = PSY_OK;
	self->file = &file;
	self->err = 0;
	self->warnings = 0;
	psy_strreset(&self->path, filename);
	if (psyfile_create(self->file, self->path, 1)) {
		psy_audio_PSY3Saver psy3saver;

		psy_audio_psy3saver_init(&psy3saver, self);
		if (psy_audio_psy3saver_saveinstrument(&psy3saver, instr)) {
			psy_audio_songfile_errfile(self);
		}
		psy_audio_psy3saver_dispose(&psy3saver);
		psyfile_close(self->file);
	} else {
		return psy_audio_songfile_errfile(self);
	}
	return status;
}

int psy_audio_songfile_loadinstrument(psy_audio_SongFile* self, const char* filename,
	psy_audio_InstrumentIndex index)
{	
	int status;
	psy_Path path;

	status = PSY_ERRFILE;
	psy_path_init(&path, filename);
	if (strcmp(psy_path_ext(&path), "xi") == 0) {
		XMSongLoader xmsongloader;

		xmsongloader_init(&xmsongloader, self);
		if (status = xmsongloader_loadxi(&xmsongloader, index)) {
			psy_audio_songfile_errfile(self);
		}
		xmsongloader_dispose(&xmsongloader);
	} else if (strcmp(psy_path_ext(&path), "psins") == 0) {
		psy_audio_PSY3Loader psy3songloader;

		psy_audio_psy3loader_init(&psy3songloader, self);
		if (status = psy_audio_psy3loader_loadpsins(&psy3songloader, index)) {
			psy_audio_songfile_errfile(self);
		}
		psy_audio_psy3loader_dispose(&psy3songloader);
	} else if (strcmp(psy_path_ext(&path), "exs") == 0) {
		psy_audio_EXS24Loader instloader;

		psy_audio_exs24loader_init(&instloader, self);
		if (status = psy_audio_exs24loader_load(&instloader)) {
			psy_audio_songfile_errfile(self);
		}
		psy_audio_exs24loader_dispose(&instloader);
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
		psy_audio_MASTER, NULL, UINTPTR_MAX));
}

int psy_audio_songfile_errfile(psy_audio_SongFile* self)
{
	self->err = errno;
	self->serr = strerror(self->err);
	return PSY_ERRFILE;
}

const char* psy_audio_songfile_loadfilter(void)
{
	return load_filters;
}

const char* psy_audio_songfile_standardloadfilter(void)
{
	return PSYCLE_DEFAULT_LOAD_EXTENSION;
}

const char* psy_audio_songfile_savefilter(void)
{
	return save_filters;
}

const char* psy_audio_songfile_standardsavefilter(void)
{
	return PSYCLE_DEFAULT_SAVE_EXTENSION;
}

const char* psy_audio_songfile_instloadfilter(void)
{
	return instloadfilter;
}

const char* psy_audio_songfile_standardinstloadfilter(void)
{
	return "psins";
}

const char* psy_audio_songfile_instsavefilter(void)
{
	return instsavefilter;
}

const char* psy_audio_songfile_standardinstsavefilter(void)
{
	return "psins";
}

