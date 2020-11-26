// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNCMDS)
#define PATTERNCMDS

#include "notestab.h"
#include "skincoord.h"
#include "tabbar.h"
#include "workspace.h"
#include <uibutton.h>
#include <uilabel.h>
#include <uiscroller.h>
#include "zoombox.h"
#include "interpolatecurveview.h"
#include "patternviewskin.h"

#include <pattern.h>

typedef struct {
	psy_Command command;
	psy_audio_PatternCursor cursor;
	psy_audio_Pattern* pattern;
	double bpl;
	psy_audio_PatternEvent event;
	psy_audio_PatternEvent oldevent;
	int insert;
	Workspace* workspace;
} InsertCommand;


InsertCommand* InsertCommandAlloc(psy_audio_Pattern* pattern, double bpl,
	psy_audio_PatternCursor cursor, psy_audio_PatternEvent event,
	Workspace* workspace);

// BlockTranspose
typedef struct {
	psy_Command command;
	psy_audio_Pattern* pattern;
	psy_audio_Pattern oldpattern;
	psy_audio_PatternCursor cursor;
	PatternSelection block;
	int transposeoffset;
	Workspace* workspace;
} BlockTransposeCommand;

BlockTransposeCommand* BlockTransposeCommandAlloc(psy_audio_Pattern* pattern,
	PatternSelection block, psy_audio_PatternCursor cursor, int transposeoffset,
	Workspace* workspace);

#endif /* PATTERNCMDS*/
