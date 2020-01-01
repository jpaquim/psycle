// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGIO_H)
#define SONGIO_H

#include <properties.h>
#include "song.h"
#include <fileio.h>

/// song load and save
typedef struct psy_audio_SongFile {
	struct psy_audio_Song* song;
	struct PsyFile* file;
	psy_Properties* workspaceproperties;
	int err;
	const char *serr;
	int warnings;
	psy_Signal signal_output;
	psy_Signal signal_warning;	
	const char* path;
} psy_audio_SongFile;

void psy_audio_songfile_init(psy_audio_SongFile*);
void psy_audio_songfile_dispose(psy_audio_SongFile*);
int psy_audio_songfile_load(psy_audio_SongFile*, const char* path);
int psy_audio_songfile_save(psy_audio_SongFile*, const char* path);
void psy_audio_songfile_warn(psy_audio_SongFile*, const char* text);
void psy_audio_songfile_message(psy_audio_SongFile*, const char* text);

#endif
