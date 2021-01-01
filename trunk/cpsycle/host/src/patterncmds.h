// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNCMDS_H)
#define PATTERNCMDS_H

// host
#include "workspace.h"
// audio
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// inherits
	psy_Command command;
	psy_audio_PatternCursor cursor;
	psy_audio_Pattern* pattern;
	double bpl;
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	int insert;
	Workspace* workspace;
} InsertCommand;

InsertCommand* insertcommand_alloc(psy_audio_Pattern* pattern, double bpl,
	psy_audio_PatternCursor cursor, psy_audio_PatternEvent event,
	Workspace* workspace);

typedef struct {
	// inherits
	psy_Command command;
	psy_audio_PatternCursor cursor;
	psy_audio_Pattern* pattern;
	double bpl;
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	int remove;
	Workspace* workspace;
} RemoveCommand;

RemoveCommand* removecommand_alloc(psy_audio_Pattern*, double bpl,
	psy_audio_PatternCursor, Workspace*);

// BlockTranspose
typedef struct {
	// inherits
	psy_Command command;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	psy_audio_PatternCursor cursor;
	psy_audio_PatternSelection block;
	int transposeoffset;
	Workspace* workspace;
} BlockTransposeCommand;

BlockTransposeCommand* blocktransposecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_PatternSelection block, psy_audio_PatternCursor cursor, int transposeoffset,
	Workspace* workspace);

typedef struct {
	// inherits
	psy_Command command;
	psy_audio_PatternSelection selection;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	bool remove;
	Workspace* workspace;
} BlockRemoveCommand;

BlockRemoveCommand* blockremovecommand_alloc(psy_audio_Pattern*,
	psy_audio_PatternSelection, Workspace*);

typedef struct BlockPasteCommand {
	// inherits
	psy_Command command;
	psy_audio_PatternCursor destcursor;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern source;
	psy_audio_Pattern oldpattern;
	psy_dsp_big_beat_t bpl;
	bool paste;
	bool mix;
	Workspace* workspace;
} BlockPasteCommand;

BlockPasteCommand* blockpastecommand_alloc(psy_audio_Pattern*,
	psy_audio_Pattern* source, psy_audio_PatternCursor,
	psy_dsp_big_beat_t bpl, bool mix, Workspace*);


#ifdef __cplusplus
}
#endif

#endif /* PATTERNCMDS_H*/
