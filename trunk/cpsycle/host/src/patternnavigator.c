/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternnavigator.h"


/* implementation */
void patternlinenavigator_init(PatternLineNavigator* self, PatternViewState*
	state, bool wraparound)
{
	self->state = state;
	self->wrap = FALSE;
	self->wraparound = wraparound;	
}

psy_audio_SequenceCursor patternlinenavigator_up(PatternLineNavigator* self,
	uintptr_t lines, psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;	
	
	assert(self);
	assert(self->state);
	
	self->wrap = FALSE;
	rv = cursor;	
	if (lines > 0) {		
		psy_audio_SequenceEntry* seqentry;
		
		seqentry = psy_audio_sequence_entry(self->state->sequence,
			rv.order_index);
		if (seqentry) {
			intptr_t currlines;
			double bpl;
			psy_dsp_big_beat_t offset;
			
			bpl = 1.0 / (double)rv.lpb;						
			currlines = cast_decimal(rv.offset / bpl);		
			offset = (currlines - (intptr_t)lines) * bpl;
			if (offset < 0.0) {
				if (patternviewstate_single_mode(self->state)) {			
					if (self->wraparound) {						
						rv = patternlinenavigator_end(self, rv);
						self->wrap = TRUE;						
					}					
				} else {
					uintptr_t order;
					double abs_offset;
					
					abs_offset = psy_audio_sequenceentry_offset(seqentry) +
						offset;					
					order = psy_audio_sequence_order(self->state->sequence,
						rv.order_index.track, abs_offset);
					if (order != psy_INDEX_INVALID) {
						psy_audio_sequencecursor_set_order_index(&rv,
							psy_audio_orderindex_make(rv.order_index.track,
								order));
						seqentry = psy_audio_sequence_entry(
							self->state->sequence, rv.order_index);
						assert(seqentry);
						offset = abs_offset - psy_audio_sequenceentry_offset(
							seqentry);
						psy_audio_sequencecursor_set_offset(&rv, offset);
					} else if (self->wraparound) {
						rv = patternlinenavigator_end(self, rv);
						self->wrap = TRUE;						
					}
				}				
			} else {
				psy_audio_sequencecursor_set_offset(&rv, offset);
			}			
		}				
	}
	return rv;
}

psy_audio_SequenceCursor patternlinenavigator_down(
	PatternLineNavigator* self, uintptr_t lines,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);	
	assert(self->state);

	self->wrap = FALSE;
	rv = cursor;		
	if (lines > 0) {
		psy_audio_SequenceEntry* seqentry;
		int currlines;
		double bpl;
				
		bpl = 1.0 / (double)rv.lpb;
		currlines = cast_decimal(rv.offset / bpl);
		seqentry = psy_audio_sequence_entry(self->state->sequence,
			rv.order_index);
		if (seqentry) {					
			double offset;
			
			offset = (currlines + lines) * bpl;		
			if (offset >= psy_audio_sequenceentry_length(seqentry)) {				
				if (patternviewstate_single_mode(self->state)) {
					if (self->wraparound) {
						offset = 0.0;
						self->wrap = TRUE;
					} else {
						offset = psy_audio_sequencecursor_offset(&rv);
					}
				} else {
					uintptr_t order;
					double abs_offset;
					
					abs_offset = psy_audio_sequenceentry_offset(seqentry) +
						offset;
					order = psy_audio_sequence_order(self->state->sequence,
						rv.order_index.track, abs_offset);
					if (order != psy_INDEX_INVALID) {
						psy_audio_sequencecursor_set_order_index(&rv,
							psy_audio_orderindex_make(rv.order_index.track,
								order));
						seqentry = psy_audio_sequence_entry(
							self->state->sequence, rv.order_index);
						assert(seqentry);
						offset = abs_offset - psy_audio_sequenceentry_offset(
							seqentry);
					} else if (self->wraparound) {
						psy_audio_sequencecursor_set_order_index(&rv,
							psy_audio_orderindex_make(0, 0));
						offset = 0.0;							
						self->wrap = TRUE;
					}
				}
			}
			psy_audio_sequencecursor_set_offset(&rv, offset);
		}
	}
	return rv;
}

psy_audio_SequenceCursor patternlinenavigator_home(PatternLineNavigator* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;	

	assert(self);

	rv = cursor;
	self->wrap = FALSE;	
	if (!patternviewstate_single_mode(self->state)) {
		psy_audio_sequencecursor_set_order_index(&rv,
		psy_audio_orderindex_make(0, 0));
	}
	psy_audio_sequencecursor_set_offset(&rv, 0.0);
	return rv;
}

psy_audio_SequenceCursor patternlinenavigator_end(PatternLineNavigator* self,
	psy_audio_SequenceCursor cursor)
{	
	psy_audio_SequenceCursor rv;
	psy_audio_SequenceEntry* seqentry;
	double bpl;

	assert(self);

	rv = cursor;
	self->wrap = FALSE;
	seqentry = NULL;
	bpl = 1.0 / (double)rv.lpb;
	if (patternviewstate_single_mode(self->state)) {				
		seqentry = psy_audio_sequence_entry(self->state->sequence,
			rv.order_index);
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
void patterncolnavigator_init(PatternColNavigator* self, TrackerState* state, bool wraparound)
{
	self->state = state;
	self->wrap = FALSE;
	self->wraparound = wraparound;
}

psy_audio_SequenceCursor patterncolnavigator_prev_track(PatternColNavigator* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);
	assert(self->state);

	self->wrap = FALSE;
	rv = cursor;
	rv.column = rv.digit = 0;	
	if (rv.track > 0) {
		--rv.track;		
	} else if (self->wraparound) {
		rv.track = patternviewstate_num_song_tracks(self->state->pv) - 1;
		self->wrap = TRUE;
	}
	return rv;
}

psy_audio_SequenceCursor patterncolnavigator_next_track(PatternColNavigator* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);
	assert(self->state);

	self->wrap = FALSE;
	rv = cursor;
	rv.column = rv.digit = 0;
	if (rv.track < patternviewstate_num_song_tracks(self->state->pv) - 1) {
		++rv.track;
	} else if (self->wraparound) {
		rv.track = 0;
		self->wrap = TRUE;
	}
	return rv;
}

psy_audio_SequenceCursor patterncolnavigator_next_col(PatternColNavigator* self,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;
	int invalidate = 1;
	const psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return cursor;
	}
	rv = cursor;
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
			if (rv.track < patternviewstate_num_song_tracks(self->state->pv) - 1) {
				rv.column = rv.digit = rv.noteindex = 0;
				++rv.track;					
			} else if (self->wraparound) {
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
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;
	const psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	assert(self);

	if (!pattern) {
		return cursor;
	}
	rv = cursor;
	self->wrap = FALSE;
	if ((rv.column == 0) || (self->state->track_config->multicolumn &&
		rv.noteindex > 0 &&
		rv.column == PATTERNEVENT_COLUMN_CMD) &&
		rv.digit == 0) {
		if (rv.noteindex > 0) {
			TrackDef* trackdef;

			--rv.noteindex;
			trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
			rv.column = trackdef_num_columns(trackdef) - 1;
			rv.digit = trackdef_num_digits(trackdef,
				rv.column) - 1;
		} else if (rv.track > 0) {
			TrackDef* trackdef;

			--rv.track;
			trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
			rv.noteindex = trackdef_visinotes(trackdef) - 1;
			rv.column = trackdef_num_columns(trackdef) - 1;
			rv.digit = trackdef_num_digits(trackdef,
				rv.column) - 1;			
		} else if (self->wraparound) {
			TrackDef* trackdef;

			rv.track = patternviewstate_num_song_tracks(self->state->pv) - 1;
			trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
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
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;
	const psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return cursor;
	}
	rv = cursor;
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
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;
	TrackDef* trackdef;
	TrackColumnDef* columndef;
	const psy_audio_Pattern* pattern;

	assert(self);

	pattern = patternviewstate_pattern(self->state->pv);
	if (!pattern) {
		return cursor;
	}
	rv = cursor;
	self->wrap = FALSE;

	trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
	columndef = trackdef_column_def(trackdef, rv.column);
	if (rv.track != patternviewstate_num_song_tracks(self->state->pv) - 1 ||
		rv.digit != columndef->numdigits - 1 ||
		rv.column != PATTERNEVENT_COLUMN_PARAM) {
		if (rv.column == PATTERNEVENT_COLUMN_PARAM &&
			rv.digit == columndef->numdigits - 1) {
			rv.track = patternviewstate_num_song_tracks(self->state->pv) - 1;
			trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
			columndef = trackdef_column_def(trackdef, PATTERNEVENT_COLUMN_PARAM);
			rv.column = PATTERNEVENT_COLUMN_PARAM;
			rv.digit = columndef->numdigits - 1;			
		} else {
			trackdef = trackerconfig_trackdef(self->state->track_config, rv.track);
			columndef = trackdef_column_def(trackdef, PATTERNEVENT_COLUMN_PARAM);
			rv.column = PATTERNEVENT_COLUMN_PARAM;
			rv.digit = columndef->numdigits - 1;
		}		
	}
	return rv;
}
