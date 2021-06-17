/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SEQUENCECMDS_H)
#define SEQUENCECMDS_H

/* host */
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SequenceCmds */
typedef struct SequenceCmds {
	/* internal */
	/* references */
	Workspace* workspace;
	psy_audio_Patterns* patterns;
	psy_audio_Sequence* sequence;
	psy_audio_Player* player;
} SequenceCmds;

void sequencecmds_init(SequenceCmds*, Workspace*);

void sequencecmds_update(SequenceCmds*);
void sequencecmds_newentry(SequenceCmds*, psy_audio_SequenceEntryType);
void sequencecmds_insertentry(SequenceCmds*, psy_audio_SequenceEntryType);
void sequencecmds_cloneentry(SequenceCmds*);
void sequencecmds_delentry(SequenceCmds*);
void sequencecmds_incpattern(SequenceCmds*);
void sequencecmds_decpattern(SequenceCmds*);
void sequencecmds_changepattern(SequenceCmds*, uintptr_t step);
void sequencecmds_copy(SequenceCmds*);
void sequencecmds_paste(SequenceCmds*);
void sequencecmds_singleselection(SequenceCmds*);
void sequencecmds_multiselection(SequenceCmds*);
void sequencecmds_clear(SequenceCmds*);
void sequencecmds_appendtrack(SequenceCmds*);
void sequencecmds_inserttrack(SequenceCmds*);
void sequencecmds_deltrack(SequenceCmds*, uintptr_t trackindex);
void sequencecmds_changeplayposition(SequenceCmds*);

#ifdef __cplusplus
}
#endif

#endif /* SEQUENCECMDS_H */
