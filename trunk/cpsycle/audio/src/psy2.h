// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSY2_H
#define psy_audio_PSY2_H

// local
#include "constants.h"
#include "machines.h"
#include "psy2converter.h"
#include "wire.h"

#ifdef __cplusplus
extern "C" {
#endif

// PSY2 file load
// This loads the psy2 file format used in older psycle
// versions, where many songs were written in.
// File save in PSY2 was dropped with the new PSY3 format.
// Later psycle versions are still able to load and convert it to PSY3.
// loader based on
// Psycle Version : 1.66
// Document Version : 1.1.2
// Date : 1 - Dec - 2002
// svn/psycle/doc/file-format.description.doc

struct psy_audio_SongFile;

/// helper struct for the PSY2 loader.
typedef struct VSTLoader {
	uint8_t valid;
	char dllName[128];
	int32_t numpars;
	float* pars;
} VSTLoader;

typedef struct VstPreload {
	char _editName[16];
	struct psy_audio_SongFile* songfile;
} VstPreload;

typedef struct PSY2Loader {
	struct psy_audio_SongFile* songfile;
	struct PsyFile* fp;
	struct psy_audio_Song* song;
	uint8_t currentoctave;
	int32_t songtracks;
	unsigned char busEffect[64];
	unsigned char busMachine[64];
	unsigned char playorder[MAX_SONG_POSITIONS];
	int32_t playlength;
	uint8_t _machineActive[128];
	psy_audio_Machine* pMac[128];
	InternalMachinesConvert converter;
	VSTLoader vstL[OLD_MAX_PLUGINS];
	psy_audio_LegacyWires legacywires;
	psy_audio_LegacyWires legacywiresremapped;
} PSY2Loader;

void psy2loader_init(PSY2Loader*, struct psy_audio_SongFile*);
void psy2loader_dispose(PSY2Loader*);
int psy2loader_load(PSY2Loader*);
// no psy2 save implemented

struct PsyFile;

psy_audio_MachineWires* psy_audio_read_psy2machinewires(struct PsyFile*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSY2_H */


/*
unsigned char* pData;
psy_audio_Pattern* pattern;

pattern = psy_audio_pattern_allocinit();
psy_audio_patterns_insert(&self->songfile->song->patterns, i, pattern);
pData = malloc(numlines * OLD_MAX_TRACKS * PSY2_EVENT_SIZE);
if (status = psyfile_read(self->songfile->file, pData,
	numlines * OLD_MAX_TRACKS * PSY2_EVENT_SIZE)) {
	free(pData);
	return status;
}
psy_audio_convert_legacypattern(pattern, pData, OLD_MAX_TRACKS,
	numlines, (uint32_t)psy_audio_song_lpb(self->song));
free(pData);
pData = NULL; 
*/