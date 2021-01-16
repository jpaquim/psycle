// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XM_SONGLOADER_H
#define psy_audio_XM_SONGLOADER_H

#include "../../detail/psydef.h"

// local
#include "machine.h"
#include "song.h"
#include "songio.h"
#include "xmdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

// XMSongLoader and MODSongLoader
//
// Imports xm, xi and mod files using sampulse. The pattern is transformed to
// sampulse cmds and instruments are mapped to virtual generators

// XMSongLoader
typedef struct XMSongLoader {
	// internal data
	int32_t instrcount;
	int32_t smplen[256];
	int8_t smpflags[256];
	uint8_t highoffset[32];
	uint8_t memportaup[32];
	uint8_t memportadown[32];
	uint8_t memportanote[32];
	uint8_t memportapos[32];
	int16_t tempoticks;
	int16_t tempobpm;
	int16_t extracolumn;
	int16_t maxextracolumn;
	XMFILEHEADER header;
	psy_audio_Machine* sampler;	
	psy_Table xmtovirtual;
	// references
	psy_audio_SongFile* songfile;
	PsyFile* fp;
	psy_audio_Song* song;
} XMSongLoader;

// inits the xm loader, songfile must contain a valid song and PsyFile pointer
void xmsongloader_init(XMSongLoader*, psy_audio_SongFile*);
void xmsongloader_dispose(XMSongLoader*);

// loads a xm file
// returns PSY_OK, if file was successfully loaded
int xmsongloader_load(XMSongLoader*);
// loads a xi instrument
// returns PSY_OK, if file was successfully loaded
int xmsongloader_loadxi(XMSongLoader*, psy_audio_InstrumentIndex);

// MODSongLoader
typedef struct MODHEADER
{
	uint8_t songlength;
	uint8_t unused;
	uint8_t order[128];
	uint8_t pID[4];
} MODHEADER;

typedef struct MODSAMPLEHEADER
{
	char sampleName[22];
	uint16_t sampleLength;
	uint8_t finetune;
	uint8_t volume;
	uint16_t loopStart;
	uint16_t loopLength;
} MODSAMPLEHEADER;

typedef struct MODSongLoader
{
	// internal data
	uint16_t smplen[32];
	int8_t smpflags[256];
	bool speedpatch;
	MODHEADER header;
	MODSAMPLEHEADER samples[32];
	psy_audio_Machine* sampler;
	psy_Table xmtovirtual;
	// references
	psy_audio_SongFile* songfile;
	PsyFile* fp;
	psy_audio_Song* song;
} MODSongLoader;

void modsongloader_init(MODSongLoader*, psy_audio_SongFile*);
void modsongloader_dispose(MODSongLoader*);

int modsongloader_load(MODSongLoader*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XM_SONGLOADER_H */
