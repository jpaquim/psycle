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
	Workspace* workspace;
} InsertCommand;

InsertCommand* insertcommand_allocinit(psy_audio_Pattern*,
	psy_audio_SequenceCursor, psy_audio_PatternEvent, Workspace*);

/* RemoveCommand */
typedef struct RemoveCommand {
	/* inherits */
	psy_Command command;
	psy_audio_SequenceCursor cursor;
	psy_audio_Pattern* pattern;
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	int remove;
	Workspace* workspace;
} RemoveCommand;

RemoveCommand* removecommand_allocinit(psy_audio_Pattern*,
	psy_audio_SequenceCursor, Workspace*);

/* BlockTranspose */
typedef struct BlockTransposeCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	psy_audio_SequenceCursor cursor;
	psy_audio_BlockSelection block;
	intptr_t transposeoffset;
	Workspace* workspace;
	psy_audio_Song* song;
} BlockTransposeCommand;

BlockTransposeCommand* blocktransposecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_BlockSelection block, psy_audio_SequenceCursor, intptr_t transposeoffset,
	psy_audio_Song*);

typedef struct BlockRemoveCommand {
	/* inherits */
	psy_Command command;
	psy_audio_BlockSelection selection;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	bool remove;
	psy_audio_Song* song;	
} BlockRemoveCommand;

BlockRemoveCommand* blockremovecommand_alloc(psy_audio_Pattern*,
	psy_audio_BlockSelection, psy_audio_Song*);

typedef struct BlockPasteCommand {
	/* inherits */
	psy_Command command;
	psy_audio_SequenceCursor destcursor;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern source;
	psy_audio_Pattern oldpattern;
	psy_dsp_big_beat_t bpl;
	bool paste;
	bool mix;	
} BlockPasteCommand;

BlockPasteCommand* blockpastecommand_alloc(psy_audio_Pattern*,
	psy_audio_Pattern* source, psy_audio_SequenceCursor,
	psy_dsp_big_beat_t bpl, bool mix);


#ifdef __cplusplus
}
#endif

#endif /* PATTERNCMDS_H*/
