/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SONGIO_H
#define psy_audio_SONGIO_H

#include "wire.h"

#include <properties.h>
#include <fileio.h>
#include <signal.h>
#include <hashtbl.h>
#include <dir.h>
#include "instruments.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int x;
	int y;
} psy_audio_MachineUi;

struct psy_audio_Instrument;
struct psy_audio_Song;

/* song loadand save */
typedef struct psy_audio_SongFile {
	struct psy_audio_Song* song;
	struct PsyFile* file;
	int err;
	const char *serr;
	int warnings;
	psy_Signal signal_output;
	psy_Signal signal_warning;	
	char* path;
	uintptr_t machinesoloed;
	/* MachineID X [psy_Table* LegacyWires (ConnectionID X psy_audio_LegacyWire*)] */
	psy_audio_LegacyWires* legacywires;
	bool convert_dos_utf8;
} psy_audio_SongFile;

void psy_audio_songfile_init(psy_audio_SongFile*);
void psy_audio_songfile_init_song(psy_audio_SongFile*, struct psy_audio_Song*);
void psy_audio_songfile_dispose(psy_audio_SongFile*);
int psy_audio_songfile_load(psy_audio_SongFile*, const char* filename);
int psy_audio_songfile_save(psy_audio_SongFile*, const char* filename);
int psy_audio_songfile_exportmodule(psy_audio_SongFile*, const char* filename);
int psy_audio_songfile_export_midi_file(psy_audio_SongFile*, const char* filename);
int psy_audio_songfile_export_ly_file(psy_audio_SongFile* self, const char* filename);
int psy_audio_songfile_saveinstrument(psy_audio_SongFile*, const char* filename,
	struct psy_audio_Instrument*);
int psy_audio_songfile_loadinstrument(psy_audio_SongFile*, const char* filename,
	psy_audio_InstrumentIndex);
void psy_audio_songfile_warn(psy_audio_SongFile*, const char* text);
void psy_audio_songfile_message(psy_audio_SongFile*, const char* text);
const char* psy_audio_songfile_loadfilter(void);
const char* psy_audio_songfile_standardloadfilter(void);
const char* psy_audio_songfile_savefilter(void);
const char* psy_audio_songfile_standardsavefilter(void);
const char* psy_audio_songfile_instloadfilter(void);
const char* psy_audio_songfile_standardinstloadfilter(void);
const char* psy_audio_songfile_instsavefilter(void);
const char* psy_audio_songfile_standardinstsavefilter(void);
int psy_audio_songfile_errfile(psy_audio_SongFile*);
void psy_audio_songfile_createmaster(psy_audio_SongFile*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SONGIO_H */
