/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PIANOROLL_H)
#define PIANOROLL_H

/* host */
#include "patternhostcmds.h"
#include "pianokeyboard.h"
#include "pianoruler.h"
#include "workspace.h"
#include "zoombox.h"
/* ui */
#include <uiscroller.h>
#include <uilabel.h>
#include <uicombobox.h>

#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** Pianoroll
**
** The Pianoroll is another way than the normal tracker view to enter notes.
** It displays a Pattern selected by the SeqView in a roll with a piano
** keyboard at the left and the time line to the right.
*/

typedef enum {
	PIANOROLL_TRACK_DISPLAY_ALL,
	PIANOROLL_TRACK_DISPLAY_CURRENT,
	PIANOROLL_TRACK_DISPLAY_ACTIVE
} PianoTrackDisplay;


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

typedef struct PianoGridDraw {
	const psy_ui_TextMetric* tm;
	psy_ui_RealSize size;
	bool cursorchanging;
	bool cursoronnoterelease;
	psy_dsp_big_beat_t sequenceentryoffset;	
	PianoTrackDisplay trackdisplay;	
	psy_audio_PatternSelection selection;
	/* references */
	KeyboardState* keyboardstate;
	PianoGridState* gridstate;
	Workspace* workspace;
	psy_audio_PatternEntry* hoverpatternentry;
	bool drawgrid;
	bool drawentries;
	bool drawcursor;
	bool drawplaybar;
	bool clip;
} PianoGridDraw;

void pianogriddraw_init(PianoGridDraw*,
	KeyboardState*, PianoGridState*,	
	psy_dsp_big_beat_t sequenceentryoffset,
	psy_audio_PatternEntry* hoverpatternentry,	
	PianoTrackDisplay,
	bool cursorchanging, bool cursoronnoterelease,
	psy_audio_PatternSelection selection,
	psy_ui_RealSize, const psy_ui_TextMetric*, Workspace*);
void pianogriddraw_ondraw(PianoGridDraw*, psy_ui_Graphics*);

INLINE void pianogriddraw_preventclip(PianoGridDraw* self)
{
	self->clip = FALSE;
}

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

/* Pianogrid */
typedef struct Pianogrid {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	PianoGridState defaultgridstate;   
	KeyboardState defaultkeyboardstate;
	psy_audio_PatternEntry* hoverpatternentry;   
	psy_dsp_big_beat_t sequenceentryoffset;
	psy_dsp_big_beat_t lastplayposition;
	psy_audio_PatternCursor oldcursor;
	psy_audio_PatternCursor dragcursor;
	bool cursoronnoterelease;
	PianoTrackDisplay trackdisplay;
	bool cursorchanging;
	psy_audio_PatternSelection selection;
	psy_audio_PatternCursor dragselectionbase;
	psy_audio_PatternCursor lastdragcursor;
	intptr_t pgupdownstep;	
	/* references */
	KeyboardState* keyboardstate;
	PianoGridState* gridstate;
	Workspace* workspace;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardState*,
	PianoGridState*, Workspace*);
void pianogrid_setsharedgridstate(Pianogrid*, PianoGridState*);
void pianogrid_setsharedkeyboardstate(Pianogrid*, KeyboardState*);
void pianogrid_invalidateline(Pianogrid*, psy_dsp_big_beat_t offset);
void pianogrid_invalidatecursor(Pianogrid*);
void pianogrid_setcursor(Pianogrid*, psy_audio_PatternCursor);
void pianogrid_storecursor(Pianogrid*);
void pianogrid_onpatternchange(Pianogrid*, psy_audio_Pattern*);
void pianogrid_settrackdisplay(Pianogrid*, PianoTrackDisplay);

INLINE void pianogrid_setpgupdownstep(Pianogrid* self, intptr_t step)
{
	self->pgupdownstep = step;
}

INLINE PianoTrackDisplay pianogrid_trackdisplay(const Pianogrid* self)
{
	assert(self);

	return self->trackdisplay;
}

INLINE const psy_audio_PatternSelection* pianogrid_selection(const Pianogrid* self)
{
	assert(self);

	return &self->selection;
}

INLINE psy_ui_Component* pianogrid_base(Pianogrid* self)
{
	assert(self);

	return &self->component;
}

/* PianoBar */
typedef struct PianoBar {
	/* inherits */
	psy_ui_Component component;
	ZoomBox zoombox_keyheight;
	psy_ui_Label keys;
	psy_ui_ComboBox keytype;
	psy_ui_Label tracks;
	psy_ui_Button tracks_all;
	psy_ui_Button track_curr;
	psy_ui_Button tracks_active;
	psy_ui_Button blockmenu;
	/* references */
	Workspace* workspace;
} PianoBar;

void pianobar_init(PianoBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* pianobar_base(PianoBar* self)
{
	assert(self);

	return &self->component;
}

/* Pianoroll */
typedef struct Pianoroll {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component top;
	PianoRuler header;
	psy_ui_Component left;
	ZoomBox zoombox_beatwidth;   
	Pianogrid grid;
	psy_ui_Scroller scroller;
	PianoBar bar;	
	KeyboardState keyboardstate;
	psy_ui_Component keyboardpane;
	PianoKeyboard keyboard;
	PianoGridState gridstate;	
	uintptr_t opcount;
	bool syncpattern;
	PatternCmds cmds;
	/* references */
	Workspace* workspace;
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, PatternViewSkin*,
	Workspace*);
void pianoroll_setpattern(Pianoroll*, psy_audio_Pattern*);
void pianoroll_updatescroll(Pianoroll*);
void pianoroll_makecmds(psy_Property* parent);
bool pianoroll_handlecommand(Pianoroll*, uintptr_t cmd);

INLINE void pianoroll_setpgupdownstep(Pianoroll* self, intptr_t step)
{
	pianogrid_setpgupdownstep(&self->grid, step);
}

/* block operations */
void pianoroll_navup(Pianoroll*);
void pianoroll_navdown(Pianoroll*);
void pianoroll_enter(Pianoroll*);
void pianoroll_rowclear(Pianoroll*);
void pianoroll_blockcut(Pianoroll*);
void pianoroll_blockcopy(Pianoroll*);
void pianoroll_blockpaste(Pianoroll*);
void pianoroll_blockdelete(Pianoroll*);
void pianoroll_selectall(Pianoroll*);
void pianoroll_selectbar(Pianoroll*);
void pianoroll_blockunmark(Pianoroll*);
void pianoroll_blockstart(Pianoroll*);
void pianoroll_blockend(Pianoroll*);

INLINE psy_ui_Component* pianoroll_base(Pianoroll* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PIANOROLL_H */
