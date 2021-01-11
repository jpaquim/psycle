// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL_H)
#define PIANOROLL_H

// host
#include "patternviewskin.h"
#include "workspace.h"
#include "zoombox.h"
// ui
#include <uiscroller.h>
#include <uilabel.h>
#include <uicombobox.h>

#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pianoroll
//
// The Pianoroll is another way than the normal tracker view to enter notes.
// It displays a Pattern selected by the SequenceView in a roll with a piano
// keyboard at the left and the time line to the right.

typedef enum {
	KEYBOARDTYPE_KEYS,
	KEYBOARDTYPE_NOTES,
	KEYBOARDTYPE_DRUMS,
	KEYBOARDTYPE_NUM
} KeyboardType;

typedef struct KeyboardState {
	uint8_t keymin;
	uint8_t keymax;
	psy_dsp_NotesTabMode notemode;
	bool drawpianokeys;
	psy_ui_Value keyheight;
	double keyheightpx;
	double keyboardheightpx;
	psy_ui_Value defaultkeyheight;
	// references
	PatternViewSkin* skin;
} KeyboardState;

void keyboardstate_init(KeyboardState*, PatternViewSkin* skin);

INLINE intptr_t keyboardstate_numkeys(KeyboardState* self)
{
	assert(self);

	return self->keymax - self->keymin;
}

INLINE double keyboardstate_height(KeyboardState* self,
	psy_ui_TextMetric* tm)
{
	assert(self);

	return keyboardstate_numkeys(self) *
		psy_ui_value_px(&self->keyheight, tm);
}

INLINE double keyboardstate_keytopx(KeyboardState* self, intptr_t key)
{
	assert(self);

	return self->keyboardheightpx - (key + 1) * self->keyheightpx;
}

INLINE uint8_t keyboardstate_pxtokey(KeyboardState* self, double px)
{
	assert(self);

	if (self->keymax - 1 >= px / self->keyheightpx) {
		return (uint8_t)(self->keymax - px / self->keyheightpx);
	}
	return 0;
}

INLINE void pianokeyboardstate_clip(KeyboardState* self,
	double clip_top_px, double clip_bottom_px,
	uint8_t* rv_keymin, uint8_t* rv_keymax)
{
	assert(self);

	*rv_keymin = psy_max(self->keymin, (uint8_t)
		((self->keyboardheightpx - clip_bottom_px) / self->keyheightpx - 1));
	*rv_keymax = psy_min(self->keymax, (uint8_t)
		((self->keyboardheightpx - clip_top_px + self->keyheightpx) /
		self->keyheightpx));
}

typedef enum {
	PIANOROLL_TRACK_DISPLAY_ALL,
	PIANOROLL_TRACK_DISPLAY_CURRENT,
	PIANOROLL_TRACK_DISPLAY_ACTIVE
} PianoTrackDisplay;

typedef struct PianoGridState {
	psy_audio_PatternCursor cursor;
	uintptr_t lpb;	
	double pxperbeat;
	double defaultbeatwidth;
	// references
	psy_audio_Pattern* pattern;
	PatternViewSkin* skin;
} PianoGridState;

void pianogridstate_init(PianoGridState*, PatternViewSkin* skin);

INLINE void pianogridstate_setpattern(PianoGridState* self, psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
}

INLINE psy_audio_Pattern* pianogridstate_pattern(PianoGridState* self)
{
	assert(self);

	return self->pattern;
}

INLINE psy_audio_PatternCursor pianogridstate_setcursor(PianoGridState* self,
	psy_audio_PatternCursor cursor)
{
	assert(self);

	self->cursor = cursor;
}

INLINE psy_audio_PatternCursor pianogridstate_cursor(const PianoGridState* self)
{
	assert(self);

	return self->cursor;
}

INLINE void pianogridstate_setlpb(PianoGridState* self, uintptr_t lpb)
{
	assert(self);

	self->lpb = lpb;
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

	return (intptr_t)(position * self->lpb);
}

INLINE psy_dsp_big_beat_t pianogridstate_quantize(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return pianogridstate_beattosteps(self, position) *
		(1 / (psy_dsp_big_beat_t)self->lpb);
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

	return 1 / (psy_dsp_big_beat_t)self->lpb;
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
	if (pianogridstate_pattern(self)) {
		*rv_right = psy_min(
			psy_audio_pattern_length(pianogridstate_pattern(self)),
			pianogridstate_pxtobeat(self, clip_right_px));
	} else {
		*rv_right = 0;
	}
}

// Header (Beatruler)
typedef struct {
	// inherits
	psy_ui_Component component;	
	// references
	PianoGridState* gridstate;
	PianoGridState defaultgridstate;
} PianoRuler;

void pianoruler_init(PianoRuler*, psy_ui_Component* parent, PianoGridState*);
void pianoruler_setsharedgridstate(PianoRuler*, PianoGridState*);

INLINE psy_ui_Component* pianoruler_base(PianoRuler* self)
{
	assert(self);

	return &self->component;
}

// Keyboard
typedef struct PianoKeyboard {
	// inherits
	psy_ui_Component component;
	// references
	KeyboardState* keyboardstate;
	KeyboardState defaultkeyboardstate;
} PianoKeyboard;

void pianokeyboard_init(PianoKeyboard*, psy_ui_Component* parent,
	KeyboardState*);
void pianokeyboard_setsharedkeyboardstate(PianoKeyboard*, KeyboardState*);
void pianokeyboard_setkeyboardtype(PianoKeyboard*, KeyboardType);

INLINE psy_ui_Component* pianokeyboard_base(PianoKeyboard* self)
{
	return &self->component;
}

typedef struct {
	uint8_t note;
	psy_dsp_big_beat_t offset;
	uintptr_t track;
	// draw hover
	bool hover;
	// draw noterelease
	bool noterelease;
	// event exists	
	bool active;
} PianogridTrackEvent;

typedef struct PianoGridDraw {
	psy_ui_TextMetric tm;
	psy_ui_RealSize size;
	bool cursorchanging;
	bool cursoronnoterelease;
	psy_dsp_big_beat_t sequenceentryoffset;	
	PianoTrackDisplay trackdisplay;
	psy_ui_Value scrolltop;
	psy_ui_Value scrollleft;
	psy_audio_PatternSelection selection;
	// references
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
	psy_ui_Value scrollleft, psy_ui_Value scrolltop,	
	psy_dsp_big_beat_t sequenceentryoffset,
	psy_audio_PatternEntry* hoverpatternentry,	
	PianoTrackDisplay,
	bool cursorchanging, bool cursoronnoterelease,
	psy_audio_PatternSelection selection,
	psy_ui_RealSize, psy_ui_TextMetric, Workspace*);
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

typedef struct Pianogrid {
	// inherits
   psy_ui_Component component;
   // internal data
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
   // references
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

typedef struct PianoBar {
	// inherits
	psy_ui_Component component;
	ZoomBox zoombox_keyheight;
	psy_ui_Label keys;
	psy_ui_ComboBox keytype;
	psy_ui_Label tracks;
	psy_ui_Button tracks_all;
	psy_ui_Button track_curr;
	psy_ui_Button tracks_active;
	psy_ui_Button blockmenu;
	// references
	Workspace* workspace;
} PianoBar;

void pianobar_init(PianoBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* pianobar_base(PianoBar* self)
{
	assert(self);

	return &self->component;
}

typedef struct Pianoroll {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component top;
	PianoRuler header;
	psy_ui_Component left;
	ZoomBox zoombox_beatwidth;   
	Pianogrid grid;
	psy_ui_Scroller scroller;
	PianoBar bar;
	// internal data
	KeyboardState keyboardstate;
	PianoKeyboard keyboard;
	PianoGridState gridstate;	
	uintptr_t opcount;
	bool syncpattern;
	// references	
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

// block operations
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
