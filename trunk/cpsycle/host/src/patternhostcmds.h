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
	/* references */
	Workspace* workspace;
	psy_audio_Pattern* pattern;
	psy_audio_Patterns* patterns;
	psy_audio_Sequence* sequence;
	psy_audio_Player* player;
} PatternCmds;

void patterncmds_init(PatternCmds*, Workspace*);

void patterncmds_update(PatternCmds*);
void patterncmds_setpattern(PatternCmds*, psy_audio_Pattern*);
void patterncmds_blocktranspose(PatternCmds*, psy_audio_PatternSelection,
	psy_audio_SequenceCursor, int offset);
void patterncmds_blockdelete(PatternCmds*, psy_audio_PatternSelection);
void patterncmds_blockpaste(PatternCmds*, psy_audio_SequenceCursor, bool mix);
void patterncmds_blockcopy(PatternCmds*, psy_audio_PatternSelection);
void patterncmds_changeinstrument(PatternCmds*, psy_audio_PatternSelection);
void patterncmds_changemachine(PatternCmds*, psy_audio_PatternSelection);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNHOSTCMDS_H */
