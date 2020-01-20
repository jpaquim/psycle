// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SONGIO_H
#define psy_audio_SONGIO_H

#include <properties.h>
#include <fileio.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

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

void psy_audio_songfile_init(struct psy_audio_SongFile*);
void psy_audio_songfile_dispose(struct psy_audio_SongFile*);
int psy_audio_songfile_load(struct psy_audio_SongFile*, const char* path);
int psy_audio_songfile_save(struct psy_audio_SongFile*, const char* path);
void psy_audio_songfile_warn(struct psy_audio_SongFile*, const char* text);
void psy_audio_songfile_message(struct psy_audio_SongFile*, const char* text);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SONGIO_H */
