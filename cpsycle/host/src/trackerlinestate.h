// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERLINESTATE)
#define TRACKERLINESTATE

// host
#include "patternviewskin.h"
// audio
#include <pattern.h>
#include <sequence.h>

#ifdef __cplusplus
extern "C" {
#endif

// TrackerLineState
//
// Stores shared data for the Trackergrid and linenumbers

typedef struct TrackerLineState {	
	psy_ui_Value lineheight;
	psy_ui_Value defaultlineheight;
	double lineheightpx;
	double flatsize;
	uintptr_t lpb;
	psy_dsp_big_beat_t bpl;
	bool drawcursor;
	psy_dsp_big_beat_t lastplayposition;
	psy_dsp_big_beat_t sequenceentryoffset;
	uintptr_t trackidx;
	// precomputed
	intptr_t visilines;
	bool cursorchanging;
	// references
	psy_audio_Sequence* sequence;
	psy_audio_Pattern* pattern;
	PatternViewSkin* skin;
	const psy_ui_Font* gridfont;
	bool singlemode;
	uintptr_t maxlines;
} TrackerLineState;

void trackerlinestate_init(TrackerLineState*);
void trackerlinestate_dispose(TrackerLineState*);
uintptr_t trackerlinestate_numlines(const TrackerLineState*);
psy_dsp_big_beat_t trackerlinestate_length(const TrackerLineState*);
bool trackerlinestate_testplaybar(TrackerLineState*, psy_dsp_big_beat_t
	offset);

INLINE intptr_t trackerlinestate_beattoline(const TrackerLineState* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return cast_decimal(offset * self->lpb);
}

INLINE void trackerlinestate_setlpb(TrackerLineState* self, uintptr_t lpb)
{
	self->lpb = lpb;
	self->bpl = 1.0 / lpb;
}

INLINE uintptr_t trackerlinestate_lpb(const TrackerLineState* self)
{
	return self->lpb;	
}

INLINE psy_dsp_big_beat_t trackerlinestate_bpl(const TrackerLineState* self)
{
	return self->bpl;
}

INLINE void trackerlinestate_setpattern(TrackerLineState* self,
	psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
}

INLINE psy_audio_Pattern* trackerlinestate_pattern(TrackerLineState* self)
{
	assert(self);

	return self->pattern;
}

INLINE void trackerlinestate_setsequence(TrackerLineState* self,
	psy_audio_Sequence* sequence)
{
	assert(self);

	self->sequence = sequence;
}

INLINE psy_audio_Sequence* trackerlinestate_sequence(TrackerLineState* self)
{
	assert(self);

	return self->sequence;
}

INLINE psy_dsp_big_beat_t trackerlinestate_quantize(const TrackerLineState*
	self, psy_dsp_big_beat_t position)
{
	assert(self);

	return trackerlinestate_beattoline(self, position) *
		self->bpl;
}

// quantized
INLINE double trackerlinestate_beattopx(TrackerLineState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return self->lineheightpx * trackerlinestate_beattoline(self, position);
}

INLINE double trackerlinestate_linetopx(TrackerLineState* self,
	uintptr_t line)
{
	assert(self);

	return self->lineheightpx * line;
}

INLINE double trackerlinestate_lineheight(TrackerLineState* self)
{
	assert(self);

	return self->lineheightpx;
}

// quantized
INLINE psy_dsp_big_beat_t trackerlinestate_pxtobeat(const TrackerLineState* self,
	double px)
{
	assert(self);

	return trackerlinestate_quantize(self,
		(px / (psy_dsp_big_beat_t)self->lineheightpx) * self->bpl);
}

INLINE psy_dsp_big_beat_t trackerlinestate_pxtobeatnotquantized(TrackerLineState* self, double px)
{
	assert(self);

	return (px / self->lineheightpx) * self->bpl;
}

INLINE bool trackerlinestate_testplayposition(TrackerLineState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	if (self->pattern) {
		return psy_dsp_testrange(position, self->sequenceentryoffset,
			psy_audio_pattern_length(self->pattern));
	}
	return FALSE;
}

void trackerlinestate_clip(TrackerLineState*, const psy_ui_RealRectangle* clip,
	psy_audio_PatternSelection* rv);

INLINE uintptr_t trackerlinestate_midline(const TrackerLineState* self,
	double scrolltop_px)
{
	return trackerlinestate_beattoline(self, trackerlinestate_pxtobeat(
		self, scrolltop_px) + self->visilines / 2);
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERLINESTATE */
