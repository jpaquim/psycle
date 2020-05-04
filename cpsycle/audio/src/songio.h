// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SONGIO_H
#define psy_audio_SONGIO_H

#include "wire.h"

#include <properties.h>
#include <fileio.h>
#include <signal.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int x;
	int y;
} psy_audio_MachineUi;

/// song load and save
typedef struct psy_audio_SongFile {
	struct psy_audio_Song* song;
	struct PsyFile* file;
	int err;
	const char *serr;
	int warnings;
	psy_Signal signal_output;
	psy_Signal signal_warning;	
	const char* path;
	uintptr_t machinesoloed;
	psy_Table machineuis;
	// MachineID X [psy_Table* LegacyWires (ConnectionID X LegacyWire*)]
	psy_audio_LegacyWires legacywires;
} psy_audio_SongFile;

void psy_audio_songfile_init(psy_audio_SongFile*);
void psy_audio_songfile_dispose(psy_audio_SongFile*);
int psy_audio_songfile_load(psy_audio_SongFile*, const char* path);
int psy_audio_songfile_save(psy_audio_SongFile*, const char* path);
void psy_audio_songfile_warn(psy_audio_SongFile*, const char* text);
void psy_audio_songfile_message(psy_audio_SongFile*, const char* text);
psy_audio_MachineUi* psy_audio_songfile_machineui(psy_audio_SongFile*,
	uintptr_t index);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SONGIO_H */
