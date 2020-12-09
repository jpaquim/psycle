// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERLINESTATE)
#define TRACKERLINESTATE

// host
#include "patternviewskin.h"
// audio
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

// TrackerLineState
//
// Stores shared data for the Trackergrid and linenumbers

typedef struct TrackerLineState {	
	int lineheight;
	uintptr_t lpb;
	psy_dsp_big_beat_t bpl;
	int drawcursor;
	psy_dsp_big_beat_t lastplayposition;
	psy_dsp_big_beat_t sequenceentryoffset;	
	// precomputed
	int visilines;
	bool cursorchanging;
	// references
	psy_audio_Pattern* pattern;
	PatternViewSkin* skin;
} TrackerLineState;

void trackerlinestate_init(TrackerLineState*);
void trackerlinestate_dispose(TrackerLineState*);
int trackerlinestate_beattoline(TrackerLineState*,
	psy_dsp_big_beat_t);
int trackerlinestate_numlines(TrackerLineState*);
int trackerlinestate_testplaybar(TrackerLineState* self,
	psy_dsp_big_beat_t offset);

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

INLINE psy_dsp_big_beat_t trackerlinestate_quantize(TrackerLineState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return trackerlinestate_beattoline(self, position) *
		self->bpl;
}

// quantized
INLINE int trackerlinestate_beattopx(TrackerLineState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return self->lineheight * trackerlinestate_beattoline(self, position);
}

INLINE int trackerlinestate_linetopx(TrackerLineState* self,
	uintptr_t line)
{
	assert(self);

	return self->lineheight * line;
}

INLINE int trackerlinestate_lineheight(TrackerLineState* self)
{
	return self->lineheight;
}

// quantized
INLINE psy_dsp_big_beat_t trackerlinestate_pxtobeat(TrackerLineState* self, int px)
{
	assert(self);

	return trackerlinestate_quantize(self,
		(px / (psy_dsp_big_beat_t)self->lineheight) * self->bpl);
}

INLINE psy_dsp_big_beat_t trackerlinestate_pxtobeatnotquantized(TrackerLineState* self, int px)
{
	assert(self);

	return (px / (psy_dsp_big_beat_t)self->lineheight) * self->bpl;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERLINESTATE */
