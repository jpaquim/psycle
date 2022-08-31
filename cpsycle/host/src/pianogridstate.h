/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOGRIDSTATE_H)
#define PIANOGRIDSTATE_H

/* host */
#include "patternviewstate.h"
/* audio */
#include <sequence.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PianoTrackDisplay {
	PIANOROLL_TRACK_DISPLAY_ALL,
	PIANOROLL_TRACK_DISPLAY_CURRENT,
	PIANOROLL_TRACK_DISPLAY_ACTIVE
} PianoTrackDisplay;

/* PianoGridState */
typedef struct PianoGridState {
	PatternViewState* pv;	
	BeatConvert beat_convert;
	double default_line_extent;
	psy_Property track_display;
	psy_Property* track_all;
	psy_Property* track_current;
	psy_Property* track_active;
} PianoGridState;

void pianogridstate_init(PianoGridState*, PatternViewState*);
void pianogridstate_dispose(PianoGridState*);


INLINE void pianogridstate_set_zoom(PianoGridState* self, psy_dsp_big_beat_t rate)
{
	assert(self);

	self->beat_convert.line_px = self->default_line_extent * rate;
}

INLINE intptr_t pianogridstate_beat_to_line(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{	
	assert(self);
	
	return beatline_beat_to_line(self->beat_convert.beat_line, position);	
}

/* quantized */
INLINE psy_dsp_big_beat_t pianogridstate_px_to_beat(
	const PianoGridState* self, double px)
{	
	assert(self);
	
	return beatconvert_px_to_beat(&self->beat_convert, px);	
}

/* quantized */
INLINE double pianogridstate_beat_to_px(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);
	
	return beatconvert_beat_to_px(&self->beat_convert, position);
}

/* not quantized */
INLINE double pianogridstate_raw_beat_to_px(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);
	
	return beatconvert_raw_beat_to_px(&self->beat_convert, position);
}

INLINE psy_dsp_big_beat_t pianogridstate_step(const PianoGridState* self)
{
	assert(self);

	return psy_audio_sequencecursor_bpl(patternviewstate_cursor(self->pv));
}

INLINE double pianogridstate_steppx(const PianoGridState* self)
{
	assert(self);

	return self->beat_convert.line_px;
}

INLINE void pianogridstate_clip(PianoGridState* self,
	double clip_left_px, double clip_right_px,
	psy_dsp_big_beat_t* rv_left, psy_dsp_big_beat_t* rv_right)
{
	assert(self);
	assert(rv_left && rv_right);

	*rv_left = pianogridstate_px_to_beat(self, clip_left_px) +
		((patternviewstate_single_mode(self->pv))
		? psy_audio_sequencecursor_seqoffset(&self->pv->cursor, self->pv->sequence)
		: 0.0);
	if (patternviewstate_pattern(self->pv)) {
		*rv_right = psy_min(			
			patternviewstate_length(self->pv) +
			((patternviewstate_single_mode(self->pv))
				? psy_audio_sequencecursor_seqoffset(&self->pv->cursor, self->pv->sequence)
				: 0.0),
			pianogridstate_px_to_beat(self, clip_right_px) +
			((patternviewstate_single_mode(self->pv))
			? psy_audio_sequencecursor_seqoffset(&self->pv->cursor, self->pv->sequence)
			: 0.0));
	} else {
		*rv_right = 0.0;
	}
}

PianoTrackDisplay pianogridstate_track_display(const PianoGridState*);

#ifdef __cplusplus
}
#endif

#endif /* PIANOGRIDSTATE_H */
