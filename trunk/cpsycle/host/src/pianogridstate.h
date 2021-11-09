/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOGRIDSTATE_H)
#define PIANOGRIDSTATE_H

/* host */
#include "patternviewskin.h"
#include "patternviewstate.h"
/* audio */
#include <sequence.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PianoGridState */
typedef struct PianoGridState {
	PatternViewState pv;	
	double pxperbeat;
	double defaultbeatwidth;	
} PianoGridState;

void pianogridstate_init(PianoGridState*, PatternViewSkin*, psy_audio_Song*);


INLINE psy_audio_SequenceCursor pianogridstate_setcursor(PianoGridState* self,
	psy_audio_SequenceCursor cursor)
{
	assert(self);

	self->pv.cursor = cursor;
}

INLINE psy_audio_SequenceCursor pianogridstate_cursor(const PianoGridState* self)
{
	assert(self);

	return self->pv.cursor;
}

INLINE void pianogridstate_setzoom(PianoGridState* self, psy_dsp_big_beat_t rate)
{
	assert(self);

	self->pxperbeat = self->defaultbeatwidth * rate;
}

INLINE intptr_t pianogridstate_beattosteps(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return (intptr_t)(position * self->pv.cursor.lpb);
}

INLINE psy_dsp_big_beat_t pianogridstate_quantize(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return pianogridstate_beattosteps(self, position) *
		(1 / (psy_dsp_big_beat_t)self->pv.cursor.lpb);
}

INLINE psy_dsp_big_beat_t pianogridstate_pxtobeat(const PianoGridState* self, double px)
{
	assert(self);

	return (psy_dsp_big_beat_t)(px / self->pxperbeat);
}

INLINE double pianogridstate_beattopx(const PianoGridState* self, psy_dsp_big_beat_t position)
{
	assert(self);

	return position * self->pxperbeat;
}

INLINE double pianogridstate_quantizebeattopx(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return pianogridstate_beattopx(self, pianogridstate_quantize(self,
		position));
}

INLINE psy_dsp_big_beat_t pianogridstate_step(const PianoGridState* self)
{
	assert(self);

	return 1 / (psy_dsp_big_beat_t)self->pv.cursor.lpb;
}

INLINE double pianogridstate_steppx(const PianoGridState* self)
{
	assert(self);

	return pianogridstate_beattopx(self, pianogridstate_step(self));
}

INLINE psy_dsp_big_beat_t pianogridstate_stepstobeat(PianoGridState* self,
	intptr_t steps)
{
	assert(self);

	return steps * pianogridstate_step(self);
}

INLINE void pianogridstate_clip(PianoGridState* self,
	double clip_left_px, double clip_right_px,
	psy_dsp_big_beat_t* rv_left, psy_dsp_big_beat_t* rv_right)
{
	assert(self);
	assert(rv_left && rv_right);

	*rv_left = pianogridstate_quantize(self,
		pianogridstate_pxtobeat(self, clip_left_px));
	if (patternviewstate_pattern(&self->pv)) {
		*rv_right = psy_min(
			psy_audio_pattern_length(patternviewstate_pattern(&self->pv)),
			pianogridstate_pxtobeat(self, clip_right_px));
	} else {
		*rv_right = 0;
	}
}

#ifdef __cplusplus
}
#endif

#endif /* PIANOGRIDSTATE_H */
