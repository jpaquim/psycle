/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNHOSTCMDS_H)
#define PATTERNHOSTCMDS_H

/* host */
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternCmds
**
** Pattern commands used by tracker and pianoroll
*/

typedef struct PatternCmds {		
	psy_UndoRedo* undoredo;
	psy_audio_Pattern* patternpaste;
	DirConfig* dirconfig;
	/* references */
	psy_audio_Pattern* pattern;
	psy_audio_Sequence* sequence;	
	psy_audio_Player* player;
} PatternCmds;

void patterncmds_init(PatternCmds*, psy_audio_Sequence*, psy_audio_Player*,
	psy_UndoRedo*, psy_audio_Pattern* patternpaste, DirConfig*);

void patterncmds_setsequence(PatternCmds*, psy_audio_Sequence*);
void patterncmds_setpattern(PatternCmds*, psy_audio_Pattern*);
void patterncmds_blocktranspose(PatternCmds*, psy_audio_BlockSelection,
	psy_audio_SequenceCursor, intptr_t offset);
void patterncmds_blockdelete(PatternCmds*, psy_audio_BlockSelection);
void patterncmds_blockpaste(PatternCmds*, psy_audio_SequenceCursor, bool mix);
void patterncmds_blockcopy(PatternCmds*, psy_audio_BlockSelection);
void patterncmds_changeinstrument(PatternCmds*, psy_audio_BlockSelection);
void patterncmds_changemachine(PatternCmds*, psy_audio_BlockSelection);
void patterncmds_importpattern(PatternCmds*, psy_dsp_big_beat_t bpl);
void patterncmds_exportpattern(PatternCmds*, psy_dsp_big_beat_t bpl,
	uintptr_t numtracks);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNHOSTCMDS_H */
