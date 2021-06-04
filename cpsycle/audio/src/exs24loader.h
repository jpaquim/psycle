/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
** derived from EXS24 For Renoise Matt Allan (MIT Licence) https://github.com/matt-allan/renoise-exs24
*/

#ifndef psy_audio_EXS24LOADER_H
#define psy_audio_EXS24LOADER_H

#include "instruments.h"
#include "wire.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Instrument import for the Logic EXS24 format
**
** todo: sample path detection: prompt for path if not found
*/

struct psy_audio_SongFile;

typedef struct psy_audio_EXS24Loader {
	struct psy_audio_SongFile* songfile;
	psy_audio_LegacyWires legacywires;	
	struct PsyFile* fp;
	struct psy_audio_Song* song;
	int progress;
	bool big_endian;
	bool is_size_expanded;
	uint8_t baseC;
} psy_audio_EXS24Loader;

void psy_audio_exs24loader_init(psy_audio_EXS24Loader*,
	struct psy_audio_SongFile*);
void psy_audio_exs24loader_dispose(psy_audio_EXS24Loader*);

int psy_audio_exs24loader_load(psy_audio_EXS24Loader*);
void psy_audio_exs24loader_dispose(psy_audio_EXS24Loader*);
psy_audio_EXS24Loader* psy_audio_exs24loader_allocinit(
	struct psy_audio_SongFile*);
void psy_audio_exs24loader_deallocate(psy_audio_EXS24Loader*);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_EXS24LOADER_H */
