/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternnavigator.h"

/* PatternLineNavigator */

/* prototypes */
static void patternlinenavigator_update_order(PatternLineNavigator*,
	psy_audio_SequenceCursor*);

/* implementation */
void patternlinennavigator_init(PatternLineNavigator* self, PatternViewState*
	state)
{
	self->state = state;
	self->wrap = FALSE;
}

psy_audio_SequenceCursor patternlinennavigator_up(PatternLineNavigator* self,
	uintptr_t lines, psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);
	assert(self->state);
	
	self->wrap = FALSE;
	rv = cursor;
	if (lines > 0) {
		int currlines;
		double bpl;
		psy_dsp_big_beat_t maxlength;

		maxlength = patternviewstate_length(self->state);
		bpl = 1.0 / (double)rv.lpb;
		currlines = cast_decimal(rv.offset / bpl);		
		psy_audio_sequencecursor_setoffset(&rv,
			(currlines - (intptr_t)lines) * bpl);
		if (rv.offset < 0.0) {
			if (self->state->wraparound) {
				psy_audio_sequencecursor_setoffset(&rv,
					rv.offset + maxlength);
				if (rv.offset < 0) {
					psy_audio_sequencecursor_setoffset(&rv, 0.0);
				}
				self->wrap = TRUE;				
			} else {
				psy_audio_sequencecursor_setoffset(&rv, 0.0);
			}
		}		
		patternlinenavigator_update_order(self, &rv);		
	}
	return rv;
}

psy_audio_SequenceCursor patternlinennavigator_down(
	PatternLineNavigator* self, uintptr_t lines,
	psy_audio_SequenceCursor cursor)
{
	psy_audio_SequenceCursor rv;

	assert(self);	
	assert(self->state);

	self->wrap = FALSE;
	rv = cursor;
	if (lines > 0) {
		int currlines;
		double bpl;
		psy_dsp_big_beat_t maxlength;		

		maxlength = patternviewstate_length(self->state);
		bpl = 1.0 / (double)rv.lpb;
		currlines = cast_decimal(rv.offset / bpl);
		psy_audio_sequencecursor_setoffset(&rv,
			(currlines + lines) * bpl);
		if (rv.offset >= maxlength) {
			if (self->state->wraparound) {
				psy_audio_sequencecursor_setoffset(&rv,
					rv.offset - maxlength);
				if (rv.offset > maxlength - bpl) {
					psy_audio_sequencecursor_setoffset(&rv,
						maxlength - bpl);
				}
				self->wrap = TRUE;
			} else {
				psy_audio_sequencecursor_setoffset(&rv,
					maxlength - bpl);
			}
		}		
		patternlinenavigator_update_order(self, &rv);				
	}
	return rv;
}

psy_audio_SequenceCursor patternlinennavigator_home(PatternLineNavigator* self,
	psy_audio_SequenceCursor cursor)
{
	assert(self);

	psy_audio_SequenceCursor rv;

	rv = cursor;
	self->wrap = FALSE;
	psy_audio_sequencecursor_setoffset(&rv, 0.0);		
	patternlinenavigator_update_order(self, &rv);
	return rv;
}

psy_audio_SequenceCursor patternlinennavigator_end(PatternLineNavigator* self,
	psy_audio_SequenceCursor cursor)
{
	assert(self);

	psy_audio_SequenceCursor rv;

	rv = cursor;
	self->wrap = FALSE;
	psy_audio_sequencecursor_setoffset(&rv,
		patternviewstate_length(self->state) -
		patternviewstate_bpl(self->state));
	patternlinenavigator_update_order(self, &rv);
	return rv;
}

void patternlinenavigator_update_order(PatternLineNavigator* self,
	psy_audio_SequenceCursor* cursor)
{
	if (cursor->absolute) {
		psy_audio_Sequence* sequence;

		sequence = patternviewstate_sequence(self->state);
		if (sequence) {
			uintptr_t order;

			order = psy_audio_sequence_order(sequence, cursor->track,
				cursor->offset);
			cursor->orderindex.order = order;
			psy_audio_sequencecursor_updateseqoffset(cursor, sequence);
		}
	}
}


/* PatternColNavigator */

/* implementation */
void patterncolnavigator_init(PatternColNavigator* self, TrackerState* state)
{
	self->state = state;
	self->wrap = FALSE;
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
	} else if (self->state->pv->wraparound) {
		rv.track = patternviewstate_numsongtracks(self->state->pv) - 1;
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
	if (rv.track < patternviewstate_numsongtracks(self->state->pv) - 1) {
		++rv.track;
	} else if (self->state->pv->wraparound) {
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

	assert(self);

	if (!self->state->pv->pattern) {
		return cursor;
	}
	rv = cursor;
	self->wrap = FALSE;
	trackdef = trackerstate_trackdef(self->state, rv.track);
	if (rv.column == trackdef_numcolumns(trackdef) - 1 &&
			rv.digit == trackdef_numdigits(trackdef, rv.column) - 1) {
		if (rv.noteindex + 1 < trackdef_visinotes(trackdef)) {
			++rv.noteindex;
			if (!self->state->trackconfig->multicolumn) {
				rv.column = 0;
			} else {
				rv.column = PATTERNEVENT_COLUMN_CMD;
			}
			rv.digit = 0;
		} else {
			if (rv.track < patternviewstate_numsongtracks(self->state->pv) - 1) {
				rv.column = 0;
				rv.digit = 0;
				rv.noteindex = 0;
				++rv.track;					
			} else if (self->state->pv->wraparound) {
				rv.column = 0;
				rv.digit = 0;
				rv.track = 0;
				rv.noteindex = 0;
				self->wrap = TRUE;
			}
		}
	} else {
		++rv.digit;
		if (rv.digit >= trackdef_numdigits(trackdef, rv.column)) {
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

	assert(self);

	if (!self->state->pv->pattern) {
		return cursor;
	}
	rv = cursor;
	self->wrap = FALSE;

	assert(self);

	if ((rv.column == 0) || (self->state->trackconfig->multicolumn &&
		rv.noteindex > 0 &&
		rv.column == PATTERNEVENT_COLUMN_CMD) &&
		rv.digit == 0) {
		if (rv.noteindex > 0) {
			TrackDef* trackdef;

			--rv.noteindex;
			trackdef = trackerstate_trackdef(self->state, rv.track);
			rv.column = trackdef_numcolumns(trackdef) - 1;
			rv.digit = trackdef_numdigits(trackdef,
				rv.column) - 1;
		} else if (rv.track > 0) {
			TrackDef* trackdef;

			--rv.track;
			trackdef = trackerstate_trackdef(self->state, rv.track);
			rv.noteindex = trackdef_visinotes(trackdef) - 1;
			rv.column = trackdef_numcolumns(trackdef) - 1;
			rv.digit = trackdef_numdigits(trackdef,
				rv.column) - 1;			
		} else if (self->state->pv->wraparound) {
			TrackDef* trackdef;

			rv.track = patternviewstate_numsongtracks(self->state->pv) - 1;
			trackdef = trackerstate_trackdef(self->state, rv.track);
			rv.column = trackdef_numcolumns(trackdef) - 1;
			rv.digit = trackdef_numdigits(trackdef,
				rv.column) - 1;
			self->wrap = TRUE;
		}
	} else if (rv.digit > 0) {
		--rv.digit;
	} else {
		TrackDef* trackdef;

		trackdef = trackerstate_trackdef(self->state,
			rv.track);
		--rv.column;
		rv.digit = trackdef_numdigits(trackdef,
			rv.column) - 1;
	}
	return rv;
}
