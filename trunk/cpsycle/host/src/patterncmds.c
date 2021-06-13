// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "patterncmds.h"
#include "workspace.h"

#include <exclusivelock.h>
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

// InsertCommand
// prototypes
static void insertcommand_dispose(InsertCommand*);
static void insertcommand_execute(InsertCommand*);
static void insertcommand_revert(InsertCommand*);
// vtable
static psy_CommandVtable insertcommandcommand_vtable;
static bool insertcommandcommand_vtable_initialized = FALSE;

static void insertcommandcommand_vtable_init(InsertCommand* self)
{
	if (!insertcommandcommand_vtable_initialized) {
		insertcommandcommand_vtable = *(self->command.vtable);
		insertcommandcommand_vtable.dispose = (psy_fp_command)insertcommand_dispose;
		insertcommandcommand_vtable.execute = (psy_fp_command)insertcommand_execute;
		insertcommandcommand_vtable.revert = (psy_fp_command)insertcommand_revert;
		insertcommandcommand_vtable_initialized = TRUE;
	}
}
// implementation
InsertCommand* insertcommand_alloc(psy_audio_Pattern* pattern, double bpl,
	psy_audio_PatternCursor cursor, psy_audio_PatternEvent event,
	Workspace* workspace)
{
	InsertCommand* rv;

	rv = (InsertCommand*)malloc(sizeof(InsertCommand));
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

void insertcommand_dispose(InsertCommand* self) { }

void insertcommand_execute(InsertCommand* self)
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
	if (self->workspace) {
		workspace_setpatterncursor(self->workspace, self->cursor);
	}
}

void insertcommand_revert(InsertCommand* self)
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
			if (self->workspace) {
				psy_audio_sequencer_checkiterators(
					&workspace_player(self->workspace)->sequencer,
					node);
			}
		} else {
			psy_audio_pattern_setevent(self->pattern, node, &self->oldevent);
		}
	}
	if (self->workspace) {
		workspace_setpatterncursor(self->workspace, self->cursor);
	}
}

// RemoveCommand
// prototypes
static void removecommand_dispose(RemoveCommand*);
static void removecommand_execute(RemoveCommand*);
static void removecommand_revert(RemoveCommand*);
// vtable
static psy_CommandVtable removecommandcommand_vtable;
static bool removecommandcommand_vtable_initialized = FALSE;

static void removecommandcommand_vtable_init(RemoveCommand* self)
{
	if (!removecommandcommand_vtable_initialized) {
		removecommandcommand_vtable = *(self->command.vtable);
		removecommandcommand_vtable.dispose = (psy_fp_command)removecommand_dispose;
		removecommandcommand_vtable.execute = (psy_fp_command)removecommand_execute;
		removecommandcommand_vtable.revert = (psy_fp_command)removecommand_revert;
		removecommandcommand_vtable_initialized = TRUE;
	}
}
// implementation
RemoveCommand* removecommand_alloc(psy_audio_Pattern* pattern, double bpl,
	psy_audio_PatternCursor cursor, Workspace* workspace)
{
	RemoveCommand* rv;

	rv = (RemoveCommand*)malloc(sizeof(RemoveCommand));
	if (rv) {
		psy_command_init(&rv->command);
		removecommandcommand_vtable_init(rv);
		rv->command.vtable = &removecommandcommand_vtable;
		rv->cursor = cursor;
		rv->bpl = bpl;
		rv->remove = 0;
		rv->pattern = pattern;
		rv->workspace = workspace;
	}
	return rv;
}

void removecommand_dispose(RemoveCommand* self) { }

void removecommand_execute(RemoveCommand* self)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	node = psy_audio_pattern_findnode(self->pattern,
		self->cursor.track,
		(psy_dsp_big_beat_t)self->cursor.offset,
		(psy_dsp_big_beat_t)self->bpl, &prev);
	if (node) {
		self->oldevent = psy_audio_pattern_event(self->pattern, node);
		psy_audio_pattern_remove(self->pattern, node);
		if (self->workspace) {
			psy_audio_sequencer_checkiterators(
				&workspace_player(self->workspace)->sequencer,
				node);
		}
		self->remove = 1;
	} else {		
		self->remove = 0;
	}
	if (self->workspace) {
		workspace_setpatterncursor(self->workspace, self->cursor);
	}
}

void removecommand_revert(RemoveCommand* self)
{
	if (self->remove) {
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;

		node = psy_audio_pattern_findnode(self->pattern,
			self->cursor.track,
			self->cursor.offset,
			self->bpl, &prev);		
		node = psy_audio_pattern_insert(self->pattern,
			prev,
			self->cursor.track,
			(psy_dsp_big_beat_t)self->cursor.offset,
			&self->oldevent);
		if (self->workspace) {
			workspace_setpatterncursor(self->workspace, self->cursor);
		}
		self->remove = 0;
	}
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

BlockTransposeCommand* blocktransposecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_PatternSelection block, psy_audio_PatternCursor cursor, int transposeoffset,
	Workspace* workspace)
{
	BlockTransposeCommand* rv;

	rv = (BlockTransposeCommand*)malloc(sizeof(BlockTransposeCommand));
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
	workspace_setpatterncursor(self->workspace, self->cursor);
	psy_audio_pattern_copy(&self->oldpattern, self->pattern);
	psy_audio_pattern_blocktranspose(self->pattern,
		self->block.topleft,
		self->block.bottomright, self->transposeoffset);
}

void BlockTransposeCommandRevert(BlockTransposeCommand* self)
{
	assert(self->pattern);
	if (self->pattern) {
		workspace_setpatterncursor(self->workspace, self->cursor);
		psy_audio_pattern_copy(self->pattern, &self->oldpattern);
	}
}

// BlockRemoveCommand
// prototypes
static void BlockRemoveCommandDispose(BlockRemoveCommand*);
static void BlockRemoveCommandExecute(BlockRemoveCommand*);
static void BlockRemoveCommandRevert(BlockRemoveCommand*);
// vtable
static psy_CommandVtable blockremovecommandcommand_vtable;
static bool blockremovecommandcommand_vtable_initialized = FALSE;

static void blockremovecommandcommand_vtable_init(BlockRemoveCommand* self)
{
	if (!blockremovecommandcommand_vtable_initialized) {
		blockremovecommandcommand_vtable = *(self->command.vtable);
		blockremovecommandcommand_vtable.dispose = (psy_fp_command)BlockRemoveCommandDispose;
		blockremovecommandcommand_vtable.execute = (psy_fp_command)BlockRemoveCommandExecute;
		blockremovecommandcommand_vtable.revert = (psy_fp_command)BlockRemoveCommandRevert;
		blockremovecommandcommand_vtable_initialized = TRUE;
	}
}
// implementation
BlockRemoveCommand* blockremovecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_PatternSelection selection, Workspace* workspace)
{
	BlockRemoveCommand* rv;

	rv = (BlockRemoveCommand*)malloc(sizeof(BlockRemoveCommand));
	if (rv) {
		psy_command_init(&rv->command);
		blockremovecommandcommand_vtable_init(rv);
		rv->command.vtable = &blockremovecommandcommand_vtable;
		rv->selection = selection;
		rv->remove = FALSE;
		rv->pattern = pattern;
		rv->workspace = workspace;
	}
	return rv;
}

void BlockRemoveCommandDispose(BlockRemoveCommand* self) { }

void BlockRemoveCommandExecute(BlockRemoveCommand* self)
{
	bool isplaying;

	psy_audio_exclusivelock_enter();
	isplaying = psy_audio_player_playing(&self->workspace->player);		
	if (isplaying) {
		psy_audio_player_pause(&self->workspace->player);
	}
	if (!self->remove) {
		psy_audio_pattern_init(&self->oldpattern);
		psy_audio_pattern_copy(&self->oldpattern,
			self->pattern);
	}
	psy_audio_pattern_blockremove(self->pattern,
		self->selection.topleft,
		self->selection.bottomright);
	self->remove = TRUE;
	if (isplaying) {
		psy_audio_player_resume(&self->workspace->player);
	}
	psy_audio_exclusivelock_leave();
}

void BlockRemoveCommandRevert(BlockRemoveCommand* self)
{
	if (self->remove) {
		bool isplaying;

		psy_audio_exclusivelock_enter();
		isplaying = psy_audio_player_playing(&self->workspace->player);
		if (isplaying) {
			psy_audio_player_pause(&self->workspace->player);
		}
		psy_audio_pattern_copy(self->pattern, &self->oldpattern);
		psy_audio_pattern_dispose(&self->oldpattern);
		self->remove = 0;
		if (isplaying) {
			psy_audio_player_resume(&self->workspace->player);
		}
		psy_audio_exclusivelock_leave();
	}
}

// BlockPasteCommand
// prototypes
static void BlockPasteCommandDispose(BlockPasteCommand*);
static void BlockPasteCommandExecute(BlockPasteCommand*);
static void BlockPasteCommandRevert(BlockPasteCommand*);
// vtable
static psy_CommandVtable blockpastecommandcommand_vtable;
static bool blockpastecommandcommand_vtable_initialized = FALSE;

static void blockpastecommandcommand_vtable_init(BlockPasteCommand* self)
{
	if (!blockpastecommandcommand_vtable_initialized) {
		blockpastecommandcommand_vtable = *(self->command.vtable);
		blockpastecommandcommand_vtable.dispose = (psy_fp_command)BlockPasteCommandDispose;
		blockpastecommandcommand_vtable.execute = (psy_fp_command)BlockPasteCommandExecute;
		blockpastecommandcommand_vtable.revert = (psy_fp_command)BlockPasteCommandRevert;
		blockpastecommandcommand_vtable_initialized = TRUE;
	}
}
// implementation
BlockPasteCommand* blockpastecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_Pattern* source, psy_audio_PatternCursor destcursor,
	psy_dsp_big_beat_t bpl, bool mix, Workspace* workspace)
{
	BlockPasteCommand* rv;

	rv = (BlockPasteCommand*)malloc(sizeof(BlockPasteCommand));
	if (rv) {
		psy_command_init(&rv->command);
		blockpastecommandcommand_vtable_init(rv);
		rv->command.vtable = &blockpastecommandcommand_vtable;
		rv->destcursor = destcursor;
		rv->paste = FALSE;
		rv->pattern = pattern;
		rv->bpl = bpl;
		rv->mix = mix;
		psy_audio_pattern_init(&rv->source);
		psy_audio_pattern_copy(&rv->source, source);
		rv->workspace = workspace;
	}
	return rv;
}

void BlockPasteCommandDispose(BlockPasteCommand* self) { }

void BlockPasteCommandExecute(BlockPasteCommand* self)
{
	if (!self->paste) {
		psy_audio_pattern_init(&self->oldpattern);
		psy_audio_pattern_copy(&self->oldpattern,
			self->pattern);
	}
	if (self->mix) {
		psy_audio_pattern_blockmixpaste(self->pattern,
			&self->source, self->destcursor,
			self->bpl);
	} else {
		psy_audio_pattern_blockpaste(self->pattern,
			&self->source, self->destcursor,
			self->bpl);
	}
	self->paste = TRUE;
	//psy_audio_sequencer_checkiterators(
		//&workspace_player(self->workspace).sequencer,
		//node);	
}

void BlockPasteCommandRevert(BlockPasteCommand* self)
{
	if (self->paste) {
		psy_audio_pattern_copy(self->pattern, &self->oldpattern);
		psy_audio_pattern_dispose(&self->oldpattern);
		self->paste = FALSE;
	}
}
