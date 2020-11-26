// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "patterncmds.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <pattern.h>
#include <patternio.h>
#include "cmdsnotes.h"
#include "skingraphics.h"
#include <uiopendialog.h>
#include <uisavedialog.h>

#include "../../driver/eventdriver.h"
#include "../../detail/trace.h"
#include "../../detail/portable.h"

static void InsertCommandDispose(InsertCommand*);
static void InsertCommandExecute(InsertCommand*);
static void InsertCommandRevert(InsertCommand*);

// vtable
static psy_CommandVtable insertcommandcommand_vtable;
static bool insertcommandcommand_vtable_initialized = FALSE;

static void insertcommandcommand_vtable_init(InsertCommand* self)
{
	if (!insertcommandcommand_vtable_initialized) {
		insertcommandcommand_vtable = *(self->command.vtable);
		insertcommandcommand_vtable.dispose = (psy_fp_command)InsertCommandDispose;
		insertcommandcommand_vtable.execute = (psy_fp_command)InsertCommandExecute;
		insertcommandcommand_vtable.revert = (psy_fp_command)InsertCommandRevert;
		insertcommandcommand_vtable_initialized = TRUE;
	}
}

InsertCommand* InsertCommandAlloc(psy_audio_Pattern* pattern, double bpl,
	psy_audio_PatternCursor cursor, psy_audio_PatternEvent event,
	Workspace* workspace)
{
	InsertCommand* rv;

	rv = malloc(sizeof(InsertCommand));
	if (rv) {
		psy_command_init(&rv->command);
		insertcommandcommand_vtable_init(rv);
		rv->command.vtable = &insertcommandcommand_vtable;
		rv->cursor = cursor;
		rv->bpl = bpl;
		rv->event = event;
		rv->insert = 0;
		rv->pattern = pattern;
		rv->workspace = workspace;
	}
	return rv;
}

void InsertCommandDispose(InsertCommand* self) { }

void InsertCommandExecute(InsertCommand* self)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(self->pattern,
		self->cursor.track,
		(psy_dsp_big_beat_t)self->cursor.offset,
		(psy_dsp_big_beat_t)self->bpl, &prev);
	if (node) {
		self->oldevent = psy_audio_pattern_event(self->pattern, node);
		psy_audio_pattern_setevent(self->pattern, node, &self->event);
		self->insert = 0;
	} else {
		node = psy_audio_pattern_insert(self->pattern,
			prev,
			self->cursor.track,
			(psy_dsp_big_beat_t)self->cursor.offset,
			&self->event);
		self->insert = 1;
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
}

void InsertCommandRevert(InsertCommand* self)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(self->pattern,
		self->cursor.track,
		self->cursor.offset,
		self->bpl, &prev);
	if (node) {
		if (self->insert) {
			psy_audio_pattern_remove(self->pattern, node);
			psy_audio_sequencer_checkiterators(
				&self->workspace->player.sequencer,
				node);
		} else {
			psy_audio_pattern_setevent(self->pattern, node, &self->oldevent);
		}
	}
	workspace_setpatterneditposition(self->workspace, self->cursor);
}



static void BlockTransposeCommandDispose(BlockTransposeCommand*);
static void BlockTransposeCommandExecute(BlockTransposeCommand*);
static void BlockTransposeCommandRevert(BlockTransposeCommand*);
// vtable
static psy_CommandVtable blocktransposecommand_vtable;
static int blocktransposecommand_vtable_initialized = 0;

static void blocktransposecommandcommand_vtable_init(BlockTransposeCommand* self)
{
	if (!blocktransposecommand_vtable_initialized) {
		blocktransposecommand_vtable = *(self->command.vtable);
		blocktransposecommand_vtable.dispose = (psy_fp_command)BlockTransposeCommandDispose;
		blocktransposecommand_vtable.execute = (psy_fp_command)BlockTransposeCommandExecute;
		blocktransposecommand_vtable.revert = (psy_fp_command)BlockTransposeCommandRevert;
		blocktransposecommand_vtable_initialized = 1;
	}
}

BlockTransposeCommand* BlockTransposeCommandAlloc(psy_audio_Pattern* pattern,
	PatternSelection block, psy_audio_PatternCursor cursor, int transposeoffset,
	Workspace* workspace)
{
	BlockTransposeCommand* rv;

	rv = malloc(sizeof(BlockTransposeCommand));
	if (rv) {
		psy_command_init(&rv->command);
		blocktransposecommandcommand_vtable_init(rv);
		rv->command.vtable = &blocktransposecommand_vtable;
		rv->pattern = pattern;
		psy_audio_pattern_init(&rv->oldpattern);
		rv->block = block;
		rv->cursor = cursor;
		rv->transposeoffset = transposeoffset;
		rv->workspace = workspace;
	}
	return rv;
}

void BlockTransposeCommandDispose(BlockTransposeCommand* self)
{
	psy_audio_pattern_dispose(&self->oldpattern);
}

void BlockTransposeCommandExecute(BlockTransposeCommand* self)
{
	workspace_setpatterneditposition(self->workspace, self->cursor);
	psy_audio_pattern_copy(&self->oldpattern, self->pattern);
	psy_audio_pattern_blocktranspose(self->pattern,
		self->block.topleft,
		self->block.bottomright, self->transposeoffset);
}

void BlockTransposeCommandRevert(BlockTransposeCommand* self)
{
	assert(self->pattern);
	if (self->pattern) {
		workspace_setpatterneditposition(self->workspace, self->cursor);
		psy_audio_pattern_copy(self->pattern, &self->oldpattern);
	}
}
