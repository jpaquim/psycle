// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencehostcmds.h"
// host
#include "workspace.h"
// audio
#include <exclusivelock.h>
#include <sequencecmds.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// SequenceCmds
// implementation
void sequencecmds_init(SequenceCmds* self, Workspace* workspace)
{
	self->workspace = workspace;
	self->player = &self->workspace->player;
	sequencecmds_update(self);
}

void sequencecmds_newentry(SequenceCmds* self,
	psy_audio_SequenceEntryType type)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);
		switch (type) {
		case psy_audio_SEQUENCEENTRY_PATTERN: {
			psy_audio_Pattern* newpattern;
			uintptr_t patidx;

			newpattern = psy_audio_pattern_allocinit();
			// change length to default lines
			psy_audio_pattern_setlength(newpattern,
				(psy_dsp_big_beat_t)
				(psy_audio_pattern_defaultlines() /
					(psy_audio_player_lpb(workspace_player(self->workspace)))));
			patidx = psy_audio_patterns_append(self->patterns, newpattern);
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequenceinsertcommand_alloc(self->sequence,
					&self->workspace->song->sequence.selection,
					psy_audio_sequenceselection_first(
						&self->workspace->song->sequence.selection),
					patidx)->command);
			break; }
		case psy_audio_SEQUENCEENTRY_SAMPLE: {			
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequencesampleinsertcommand_alloc(self->sequence,
					&self->workspace->song->sequence.selection,
					psy_audio_sequenceselection_first(
						&self->workspace->song->sequence.selection),
					self->workspace->song->samples.selected)->command);
			break; }
		case psy_audio_SEQUENCEENTRY_MARKER: {
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequencemarkerinsertcommand_alloc(self->sequence,
					&self->workspace->song->sequence.selection,
					psy_audio_sequenceselection_first(
						&self->workspace->song->sequence.selection),
					"Untitled")->command);
			break; }
		default:
			break;
		}
	}
}

void sequencecmds_insertentry(SequenceCmds* self,
	psy_audio_SequenceEntryType type)
{
	if (workspace_song(self->workspace)) {		
		sequencecmds_update(self);
		switch (type) {
		case psy_audio_SEQUENCEENTRY_PATTERN: {
			psy_audio_SequencePatternEntry* entry;

			entry = (psy_audio_SequencePatternEntry*)
				psy_audio_sequence_entry(self->sequence,
				psy_audio_sequenceselection_first(
					&self->workspace->song->sequence.selection));
			if (entry) {
				psy_undoredo_execute(&self->workspace->undoredo,
					&psy_audio_sequenceinsertcommand_alloc(self->sequence,
						&self->workspace->song->sequence.selection,
						psy_audio_sequenceselection_first(
							&self->workspace->song->sequence.selection),
						entry->patternslot)->command);
			}
			break; }
		case psy_audio_SEQUENCEENTRY_MARKER: {
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequencemarkerinsertcommand_alloc(self->sequence,
					&self->workspace->song->sequence.selection,
					psy_audio_sequenceselection_first(
						&self->workspace->song->sequence.selection),
					"Untitled")->command);
			break; }
		default:
			break;
		}
	}
}

void sequencecmds_cloneentry(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Pattern* pattern;

		sequencecmds_update(self);
		pattern = psy_audio_sequence_pattern(self->sequence,
			psy_audio_sequenceselection_first(&self->workspace->song->sequence.selection));
		if (pattern) {
			psy_audio_Pattern* newpattern;
			uintptr_t patidx;

			newpattern = psy_audio_pattern_clone(pattern);
			patidx = psy_audio_patterns_append(self->patterns, newpattern);
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequenceinsertcommand_alloc(self->sequence,
					&self->workspace->song->sequence.selection,
					psy_audio_sequenceselection_first(&self->workspace->song->sequence.selection),
					patidx)->command);
		}
	}
}

void sequencecmds_delentry(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		bool playing;
		sequencecmds_update(self);
		
		playing = psy_audio_player_playing(&self->workspace->player);
		psy_audio_player_stop(&self->workspace->player);
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceremovecommand_alloc(self->sequence,
				&self->workspace->song->sequence.selection)->command);
		if (playing) {
			psy_audio_exclusivelock_enter();		
			psy_audio_player_setposition(&self->workspace->player,
				psy_audio_player_position(&self->workspace->player));
			psy_audio_player_start(&self->workspace->player);
			psy_audio_exclusivelock_leave();
		}
	}
}

void sequencecmds_incpattern(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequencechangepatterncommand_alloc(self->sequence,
				&self->workspace->song->sequence.selection, 1)->command);
	}
}

void sequencecmds_decpattern(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequencechangepatterncommand_alloc(self->sequence,
				&self->workspace->song->sequence.selection, -1)->command);
	}
}

void sequencecmds_changepattern(SequenceCmds* self, uintptr_t step)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequencechangepatterncommand_alloc(self->sequence,
				&self->workspace->song->sequence.selection, step)->command);
	}
}

void sequencecmds_update(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		self->patterns = &workspace_song(self->workspace)->patterns;
		self->sequence = &workspace_song(self->workspace)->sequence;
	} else {
		self->patterns = NULL;
		self->sequence = NULL;
	}
}

void sequencecmds_copy(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);
		psy_audio_sequencepaste_copy(
			&self->workspace->sequencepaste,
			self->sequence,
			&self->workspace->song->sequence.selection);
	}
}

void sequencecmds_paste(SequenceCmds* self)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		psy_List* p;		

		sequencecmds_update(self);
		for (p = self->workspace->sequencepaste.nodes; p != NULL; psy_list_next(&p)) {
			psy_audio_Order* order;
			psy_audio_OrderIndex insertposition;
			psy_audio_SequencePatternEntry* newentry;

			order = (psy_audio_Order*)psy_list_entry(p);
			if (order->entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				insertposition = psy_audio_sequenceselection_first(&self->workspace->song->sequence.selection);
				insertposition.order += order->index.order;
				insertposition.track += order->index.track;
				psy_undoredo_execute(&self->workspace->undoredo,
					&psy_audio_sequenceinsertcommand_alloc(self->sequence,
						&self->workspace->song->sequence.selection,
						insertposition,
						((psy_audio_SequencePatternEntry*)order->entry)->patternslot)->command);
				newentry = (psy_audio_SequencePatternEntry*)
					psy_audio_sequence_entry(self->sequence, insertposition);
				if (newentry) {
					*newentry = *((psy_audio_SequencePatternEntry*)(order->entry));
				}
			}
		}
	}
}

void sequencecmds_clear(SequenceCmds* self)
{
	if (self->sequence) {		
		sequencecmds_update(self);
		assert(self->sequence);
		psy_audio_player_stop(&self->workspace->player);
		psy_audio_exclusivelock_enter();		
		workspace_clear_sequence_paste(self->workspace);
		/* no undo / redo */
		psy_audio_patterns_clear(self->sequence->patterns);
		psy_audio_patterns_insert(self->sequence->patterns, 0,
			psy_audio_pattern_allocinit());
		/*
		** order can be restored but not patterndata
		** psycle mfc behaviour
		*/
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceclearcommand_alloc(self->sequence,
				&self->sequence->selection)->command);		
		psy_audio_exclusivelock_leave();		
	}
}

void sequencecmds_append_track(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_append_track(self->sequence,
			psy_audio_sequencetrack_allocinit());
		psy_audio_exclusivelock_leave();		
		psy_audio_sequence_set_cursor(self->sequence,
			psy_audio_sequencecursor_make(psy_audio_orderindex_make(
					psy_audio_sequence_width(&self->workspace->song->sequence) - 1,
					psy_INDEX_INVALID),		
				0, 0.0));
	}
}

void sequencecmds_insert_track(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_OrderIndex editpos;		

		sequencecmds_update(self);		
		editpos = psy_audio_sequenceselection_first(
			&self->workspace->song->sequence.selection);
		psy_audio_exclusivelock_enter();		
		editpos.track = psy_audio_sequence_set_track(self->sequence,
			psy_audio_sequencetrack_allocinit(), editpos.track);			
		psy_audio_exclusivelock_leave();	
		psy_audio_sequenceselection_select_first(
			&self->workspace->song->sequence.selection, editpos);
	}
}

void sequencecmds_remove_track(SequenceCmds* self, uintptr_t trackindex)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);
		if (trackindex != psy_INDEX_INVALID) {
			psy_audio_SequenceCursor cursor;

			psy_audio_exclusivelock_enter();
			psy_audio_sequence_remove_track(self->sequence, trackindex);
			psy_audio_exclusivelock_leave();
			cursor = self->sequence->cursor;
			if (trackindex >= psy_audio_sequence_width(self->sequence)) {
				psy_audio_OrderIndex index;
				
				index = cursor.order_index;
				if (psy_audio_sequence_width(self->sequence) > 0) {
					index.track = psy_audio_sequence_width(self->sequence) - 1;
				} else {
					index.track = psy_INDEX_INVALID;
				}
				if (index.order >= psy_audio_sequence_track_size(self->sequence, trackindex)) {
					if (psy_audio_sequence_track_size(self->sequence, trackindex) > 0) {
						index.order = psy_audio_sequence_track_size(self->sequence, trackindex) - 1;
					} else {
						index.order = psy_INDEX_INVALID;
					}
				}
				psy_audio_sequencecursor_set_order_index(&cursor, index);		
			}	
			psy_audio_sequence_set_cursor(self->sequence, cursor);
		}
	}
}

void sequencecmds_changeplayposition(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_SequenceEntry* entry;
		psy_dsp_big_beat_t startposition;

		sequencecmds_update(self);
		entry = psy_audio_sequence_entry(self->sequence,
			psy_audio_sequenceselection_first(
				&self->workspace->song->sequence.selection));
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_setplayselection(self->sequence,
			&self->workspace->song->sequence.selection);
		psy_audio_exclusivelock_leave();
		if (entry) {
			startposition = entry->offset;
			psy_audio_exclusivelock_enter();
			psy_audio_player_stop(&self->workspace->player);
			psy_audio_player_setposition(&self->workspace->player, startposition);
			psy_audio_player_start(&self->workspace->player);
			psy_audio_exclusivelock_leave();
		}
	}
}
