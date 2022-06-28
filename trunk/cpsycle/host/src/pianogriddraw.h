/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOGRIDDRAW_H)
#define PIANOROLL_H

/* host */
#include "patternhostcmds.h"
#include "pianokeyboardstate.h"
#include "pianogridstate.h"
#include "workspace.h"
/* ui */
#include <uicomponent.h>
/* audio */
#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Pianogriddraw
*/

typedef enum {
	PIANOROLL_TRACK_DISPLAY_ALL,
	PIANOROLL_TRACK_DISPLAY_CURRENT,
	PIANOROLL_TRACK_DISPLAY_ACTIVE
} PianoTrackDisplay;

/* PianogridTrackEvent */
typedef struct PianogridTrackEvent {
	uint8_t note;
	psy_dsp_big_beat_t offset;	
	uintptr_t track;
	/* draw hover */
	bool hover;
	/* draw noterelease */
	bool noterelease;
	/* event exists */
	bool active;
} PianogridTrackEvent;

/* PianoGridDraw */
typedef struct PianoGridDraw {
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;	
	bool cursoronnoterelease;
	psy_dsp_big_beat_t sequenceentryoffset;	
	PianoTrackDisplay trackdisplay;	
	/* references */
	KeyboardState* keyboardstate;
	PianoGridState* gridstate;
	Workspace* workspace;
	psy_audio_PatternEntry* hoverpatternentry;
	bool drawgrid;
	bool drawentries;
	bool drawcursor;
	bool drawplaybar;	
} PianoGridDraw;

void pianogriddraw_init(PianoGridDraw*,
	KeyboardState*, PianoGridState*,	
	psy_dsp_big_beat_t sequenceentryoffset,
	psy_audio_PatternEntry* hoverpatternentry,	
	PianoTrackDisplay,
	bool cursoronnoterelease,	
	psy_ui_RealSize, Workspace*);

void pianogriddraw_ondraw(PianoGridDraw*, psy_ui_Graphics*);

INLINE void pianogriddraw_preventgrid(PianoGridDraw* self)
{
	self->drawgrid = FALSE;
}

INLINE void pianogriddraw_preventplaybar(PianoGridDraw* self)
{
	self->drawplaybar = FALSE;
}

INLINE void pianogriddraw_preventcursor(PianoGridDraw* self)
{
	self->drawcursor = FALSE;
}

INLINE void pianogriddraw_preventeventdraw(PianoGridDraw* self)
{
	self->drawentries = FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* PIANOGRIDDRAW_H */
