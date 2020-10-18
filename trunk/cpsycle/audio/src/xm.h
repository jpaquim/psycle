// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XM_H
#define psy_audio_XM_H

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_SongFile;

struct MODHEADER
{
	uint8_t songlength;
	uint8_t unused;
	uint8_t order[128];
	uint8_t pID[4];
};

struct MODSAMPLEHEADER
{
	char sampleName[22];
	uint16_t sampleLength;
	uint8_t finetune;
	uint8_t volume;
	uint16_t loopStart;
	uint16_t loopLength;
};

void psy_audio_xm_load(struct psy_audio_SongFile*);
void psy_audio_xm_save(struct psy_audio_SongFile*);
void psy_audio_xi_load(struct psy_audio_SongFile*, uintptr_t slot);
bool psy_audio_mod_load(struct psy_audio_SongFile*);
bool psy_audio_mod_isvalid(struct psy_audio_SongFile*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XM_H */
