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
	psy_audio_Pattern* pattern;
	PatternViewSkin* skin;
	int lineheight;
	int lpb;
	int drawcursor;
	psy_dsp_big_beat_t lastplayposition;
	psy_dsp_big_beat_t sequenceentryoffset;	
	// precomputed
	int visilines;
	bool cursorchanging;
} TrackerLineState;

void trackerlinestate_init(TrackerLineState*);
void trackerlinestate_dispose(TrackerLineState*);
int trackerlinestate_offsettoscreenline(TrackerLineState*,
	psy_dsp_big_beat_t);
double trackerlinestate_offset(TrackerLineState*, int y, unsigned int* lines);
int trackerlinestate_numlines(TrackerLineState*);
int trackerlinestate_testplaybar(TrackerLineState* self,
	psy_dsp_big_beat_t offset);

#ifdef __cplusplus
}
#endif

#endif /* TRACKERLINESTATE */
