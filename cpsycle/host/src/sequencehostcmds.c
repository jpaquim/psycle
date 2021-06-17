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
					&self->workspace->sequenceselection,
					self->workspace->sequenceselection.editposition,
					patidx)->command);
			break; }
		case psy_audio_SEQUENCEENTRY_SAMPLE: {			
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequencesampleinsertcommand_alloc(self->sequence,
					&self->workspace->sequenceselection,
					self->workspace->sequenceselection.editposition,
					self->workspace->song->samples.selected)->command);
			break; }
		case psy_audio_SEQUENCEENTRY_MARKER: {
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequencemarkerinsertcommand_alloc(self->sequence,
					&self->workspace->sequenceselection,
					self->workspace->sequenceselection.editposition,
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
				self->workspace->sequenceselection.editposition);
			if (entry) {
				psy_undoredo_execute(&self->workspace->undoredo,
					&psy_audio_sequenceinsertcommand_alloc(self->sequence,
						&self->workspace->sequenceselection,
						self->workspace->sequenceselection.editposition,
						entry->patternslot)->command);
			}
			break; }
		case psy_audio_SEQUENCEENTRY_MARKER: {
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequencemarkerinsertcommand_alloc(self->sequence,
					&self->workspace->sequenceselection,
					self->workspace->sequenceselection.editposition,
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
			self->workspace->sequenceselection.editposition);
		if (pattern) {
			psy_audio_Pattern* newpattern;
			uintptr_t patidx;

			newpattern = psy_audio_pattern_clone(pattern);
			patidx = psy_audio_patterns_append(self->patterns, newpattern);
			psy_undoredo_execute(&self->workspace->undoredo,
				&psy_audio_sequenceinsertcommand_alloc(self->sequence,
					&self->workspace->sequenceselection,
					self->workspace->sequenceselection.editposition,
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
				&self->workspace->sequenceselection)->command);
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
				&self->workspace->sequenceselection, 1)->command);
	}
}

void sequencecmds_decpattern(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequencechangepatterncommand_alloc(self->sequence,
				&self->workspace->sequenceselection, -1)->command);
	}
}

void sequencecmds_changepattern(SequenceCmds* self, uintptr_t step)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequencechangepatterncommand_alloc(self->sequence,
				&self->workspace->sequenceselection, step)->command);
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
			&self->workspace->sequenceselection);
	}
}

void sequencecmds_paste(SequenceCmds* self)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		psy_List* p;		

		sequencecmds_update(self);
		for (p = self->workspace->sequencepaste.entries; p != NULL; psy_list_next(&p)) {
			psy_audio_Order* order;
			psy_audio_OrderIndex insertposition;
			psy_audio_SequencePatternEntry* newentry;

			order = (psy_audio_Order*)psy_list_entry(p);
			if (order->entry->type == psy_audio_SEQUENCEENTRY_PATTERN) {
				insertposition = self->workspace->sequenceselection.editposition;
				insertposition.order += order->index.order;
				insertposition.track += order->index.track;
				psy_undoredo_execute(&self->workspace->undoredo,
					&psy_audio_sequenceinsertcommand_alloc(self->sequence,
						&self->workspace->sequenceselection,
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

void sequencecmds_singleselection(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_audio_sequenceselection_setmode(&self->workspace->sequenceselection,
			psy_audio_SEQUENCESELECTION_SINGLE);
		psy_audio_sequenceselection_update(&self->workspace->sequenceselection);
	}
}

void sequencecmds_multiselection(SequenceCmds* self)
{	
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);

		psy_audio_sequenceselection_setmode(&self->workspace->sequenceselection,
			psy_audio_SEQUENCESELECTION_MULTI);
	}
}

void sequencecmds_clear(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		bool playing;
		sequencecmds_update(self);

		playing = psy_audio_player_playing(&self->workspace->player);
		psy_audio_player_stop(&self->workspace->player);
		psy_audio_exclusivelock_enter();
		workspace_clearsequencepaste(self->workspace);
		// no undo/redo
		psy_audio_patterns_clear(&workspace_song(self->workspace)->patterns);
		psy_audio_patterns_insert(&workspace_song(self->workspace)->patterns, 0,
			psy_audio_pattern_allocinit());
		// order can be restored but not patterndata
		// psycle mfc behaviour
		psy_undoredo_execute(&self->workspace->undoredo,
			&psy_audio_sequenceclearcommand_alloc(self->sequence,
				&self->workspace->sequenceselection)->command);
		psy_audio_exclusivelock_leave();
		workspace_setseqeditposition(self->workspace,
			psy_audio_orderindex_make(0, 0));		
		psy_audio_exclusivelock_enter();
		psy_audio_player_setposition(&self->workspace->player, 0.0);		
		psy_audio_exclusivelock_leave();		
	}
}

void sequencecmds_appendtrack(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		sequencecmds_update(self);
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_appendtrack(self->sequence,
			psy_audio_sequencetrack_allocinit());
		psy_audio_exclusivelock_leave();		
		workspace_setseqeditposition(self->workspace,
			psy_audio_orderindex_make(
				psy_audio_sequence_width(&self->workspace->song->sequence) - 1,
				0));
	}
}

void sequencecmds_inserttrack(SequenceCmds* self)
{
	if (workspace_song(self->workspace)) {
		psy_audio_OrderIndex editpos;		

		sequencecmds_update(self);		
		editpos = self->workspace->sequenceselection.editposition;
		psy_audio_exclusivelock_enter();		
		editpos.track = psy_audio_sequence_inserttrack(self->sequence,
			psy_audio_sequencetrack_allocinit(), editpos.track);			
		psy_audio_exclusivelock_leave();		
		workspace_setseqeditposition(self->workspace, editpos);
	}
}

void sequencecmds_deltrack(SequenceCmds* self, uintptr_t trackindex)
{
	if (workspace_song(self->workspace)) {
		psy_audio_SequencePosition position;

		sequencecmds_update(self);
		position = psy_audio_sequence_at(self->sequence, trackindex, 0);
		if (position.tracknode) {
			psy_audio_exclusivelock_enter();
			psy_audio_sequence_removetrack(self->sequence, position.tracknode);
			psy_audio_exclusivelock_leave();
			workspace_setseqeditposition(self->workspace,
				psy_audio_orderindex_make(trackindex, 0));
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
			self->workspace->sequenceselection.editposition);
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_setplayselection(self->sequence,
			&self->workspace->sequenceselection);
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