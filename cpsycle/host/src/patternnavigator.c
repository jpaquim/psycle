/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternnavigator.h"


/* implementation */
void patternlinenavigator_init(PatternLineNavigator* self, PatternViewState*
	state, bool wrap_around)
{
	self->state = state;
	self->wrap = FALSE;
	self->wrap_around = wrap_around;	
}

psy_audio_SequenceCursor patternlinenavigator_up(PatternLineNavigator* self,
	uintptr_t lines, const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	psy_audio_SequenceEntry* seqentry;
	psy_audio_Sequence* sequence;
	
	assert(self);
	assert(self->state);
	assert(cursor);
	
	self->wrap = FALSE;	
	if (lines == 0) {
		return *cursor;
	}
	sequence = patternviewstate_sequence(self->state);
	if (!sequence) {
		return *cursor;
	}
	rv = *cursor;				
	seqentry = psy_audio_sequence_entry(sequence,
		psy_audio_sequencecursor_order_index(&rv));
	if (seqentry) {
		intptr_t currlines;
		psy_dsp_big_beat_t bpl;
		psy_dsp_big_beat_t offset;
		
		bpl = psy_audio_sequencecursor_bpl(&rv);
		currlines = cast_decimal(psy_audio_sequencecursor_offset(&rv) / bpl);		
		offset = (currlines - (intptr_t)lines) * bpl;
		if (offset < 0.0) {
			if (patternviewstate_single_mode(self->state)) {			
				if (self->wrap_around) {						
					rv = patternlinenavigator_end(self, &rv);
					self->wrap = TRUE;						
				} else {
					rv = patternlinenavigator_home(self, &rv);					
				}					
			} else {						
				psy_dsp_big_beat_t abs_offset;
				psy_audio_OrderIndex order_index;
				
				abs_offset = psy_audio_sequenceentry_offset(seqentry) + offset;
				order_index = psy_audio_sequencecursor_order_index(&rv);
				order_index.order = psy_audio_sequence_order(sequence,
					order_index.track, abs_offset);
				if (order_index.order != psy_INDEX_INVALID) {
					psy_audio_sequencecursor_set_order_index(&rv, order_index);
					seqentry = psy_audio_sequence_entry(sequence, order_index);
					assert(seqentry);
					psy_audio_sequencecursor_set_offset(&rv, abs_offset -
						psy_audio_sequenceentry_offset(seqentry));					
				} else if (self->wrap_around) {
					rv = patternlinenavigator_end(self, &rv);
					self->wrap = TRUE;						
				} else {
					rv = patternlinenavigator_home(self, &rv);
				}
			}				
		} else {
			psy_audio_sequencecursor_set_offset(&rv, offset);
		}			
	}
	return rv;
}

psy_audio_SequenceCursor patternlinenavigator_down(
	PatternLineNavigator* self, uintptr_t lines,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	psy_audio_SequenceEntry* seqentry;
	psy_audio_Sequence* sequence;	

	assert(self);	
	assert(self->state);
	assert(cursor);

	self->wrap = FALSE;	
	if (lines == 0) {
		return *cursor;
	}
	sequence = patternviewstate_sequence(self->state);
	if (!sequence) {
		return *cursor;
	}
	rv = *cursor;	
	seqentry = psy_audio_sequence_entry(sequence,
		psy_audio_sequencecursor_order_index(&rv));
	if (seqentry) {
		psy_dsp_big_beat_t bpl;
		psy_dsp_big_beat_t offset;		
		intptr_t currlines;
		
		bpl = psy_audio_sequencecursor_bpl(&rv);
		currlines = cast_decimal(psy_audio_sequencecursor_offset(&rv) / bpl);
		offset = (currlines + lines) * bpl;		
		if (offset >= psy_audio_sequenceentry_length(seqentry)) {
			if (patternviewstate_single_mode(self->state)) {
				if (self->wrap_around) {
					offset -= psy_audio_sequenceentry_length(seqentry);
					if (offset > psy_audio_sequenceentry_length(seqentry)) {
						offset = psy_audio_sequenceentry_length(seqentry) - bpl;
					}
					psy_audio_sequencecursor_set_offset(&rv, offset);
					self->wrap = TRUE;
				} else {
					rv = patternlinenavigator_end(self, &rv);
				}
			} else {
				psy_dsp_big_beat_t abs_offset;
				psy_audio_OrderIndex order_index;
				
				abs_offset = psy_audio_sequenceentry_offset(seqentry) + offset;
				order_index = psy_audio_sequencecursor_order_index(&rv);
				order_index.order = psy_audio_sequence_order(sequence,
					order_index.track, abs_offset);
				if (order_index.order != psy_INDEX_INVALID) {
					psy_audio_sequencecursor_set_order_index(&rv, order_index);
					seqentry = psy_audio_sequence_entry(sequence, order_index);
					assert(seqentry);
					psy_audio_sequencecursor_set_offset(&rv, abs_offset -
						psy_audio_sequenceentry_offset(seqentry));					
				} else if (self->wrap_around) {
					rv = patternlinenavigator_home(self, &rv);
					self->wrap = TRUE;						
				} else {
					rv = patternlinenavigator_end(self, &rv);
				}				
			}
		} else {
			psy_audio_sequencecursor_set_offset(&rv, offset);
		}	
	}
	return rv;
}

psy_audio_SequenceCursor patternlinenavigator_home(PatternLineNavigator* self,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;	

	assert(self);
	assert(self->state);
	assert(cursor);
	
	rv = *cursor;
	self->wrap = FALSE;	
	if (!patternviewstate_single_mode(self->state)) {
		psy_audio_sequencecursor_set_order_index(&rv,
			psy_audio_orderindex_make(0, 0));
	}
	psy_audio_sequencecursor_set_offset(&rv, 0.0);
	return rv;
}

psy_audio_SequenceCursor patternlinenavigator_end(PatternLineNavigator* self,
	const psy_audio_SequenceCursor* cursor)
{	
	psy_audio_SequenceCursor rv;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceEntry* seqentry;
	psy_dsp_big_beat_t bpl;

	assert(self);
	assert(self->state);
	assert(cursor);

	sequence = patternviewstate_sequence(self->state);
	if (!sequence) {
		return *cursor;
	}
	rv = *cursor;
	self->wrap = FALSE;
	seqentry = NULL;
	bpl = psy_audio_sequencecursor_bpl(&rv);
	if (patternviewstate_single_mode(self->state)) {				
		seqentry = psy_audio_sequence_entry(sequence,
			psy_audio_sequencecursor_order_index(&rv));
		assert(seqentry);		
	} else {
		const psy_audio_SequenceTrack* seqtrack;		
		uintptr_t num_orders;
		
		seqtrack = psy_audio_sequence_track_at_const(self->state->sequence,
			rv.order_index.track);
		assert(seqtrack);
		num_orders = psy_audio_sequencetrack_size(seqtrack);		
		if (num_orders != 0) {			
			psy_audio_OrderIndex last;
			
			last = psy_audio_orderindex_make(rv.order_index.track,
				num_orders - 1);
			seqentry = psy_audio_sequence_entry(self->state->sequence, last);
			assert(seqentry);
			psy_audio_sequencecursor_set_order_index(&rv, last);			
		}
	}
	if (seqentry) {
		psy_audio_sequencecursor_set_offset(&rv,			
			psy_audio_sequenceentry_length(seqentry) - bpl);
	}
	return rv;
}

/* PatternColNavigator */

/* implementation */
void patterncolnavigator_init(PatternColNavigator* self, TrackerState* state,
	bool wrap_around)
{
	assert(self);
	
	self->state = state;
	self->wrap = FALSE;
	self->wrap_around = wrap_around;
}

psy_audio_SequenceCursor patterncolnavigator_prev_track(
	PatternColNavigator* self, const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);
	assert(self->state);
	assert(cursor);

	rv = *cursor;
	self->wrap = FALSE;	
	rv.column = rv.digit = 0;	
	if (rv.track > 0) {
		--rv.track;		
	} else if (self->wrap_around) {
		rv.track = patternviewstate_num_song_tracks(self->state->pv) - 1;
		self->wrap = TRUE;
	}
	return rv;
}

psy_audio_SequenceCursor patterncolnavigator_next_track(
	PatternColNavigator* self, const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);
	assert(self->state);
	assert(cursor);

	rv = *cursor;
	self->wrap = FALSE;	
	rv.column = rv.digit = 0;
	if (rv.track < patternviewstate_num_song_tracks(self->state->pv) - 1) {
		++rv.track;
	} else if (self->wrap_around) {
		rv.track = 0;
		self->wrap = TRUE;
	}
	return rv;
}

psy_audio_SequenceCursor patterncolnavigator_next_col(PatternColNavigator* self,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;
	int invalidate = 1;
	const psy_audio_Pattern* pattern;

	assert(self);
	assert(cursor);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return *cursor;
	}
	rv = *cursor;
	self->wrap = FALSE;
	trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
	if (rv.column == trackdef_num_columns(trackdef) - 1 &&
			rv.digit == trackdef_num_digits(trackdef, rv.column) - 1) {
		if (rv.noteindex + 1 < trackdef_visinotes(trackdef)) {
			++rv.noteindex;
			if (!self->state->track_config->multicolumn) {
				rv.column = 0;
			} else {
				rv.column = PATTERNEVENT_COLUMN_CMD;
			}
			rv.digit = 0;
		} else {
			if (rv.track < patternviewstate_num_song_tracks(self->state->pv) -
					1) {
				rv.column = rv.digit = rv.noteindex = 0;
				++rv.track;					
			} else if (self->wrap_around) {
				rv.column = rv.digit = rv.track = rv.noteindex = 0;
				self->wrap = TRUE;
			}
		}
	} else {
		++rv.digit;
		if (rv.digit >= trackdef_num_digits(trackdef, rv.column)) {
			++rv.column;
			rv.digit = 0;
		}
	}	
	return rv;	
}

psy_audio_SequenceCursor patterncolnavigator_prev_col(PatternColNavigator* self,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	const psy_audio_Pattern* pattern;

	assert(self);
	assert(cursor);

	pattern = patternviewstate_pattern(self->state->pv);
	assert(self);

	if (!pattern) {
		return *cursor;
	}
	rv = *cursor;
	self->wrap = FALSE;
	if ((rv.column == 0) || (self->state->track_config->multicolumn &&
		rv.noteindex > 0 &&
		rv.column == PATTERNEVENT_COLUMN_CMD) &&
		rv.digit == 0) {
		if (rv.noteindex > 0) {
			TrackDef* trackdef;

			--rv.noteindex;
			trackdef = trackerconfig_trackdef(self->state->track_config,
				rv.track);
			rv.column = trackdef_num_columns(trackdef) - 1;
			rv.digit = trackdef_num_digits(trackdef,
				rv.column) - 1;
		} else if (rv.track > 0) {
			TrackDef* trackdef;

			--rv.track;
			trackdef = trackerconfig_trackdef(self->state->track_config,
				rv.track);
			rv.noteindex = trackdef_visinotes(trackdef) - 1;
			rv.column = trackdef_num_columns(trackdef) - 1;
			rv.digit = trackdef_num_digits(trackdef,
				rv.column) - 1;			
		} else if (self->wrap_around) {
			TrackDef* trackdef;

			rv.track = patternviewstate_num_song_tracks(self->state->pv) - 1;
			trackdef = trackerconfig_trackdef(self->state->track_config,
				rv.track);
			rv.column = trackdef_num_columns(trackdef) - 1;
			rv.digit = trackdef_num_digits(trackdef,
				rv.column) - 1;
			self->wrap = TRUE;
		}
	} else if (rv.digit > 0) {
		--rv.digit;
	} else {
		TrackDef* trackdef;

		trackdef = trackerconfig_trackdef(self->state->track_config,
			rv.track);
		--rv.column;
		rv.digit = trackdef_num_digits(trackdef,
			rv.column) - 1;
	}
	return rv;
}

psy_audio_SequenceCursor patterncolnavigator_home(PatternColNavigator* self,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	const psy_audio_Pattern* pattern;

	assert(self);
	assert(cursor);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return *cursor;
	}
	rv = *cursor;
	self->wrap = FALSE;

	if (rv.column != 0) {
		rv.column = 0;
	} else {
		rv.track = 0;
		rv.column = 0;
	}
	return rv;
}

psy_audio_SequenceCursor patterncolnavigator_end(PatternColNavigator* self,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;
	TrackColumnDef* columndef;
	const psy_audio_Pattern* pattern;

	assert(self);
	assert(cursor);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return *cursor;
	}
	rv = *cursor;
	self->wrap = FALSE;

	trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
	columndef = trackdef_column_def(trackdef, rv.column);
	if (rv.track != patternviewstate_num_song_tracks(self->state->pv) - 1 ||
		rv.digit != columndef->numdigits - 1 ||
		rv.column != PATTERNEVENT_COLUMN_PARAM) {
		if (rv.column == PATTERNEVENT_COLUMN_PARAM &&
			rv.digit == columndef->numdigits - 1) {
			rv.track = patternviewstate_num_song_tracks(self->state->pv) - 1;
			trackdef = trackerconfig_trackdef(self->state->track_config,
				rv.track);
			columndef = trackdef_column_def(trackdef,
				PATTERNEVENT_COLUMN_PARAM);
			rv.column = PATTERNEVENT_COLUMN_PARAM;
			rv.digit = columndef->numdigits - 1;			
		} else {
			trackdef = trackerconfig_trackdef(self->state->track_config,
				rv.track);
			columndef = trackdef_column_def(trackdef,
				PATTERNEVENT_COLUMN_PARAM);
			rv.column = PATTERNEVENT_COLUMN_PARAM;
			rv.digit = columndef->numdigits - 1;
		}		
	}
	return rv;
}


/* PatternKeyNavigator */

/* implementation */
void patternkeynavigator_init(PatternKeyNavigator* self, KeyboardState* state,
	bool wrap_around)
{
	self->state = state;
	self->wrap = FALSE;
	self->wrap_around = wrap_around;	
}

psy_audio_SequenceCursor patternkeynavigator_up(PatternKeyNavigator* self,
	uintptr_t keys, const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	
	assert(self);
	assert(cursor);
	
	rv = *cursor;
	self->wrap = FALSE;	
	if (keys > 0) {
		if (rv.key >= keys) {
			rv.key -= keys;
		} else {
			if (self->wrap_around) {
				rv.key += self->state->keymax;
				self->wrap =TRUE;
			} else {
				rv.key = 0;
			}
		}
	}	
	return rv;
}

psy_audio_SequenceCursor patternkeynavigator_down(
	PatternKeyNavigator* self, uintptr_t keys,
	const psy_audio_SequenceCursor* cursor)
{
	psy_audio_SequenceCursor rv;
	
	assert(self);
	assert(cursor);
	
	rv = *cursor;	
	self->wrap = FALSE;
	if (keys > 0) {
		rv.key += keys;
		if (rv.key >= self->state->keymax) {
			if (self->wrap_around) {
				rv.key = rv.key - self->state->keymax;
				if (rv.key > self->state->keymax - 1) {
					rv.key = self->state->keymax - 1;
				}				
			} else {
				rv.key = self->state->keymax - 1;
				self->wrap = TRUE;
			}
		}
	}	
	return rv;
}

/* PatternSelect */

void patternselect_init(PatternSelect* self, PatternViewState* state)
{
	self->state = state;	
}

void patternselect_select_col(PatternSelect* self)
{
	psy_audio_SequenceCursor top;
	psy_audio_SequenceCursor bottom;
	
	assert(self);
	
	if (patternviewstate_single_mode(self->state)) {				
		top = psy_audio_sequencecursor_make(
			self->state->cursor.order_index,
			self->state->cursor.track, 
			0.0);
		bottom = psy_audio_sequencecursor_make(
			self->state->cursor.order_index,
			self->state->cursor.track + 1,
			patternviewstate_length(self->state));
	} else {		
		const psy_audio_SequenceTrack* seqtrack;		
		const psy_audio_SequenceEntry* last_entry;
		uintptr_t num_orders;
		
		seqtrack = psy_audio_sequence_track_at_const(self->state->sequence,
			self->state->cursor.order_index.track);
		if (!seqtrack) {
			return;
		}
		num_orders = psy_audio_sequencetrack_size(seqtrack);		
		if (num_orders == 0) {
			return;
		}
		last_entry = psy_audio_sequence_entry(self->state->sequence,
			psy_audio_orderindex_make(self->state->cursor.order_index.track,
				num_orders - 1));
		if (!last_entry) {
			return;
		}
		top = psy_audio_sequencecursor_make(
			psy_audio_orderindex_make(self->state->cursor.order_index.track, 0),
			self->state->cursor.track,
			0.0);
		bottom = psy_audio_sequencecursor_make(
			psy_audio_orderindex_make(
				self->state->cursor.order_index.track,
				num_orders -1),			
			self->state->cursor.track + 1,
			psy_audio_sequenceentry_length(last_entry));			
	}
	self->state->selection = psy_audio_blockselection_make(top, bottom);	
	patternviewstate_invalidate(self->state);	
}

void patternselect_select_bar(PatternSelect* self)
{		
	psy_audio_SequenceCursor top;
	psy_audio_SequenceCursor bottom;
	const psy_audio_SequenceEntry* seq_entry;
	
	assert(self);
		
	seq_entry = psy_audio_sequence_entry(self->state->sequence,
		psy_audio_sequencecursor_order_index(&self->state->cursor));
	if (!seq_entry) {
		return;
	}
	top = self->state->cursor;
	bottom = psy_audio_sequencecursor_make(
		self->state->cursor.order_index,
		self->state->cursor.track + 1,
		psy_min(
			psy_audio_sequencecursor_offset(&self->state->cursor) + 4.0,
			psy_audio_sequenceentry_length(seq_entry)));	
	self->state->selection = psy_audio_blockselection_make(top, bottom);	
	patternviewstate_invalidate(self->state);	
}

void patternselect_select_all(PatternSelect* self)
{
	psy_audio_SequenceCursor top;
	psy_audio_SequenceCursor bottom;
	
	assert(self);
	
	if (patternviewstate_single_mode(self->state)) {				
		top = psy_audio_sequencecursor_make(
			self->state->cursor.order_index,
			0, 
			0.0);
		bottom = psy_audio_sequencecursor_make(
			self->state->cursor.order_index,
			patternviewstate_num_song_tracks(self->state),
			patternviewstate_length(self->state));
	} else {
		const psy_audio_SequenceTrack* seqtrack;		
		const psy_audio_SequenceEntry* last_entry;
		uintptr_t num_orders;
		
		seqtrack = psy_audio_sequence_track_at_const(self->state->sequence,
			self->state->cursor.order_index.track);
		if (!seqtrack) {
			return;
		}
		num_orders = psy_audio_sequencetrack_size(seqtrack);		
		if (num_orders == 0) {
			return;
		}
		last_entry = psy_audio_sequence_entry(self->state->sequence,
			psy_audio_orderindex_make(self->state->cursor.order_index.track,
				num_orders - 1));
		if (!last_entry) {
			return;
		}
		top = psy_audio_sequencecursor_make(
			psy_audio_orderindex_make(self->state->cursor.order_index.track, 0),
			0,
			0.0);
		bottom = psy_audio_sequencecursor_make(
			psy_audio_orderindex_make(
				self->state->cursor.order_index.track,
				num_orders -1),			
			patternviewstate_num_song_tracks(self->state),
			psy_audio_sequenceentry_length(last_entry));			
	}
	self->state->selection = psy_audio_blockselection_make(top, bottom);	
	patternviewstate_invalidate(self->state);	
}
