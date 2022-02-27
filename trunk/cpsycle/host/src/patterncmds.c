/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patterncmds.h"
/* local */
#include "cmdsnotes.h"
#include "workspace.h"
/* audio */
#include <exclusivelock.h>
#include <pattern.h>
#include <patternio.h>
/* ui */
#include <uiopendialog.h>
#include <uisavedialog.h>
/* std*/
#include <math.h>
#include <assert.h>
/* driver */
#include "../../driver/eventdriver.h"
/* platform */
#include "../../detail/trace.h"
#include "../../detail/portable.h"


/* InsertCommand */

/* prototypes */
static void insertcommand_dispose(InsertCommand*);
static void insertcommand_execute(InsertCommand*, psy_Property* params);
static void insertcommand_revert(InsertCommand*);

/* vtable */
static psy_CommandVtable insertcommand_vtable;
static bool insertcommand_vtable_initialized = FALSE;

static void insertcommand_vtable_init(InsertCommand* self)
{
	if (!insertcommand_vtable_initialized) {
		insertcommand_vtable = *(self->command.vtable);
		insertcommand_vtable.dispose =
			(psy_fp_command)
			insertcommand_dispose;
		insertcommand_vtable.execute =
			(psy_fp_command_params)
			insertcommand_execute;
		insertcommand_vtable.revert =
			(psy_fp_command)
			insertcommand_revert;
		insertcommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &insertcommand_vtable;
}

/* implementation */
InsertCommand* insertcommand_allocinit(psy_audio_Pattern* pattern,
	psy_audio_SequenceCursor cursor, psy_audio_PatternEvent event,
	psy_audio_Sequence* sequence)
{
	InsertCommand* rv;

	rv = (InsertCommand*)malloc(sizeof(InsertCommand));
	if (rv) {
		psy_command_init(&rv->command);
		insertcommand_vtable_init(rv);
		rv->insert = FALSE;
		rv->cursor = cursor;		
		rv->event = event;		
		rv->pattern = pattern;
		rv->sequence = sequence;		
	}
	return rv;
}

void insertcommand_dispose(InsertCommand* self) { }

void insertcommand_execute(InsertCommand* self, psy_Property* params)
{
	psy_audio_exclusivelock_enter();
	self->oldevent = psy_audio_pattern_event_at_cursor(self->pattern,
		self->cursor);
	self->insert = (psy_audio_pattern_set_event_at_cursor(self->pattern,
		self->cursor, &self->event) != NULL);
	psy_audio_exclusivelock_leave();
	if (self->sequence) {
		psy_audio_sequence_set_cursor(self->sequence, self->cursor);
	}
}

void insertcommand_revert(InsertCommand* self)
{
	psy_audio_PatternNode* node;

	if (self->insert) {
		node = psy_audio_pattern_set_event_at_cursor(self->pattern, self->cursor, 
			&self->oldevent);
	} else {
		node = psy_audio_pattern_set_event_at_cursor(self->pattern, self->cursor,
			NULL);
	}		
	if (self->sequence) {
		psy_audio_sequence_set_cursor(self->sequence, self->cursor);			
	}
}

/* RemoveCommand */

/* prototypes */
static void removecommand_dispose(RemoveCommand*);
static void removecommand_execute(RemoveCommand*, psy_Property* params);
static void removecommand_revert(RemoveCommand*);
/* vtable */
static psy_CommandVtable removecommandcommand_vtable;
static bool removecommandcommand_vtable_initialized = FALSE;

static void removecommandcommand_vtable_init(RemoveCommand* self)
{
	if (!removecommandcommand_vtable_initialized) {
		removecommandcommand_vtable = *(self->command.vtable);
		removecommandcommand_vtable.dispose =
			(psy_fp_command)
			removecommand_dispose;
		removecommandcommand_vtable.execute =
			(psy_fp_command_params)
			removecommand_execute;
		removecommandcommand_vtable.revert =
			(psy_fp_command)
			removecommand_revert;
		removecommandcommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &removecommandcommand_vtable;
}

/* implementation */
RemoveCommand* removecommand_allocinit(psy_audio_Pattern* pattern,
	psy_audio_SequenceCursor cursor, psy_audio_Sequence* sequence)
{
	RemoveCommand* rv;

	rv = (RemoveCommand*)malloc(sizeof(RemoveCommand));
	if (rv) {
		psy_command_init(&rv->command);
		removecommandcommand_vtable_init(rv);		
		rv->cursor = cursor;		
		rv->remove = FALSE;
		rv->pattern = pattern;
		rv->sequence = sequence;		
	}
	return rv;
}

void removecommand_dispose(RemoveCommand* self) { }

void removecommand_execute(RemoveCommand* self, psy_Property* params)
{
	psy_audio_PatternNode* node;
	
	psy_audio_exclusivelock_enter();
	self->oldevent = psy_audio_pattern_event_at_cursor(self->pattern,
		self->cursor);
	node = psy_audio_pattern_set_event_at_cursor(self->pattern,
		self->cursor, NULL);
	self->remove = node != NULL;
	psy_audio_exclusivelock_leave();
	if (self->sequence) {
		psy_audio_sequence_set_cursor(self->sequence, self->cursor);
	}
}

void removecommand_revert(RemoveCommand* self)
{
	if (self->remove) {
		psy_audio_PatternNode* node;

		node = psy_audio_pattern_set_event_at_cursor(self->pattern,
			self->cursor, &self->oldevent);		
		self->remove = 0;
	}
	if (self->sequence) {
		psy_audio_sequence_set_cursor(self->sequence, self->cursor);
	}
}

/* BlockTransposeCommand*/

/* prototypes */
static void BlockTransposeCommandDispose(BlockTransposeCommand*);
static void BlockTransposeCommandExecute(BlockTransposeCommand*, psy_Property* params);
static void BlockTransposeCommandRevert(BlockTransposeCommand*);

/* vtable */
static psy_CommandVtable blocktransposecommand_vtable;
static bool blocktransposecommand_vtable_initialized = FALSE;

static void blocktransposecommandcommand_vtable_init(BlockTransposeCommand* self)
{
	if (!blocktransposecommand_vtable_initialized) {
		blocktransposecommand_vtable = *(self->command.vtable);
		blocktransposecommand_vtable.dispose =
			(psy_fp_command)
			BlockTransposeCommandDispose;
		blocktransposecommand_vtable.execute =
			(psy_fp_command_params)
			BlockTransposeCommandExecute;
		blocktransposecommand_vtable.revert =
			(psy_fp_command)
			BlockTransposeCommandRevert;
		blocktransposecommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &blocktransposecommand_vtable;
}

/* implementation */
BlockTransposeCommand* blocktransposecommand_alloc(psy_audio_Pattern* pattern,
	psy_audio_BlockSelection block, psy_audio_SequenceCursor cursor,
	intptr_t transposeoffset, psy_audio_Sequence* sequence)
{
	BlockTransposeCommand* rv;

	rv = (BlockTransposeCommand*)malloc(sizeof(BlockTransposeCommand));
	if (rv) {
		psy_command_init(&rv->command);
		blocktransposecommandcommand_vtable_init(rv);		
		rv->pattern = pattern;
		psy_audio_pattern_init(&rv->oldpattern);
		rv->block = block;
		rv->cursor = cursor;
		rv->transposeoffset = transposeoffset;		
		rv->sequence = sequence;		
	}
	return rv;
}

void BlockTransposeCommandDispose(BlockTransposeCommand* self)
{
	psy_audio_pattern_dispose(&self->oldpattern);
}

void BlockTransposeCommandExecute(BlockTransposeCommand* self, psy_Property* params)
{
	if (self->sequence) {
		psy_audio_sequence_set_cursor(self->sequence, self->cursor);
	}
	psy_audio_pattern_copy(&self->oldpattern, self->pattern);
	psy_audio_pattern_blocktranspose(self->pattern,
		self->block.topleft,
		self->block.bottomright, self->transposeoffset);
}

void BlockTransposeCommandRevert(BlockTransposeCommand* self)
{
	if (self->pattern && self->sequence) {
		psy_audio_sequence_set_cursor(self->sequence, self->cursor);		
		psy_audio_pattern_copy(self->pattern, &self->oldpattern);
	}
}

/* BlockRemoveCommand */

/* prototypes */
static void BlockRemoveCommandDispose(BlockRemoveCommand*);
static void BlockRemoveCommandExecute(BlockRemoveCommand*, psy_Property* params);
static void BlockRemoveCommandRevert(BlockRemoveCommand*);

/* vtable */
static psy_CommandVtable blockremovecommandcommand_vtable;
static bool blockremovecommandcommand_vtable_initialized = FALSE;

static void blockremovecommandcommand_vtable_init(BlockRemoveCommand* self)
{
	if (!blockremovecommandcommand_vtable_initialized) {
		blockremovecommandcommand_vtable = *(self->command.vtable);
		blockremovecommandcommand_vtable.dispose =
			(psy_fp_command)
			BlockRemoveCommandDispose;
		blockremovecommandcommand_vtable.execute =
			(psy_fp_command_params)
			BlockRemoveCommandExecute;
		blockremovecommandcommand_vtable.revert =
			(psy_fp_command)
			BlockRemoveCommandRevert;
		blockremovecommandcommand_vtable_initialized = TRUE;
	}
}

/* implementation */
BlockRemoveCommand* blockremovecommand_alloc(psy_audio_Sequence* sequence,
	psy_audio_BlockSelection selection)
{
	BlockRemoveCommand* rv;

	rv = (BlockRemoveCommand*)malloc(sizeof(BlockRemoveCommand));
	if (rv) {
		psy_command_init(&rv->command);
		blockremovecommandcommand_vtable_init(rv);
		rv->command.vtable = &blockremovecommandcommand_vtable;
		rv->selection = selection;
		rv->remove = FALSE;		
		rv->sequence = sequence;
		psy_audio_patterns_init(&rv->oldpatterns);
	}
	return rv;
}

void BlockRemoveCommandDispose(BlockRemoveCommand* self)
{
	psy_audio_patterns_dispose(&self->oldpatterns);
}

void BlockRemoveCommandExecute(BlockRemoveCommand* self, psy_Property* param)
{
	psy_audio_OrderIndex curr;

	psy_audio_exclusivelock_enter();
	curr = self->selection.topleft.orderindex;
	for (; curr.order <= self->selection.bottomright.orderindex.order; ++curr.order) {
		psy_audio_SequenceEntry* entry;

		entry = psy_audio_sequence_entry(self->sequence, curr);
		if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
			psy_audio_Pattern* pattern;
			psy_audio_SequencePatternEntry* patternentry;

			patternentry = (psy_audio_SequencePatternEntry*)entry;
			pattern = psy_audio_patterns_at(self->sequence->patterns, patternentry->patternslot);
			if (pattern) {
				psy_audio_patterns_insert(&self->oldpatterns, patternentry->patternslot,
					psy_audio_pattern_clone(pattern));
			}
		}
	}
	psy_audio_sequence_blockremove(self->sequence, self->selection);
	self->remove = TRUE;	
	psy_audio_exclusivelock_leave();
}

void BlockRemoveCommandRevert(BlockRemoveCommand* self)
{
	if (self->remove) {
		psy_audio_OrderIndex curr;

		curr = self->selection.topleft.orderindex;
		for (; curr.order <= self->selection.bottomright.orderindex.order; ++curr.order) {
			psy_audio_SequenceEntry* entry;

			entry = psy_audio_sequence_entry(self->sequence, curr);
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_Pattern* pattern;
				psy_audio_SequencePatternEntry* patternentry;

				patternentry = (psy_audio_SequencePatternEntry*)entry;
				pattern = psy_audio_patterns_at(&self->oldpatterns, patternentry->patternslot);
				if (pattern) {
					psy_audio_Pattern* currpattern;
					double length;
					uintptr_t tracks;

					currpattern = psy_audio_patterns_at(self->sequence->patterns,
						patternentry->patternslot);
					length = currpattern->length;
					tracks = currpattern->maxsongtracks;
					psy_audio_pattern_clear(currpattern);
					psy_audio_pattern_copy(currpattern, pattern);					
					currpattern->maxsongtracks = tracks;
					currpattern->length = length;					
				}
			}
		}		
		self->remove = 0;
		psy_audio_exclusivelock_leave();
	}
}

/* BlockPasteCommand */

/* prototypes */
static void BlockPasteCommandDispose(BlockPasteCommand*);
static void BlockPasteCommandExecute(BlockPasteCommand*, psy_Property* params);
static void BlockPasteCommandRevert(BlockPasteCommand*);

/* vtable */
static psy_CommandVtable blockpastecommandcommand_vtable;
static bool blockpastecommandcommand_vtable_initialized = FALSE;

static void blockpastecommandcommand_vtable_init(BlockPasteCommand* self)
{
	if (!blockpastecommandcommand_vtable_initialized) {
		blockpastecommandcommand_vtable = *(self->command.vtable);
		blockpastecommandcommand_vtable.dispose =
			(psy_fp_command)
			BlockPasteCommandDispose;
		blockpastecommandcommand_vtable.execute =
			(psy_fp_command_params)
			BlockPasteCommandExecute;
		blockpastecommandcommand_vtable.revert =
			(psy_fp_command)
			BlockPasteCommandRevert;
		blockpastecommandcommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &blockpastecommandcommand_vtable;
}

/* implementation */
BlockPasteCommand* blockpastecommand_alloc(psy_audio_Sequence* sequence,
	psy_audio_Pattern* source, psy_audio_SequenceCursor destcursor,
	psy_dsp_big_beat_t bpl, bool mix)
{
	BlockPasteCommand* rv;

	rv = (BlockPasteCommand*)malloc(sizeof(BlockPasteCommand));
	if (rv) {
		psy_command_init(&rv->command);
		blockpastecommandcommand_vtable_init(rv);		
		rv->destcursor = destcursor;
		rv->paste = FALSE;
		rv->sequence = sequence;
		rv->bpl = bpl;
		rv->mix = mix;
		rv->source = source;
		psy_audio_patterns_init(&rv->oldpatterns);
	}
	return rv;
}

void BlockPasteCommandDispose(BlockPasteCommand* self)
{
	psy_audio_patterns_dispose(&self->oldpatterns);
}

void BlockPasteCommandExecute(BlockPasteCommand* self, psy_Property* params)
{
	psy_audio_SequenceTrackIterator it;
	psy_audio_SequenceTrack* track;	
	psy_audio_PatternNode* p;
	psy_audio_PatternNode* prev = 0;
	psy_dsp_big_beat_t dstoffset;
	psy_dsp_big_beat_t srcoffset;
	psy_dsp_big_beat_t currseqoffset;
	intptr_t trackoffset;	
	// psy_audio_SequenceCursor end;
	psy_audio_Pattern* dest;
	psy_audio_SequenceEntry* seqentry;
	psy_audio_OrderIndex curr;	
	uintptr_t order;

	self->selection.topleft = self->destcursor;
	self->selection.bottomright = self->destcursor;
	self->selection.bottomright.offset = self->destcursor.offset +
		psy_audio_pattern_length(self->source);
	order = psy_audio_sequence_order(self->sequence,
		self->selection.bottomright.orderindex.track,
	psy_audio_sequencecursor_offset_abs(&self->selection.bottomright));
	self->selection.bottomright.orderindex.order = order;
	psy_audio_sequencecursor_updateseqoffset(&self->selection.bottomright,
		self->sequence);

	psy_audio_exclusivelock_enter();
	curr = self->selection.topleft.orderindex;
	for (; curr.order <= self->selection.bottomright.orderindex.order; ++curr.order) {
		psy_audio_SequenceEntry* entry;

		entry = psy_audio_sequence_entry(self->sequence, curr);
		if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
			psy_audio_Pattern* pattern;
			psy_audio_SequencePatternEntry* patternentry;

			patternentry = (psy_audio_SequencePatternEntry*)entry;
			pattern = psy_audio_patterns_at(self->sequence->patterns, patternentry->patternslot);
			if (pattern) {
				psy_audio_patterns_insert(&self->oldpatterns, patternentry->patternslot,
					psy_audio_pattern_clone(pattern));
			}
		}
	}

	track = (psy_audio_SequenceTrack*)self->sequence->tracks->entry;
	psy_audio_sequencetrackiterator_init(&it);
	psy_audio_sequence_begin(self->sequence, track,
		psy_audio_sequencecursor_offset_abs(&self->destcursor),
		&it);	
	dstoffset = psy_audio_sequencecursor_pattern_offset(&self->destcursor);
	srcoffset = 0;
	trackoffset = self->destcursor.track;	
	if (it.sequencentrynode) {
		seqentry = (psy_audio_SequenceEntry*)it.sequencentrynode->entry;
	} else {
		seqentry = NULL;
		return;
	}
	p = self->source->events;
	currseqoffset = psy_audio_sequenceentry_offset(seqentry);
	while (p != NULL) {
		psy_audio_PatternEntry* pasteentry;
		psy_audio_PatternNode* node;	
		psy_dsp_big_beat_t pattern_dest_offset;

		pasteentry = psy_audio_patternnode_entry(p);
		pattern_dest_offset = pasteentry->offset + dstoffset;
		while (it.sequencentrynode &&
				!(pattern_dest_offset >= psy_audio_sequenceentry_offset(seqentry) - currseqoffset &&
					pattern_dest_offset < (psy_audio_sequenceentry_offset(seqentry) -
						currseqoffset + psy_audio_sequenceentry_length(seqentry)))) {
			psy_audio_sequencetrackiterator_inc_entry(&it);			
			if (it.sequencentrynode) {
				seqentry = (psy_audio_SequenceEntry*)it.sequencentrynode->entry;
				pattern_dest_offset = pattern_dest_offset -
					(psy_audio_sequenceentry_offset(seqentry) - currseqoffset);				
				currseqoffset = psy_audio_sequenceentry_offset(seqentry);
				dstoffset = -pasteentry->offset;
			} else {
				seqentry = NULL;				
			}
		}
		if (!it.sequencentrynode || !it.pattern) {
			break;
		}		
		dest = it.pattern;
		node = psy_audio_pattern_findnode(dest,
			pasteentry->track + trackoffset,
			pattern_dest_offset,
			(psy_dsp_big_beat_t)self->bpl,
			&prev);
		if (node) {
			psy_audio_PatternEntry* entry;

			entry = (psy_audio_PatternEntry*)node->entry;
			*psy_audio_patternentry_front(entry) =
				*psy_audio_patternentry_front(pasteentry);
		} else {
			psy_audio_pattern_insert(dest,
				prev,
				pasteentry->track + trackoffset,
				pattern_dest_offset,
				psy_audio_patternentry_front(pasteentry));
		}
		p = p->next;
	}	
	psy_audio_sequencetrackiterator_dispose(&it);
	self->paste = TRUE;
	psy_audio_exclusivelock_leave();
}

void BlockPasteCommandRevert(BlockPasteCommand* self)
{
	if (self->paste) {
		psy_audio_exclusivelock_enter();
		psy_audio_OrderIndex curr;		

		curr = self->selection.topleft.orderindex;
		for (; curr.order <= self->selection.bottomright.orderindex.order; ++curr.order) {
			psy_audio_SequenceEntry* entry;

			entry = psy_audio_sequence_entry(self->sequence, curr);
			if (entry && entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				psy_audio_Pattern* pattern;
				psy_audio_SequencePatternEntry* patternentry;

				patternentry = (psy_audio_SequencePatternEntry*)entry;
				pattern = psy_audio_patterns_at(&self->oldpatterns, patternentry->patternslot);
				if (pattern) {
					psy_audio_Pattern* currpattern;
					double length;
					uintptr_t tracks;

					currpattern = psy_audio_patterns_at(self->sequence->patterns,
						patternentry->patternslot);
					length = currpattern->length;
					tracks = currpattern->maxsongtracks;
					psy_audio_pattern_clear(currpattern);
					psy_audio_pattern_copy(currpattern, pattern);
					currpattern->maxsongtracks = tracks;
					currpattern->length = length;
				}
			}
		}
		self->paste = FALSE;
		psy_audio_exclusivelock_leave();
	}
}
