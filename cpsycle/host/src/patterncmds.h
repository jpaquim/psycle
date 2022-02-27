/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNCMDS_H)
#define PATTERNCMDS_H

/* host */
#include "workspace.h"
/* audio */
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

/* InsertCommand */
typedef struct InsertCommand {
	/* inherits */
	psy_Command command;
	/* internal */
	psy_audio_SequenceCursor cursor;
	psy_audio_Pattern* pattern;	
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	bool insert;
	/* references */
	psy_audio_Sequence* sequence;	
} InsertCommand;

InsertCommand* insertcommand_allocinit(psy_audio_Pattern*,
	psy_audio_SequenceCursor, psy_audio_PatternEvent,
	psy_audio_Sequence*);

/* RemoveCommand */
typedef struct RemoveCommand {
	/* inherits */
	psy_Command command;
	psy_audio_SequenceCursor cursor;
	psy_audio_Pattern* pattern;
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	bool remove;
	/* references */
	psy_audio_Sequence* sequence;	
} RemoveCommand;

RemoveCommand* removecommand_allocinit(psy_audio_Pattern*,
	psy_audio_SequenceCursor, psy_audio_Sequence*);

/* BlockTranspose */
typedef struct BlockTransposeCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	psy_audio_SequenceCursor cursor;
	psy_audio_BlockSelection block;
	intptr_t transposeoffset;	
	/* references */
	psy_audio_Sequence* sequence;	
} BlockTransposeCommand;

BlockTransposeCommand* blocktransposecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_BlockSelection block, psy_audio_SequenceCursor, intptr_t transposeoffset,
	psy_audio_Sequence*);

typedef struct BlockRemoveCommand {
	/* inherits */
	psy_Command command;
	psy_audio_BlockSelection selection;	
	psy_audio_Patterns oldpatterns;
	bool remove;
	/* references */
	psy_audio_Sequence* sequence;	
} BlockRemoveCommand;

BlockRemoveCommand* blockremovecommand_alloc(psy_audio_Sequence*,
	psy_audio_BlockSelection);

typedef struct BlockPasteCommand {
	/* inherits */
	psy_Command command;
	psy_audio_SequenceCursor destcursor;	
	psy_audio_Patterns oldpatterns;
	psy_dsp_big_beat_t bpl;
	bool paste;
	bool mix;
	psy_audio_BlockSelection selection;
	/* references */
	psy_audio_Sequence* sequence;
	psy_audio_Pattern* source;
} BlockPasteCommand;

BlockPasteCommand* blockpastecommand_alloc(psy_audio_Sequence*,
	psy_audio_Pattern* source, psy_audio_SequenceCursor,
	psy_dsp_big_beat_t bpl, bool mix);


#ifdef __cplusplus
}
#endif

#endif /* PATTERNCMDS_H*/
