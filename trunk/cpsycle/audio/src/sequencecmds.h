// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SEQUENCECMDS_H
#define psy_audio_SEQUENCECMDS_H

// audio
#include "sequence.h"

typedef struct psy_audio_SequenceInsertCommand {
	// inherits
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
	psy_audio_OrderIndex index;
	uintptr_t patidx;
	psy_audio_SequenceSelection restoreselection;
} psy_audio_SequenceInsertCommand;

psy_audio_SequenceInsertCommand* psy_audio_sequenceinsertcommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex, uintptr_t patidx);

typedef struct psy_audio_SequenceRemoveCommand {
	// inherits
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
	psy_audio_OrderIndex index;
	uintptr_t patidx;
	psy_audio_SequenceSelection restoreselection;
	bool success;
} psy_audio_SequenceRemoveCommand;

psy_audio_SequenceRemoveCommand* psy_audio_sequenceremovecommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex);

typedef struct psy_audio_SequenceClearCommand {
	// inherits
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;		
	psy_audio_SequenceSelection restoreselection;
	psy_audio_Sequence restoresequence;
} psy_audio_SequenceClearCommand;

psy_audio_SequenceClearCommand* psy_audio_sequenceclearcommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCECMDS_H */
