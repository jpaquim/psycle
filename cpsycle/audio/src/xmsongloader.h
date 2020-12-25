// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XM_SONGLOADER_H
#define psy_audio_XM_SONGLOADER_H

#include "../../detail/psydef.h"

// local
#include "machine.h"
#include "songio.h"
#include "xmdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

// XMSongLoader and MODSongLoader
//
// Imports xm and mod files using sampulse. The pattern is transformed to
// sampulse cmds and instruments are mapped to virtual generators

// XMSongLoader
typedef struct XMSongLoader {
	// internal data
	int32_t m_iInstrCnt;
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
} XMSongLoader;

void xmsongloader_init(XMSongLoader*, psy_audio_SongFile*);
void xmsongloader_dispose(XMSongLoader*);

bool xmsongloader_load(XMSongLoader*);

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
	bool speedpatch;
	MODHEADER header;
	MODSAMPLEHEADER samples[32];
	psy_audio_Machine* sampler;
	psy_Table xmtovirtual;
	// references
	psy_audio_SongFile* songfile;
} MODSongLoader;

void modsongloader_init(MODSongLoader*, psy_audio_SongFile*);
void modsongloader_dispose(MODSongLoader*);

bool modsongloader_load(MODSongLoader*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XM_SONGLOADER_H */
