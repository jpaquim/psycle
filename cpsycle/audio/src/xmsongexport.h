// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XMEXPORT_H
#define psy_audio_XMEXPORT_H

#include "constants.h"
#include "psyconvert.h"
#include "xmdefs.h"
	
#ifdef __cplusplus
extern "C" {
#endif

typedef struct XMSongExport {
	struct XMFILEHEADER m_Header;
	int macInstruments;
	int xmInstruments;
	int correctionIndex;
	bool isSampler[MAX_BUSES];
	bool isSampulse[MAX_BUSES];
	bool isBlitzorVst[256];
	int lastInstr[32];
	const psy_audio_LegacyPatternEntry* extraEntry[32];
	int addTicks;
} XMSongExport;

void xmsongexport_init(XMSongExport*);
void xmsongexport_dispose(XMSongExport*);
void xmsongexport_exportsong(XMSongExport*, struct psy_audio_SongFile*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XMEXPORT_H */
