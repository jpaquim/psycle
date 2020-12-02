// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL)
#define PIANOROLL

#include "patternviewskin.h"
#include "workspace.h"
#include "zoombox.h"

#include <uiscroller.h>
#include <uilabel.h>
#include <uicombobox.h>

#include <pattern.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KEYBOARDTYPE_KEYS,
	KEYBOARDTYPE_NOTES,
	KEYBOARDTYPE_DRUMS,
	KEYBOARDTYPE_NUM
} KeyboardType;

typedef struct {
	intptr_t keymin;
	intptr_t keymax;
	psy_dsp_NotesTabMode notemode;
	bool drawpianokeys;
	psy_ui_Value keyheight;
	int keyheightpx;
	int keyboardheightpx;
	psy_ui_Value defaultkeyheight;
	PatternViewSkin* skin;	
} KeyboardState;

void keyboardstate_init(KeyboardState*, PatternViewSkin* skin);

INLINE intptr_t keyboardstate_numkeys(KeyboardState* self)
{
	assert(self);

	return self->keymax - self->keymin;
}

INLINE int keyboardstate_height(KeyboardState* self,
	psy_ui_TextMetric* tm)
{
	assert(self);

	return keyboardstate_numkeys(self) * psy_ui_value_px(&self->keyheight, tm);
}

INLINE int keyboardstate_keytopx(KeyboardState* self, intptr_t key)
{
	assert(self);

	return (int)(self->keyboardheightpx - (key + 1) * self->keyheightpx);
}

INLINE intptr_t keyboardstate_pxtokey(KeyboardState* self, int px)
{
	assert(self);

	return self->keymax - 1 - px / self->keyheightpx;
}

INLINE void pianokeyboardstate_clip(KeyboardState* self,
	int clip_top_px, int clip_bottom_px,
	uint8_t* rv_keymin, uint8_t* rv_keymax)
{
	assert(self);

	*rv_keymin = psy_max(self->keymin,
		(self->keyboardheightpx - clip_bottom_px) / self->keyheightpx - 1);
	*rv_keymax = psy_min(self->keymax,
		(self->keyboardheightpx - clip_top_px + self->keyheightpx) /
		self->keyheightpx);
}

typedef enum {
	PIANOROLL_TRACK_DISPLAY_ALL,
	PIANOROLL_TRACK_DISPLAY_CURRENT,
	PIANOROLL_TRACK_DISPLAY_ACTIVE
} PianoTrackDisplay;

typedef struct {
	psy_audio_Pattern* pattern;	
	psy_audio_PatternCursor cursor;
	uintptr_t lpb;
	PatternViewSkin* skin;
	int pxperbeat;
	int defaultbeatwidth;	
} PianoGridState;

void gridstate_init(PianoGridState*, PatternViewSkin* skin);

INLINE void pianogridstate_setpattern(PianoGridState* self, psy_audio_Pattern* pattern)
{
	assert(self);

	self->pattern = pattern;
}

INLINE  psy_audio_Pattern* pianogridstate_pattern(PianoGridState* self)
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

	self->pxperbeat = (int)(self->defaultbeatwidth * rate);
}

INLINE psy_dsp_big_beat_t pianogridstate_pxtobeat(const PianoGridState* self, int px)
{
	assert(self);

	return px / (psy_dsp_big_beat_t)self->pxperbeat;
}

INLINE int pianogridstate_beattopx(const PianoGridState* self, psy_dsp_big_beat_t position)
{
	assert(self);

	return (int)(position * self->pxperbeat);
}

INLINE psy_dsp_big_beat_t pianogridstate_step(const PianoGridState* self)
{
	assert(self);

	return 1 / (psy_dsp_big_beat_t)self->lpb;
}

INLINE int pianogridstate_steppx(const PianoGridState* self)
{
	assert(self);

	return pianogridstate_beattopx(self, pianogridstate_step(self));
}

INLINE intptr_t pianogridstate_steps(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	return (intptr_t)(position * self->lpb);
}

INLINE psy_dsp_big_beat_t pianogridstate_stepstobeat(PianoGridState* self,
	intptr_t steps)
{
	assert(self);

	return steps * pianogridstate_step(self);
}

INLINE psy_dsp_big_beat_t pianogridstate_quantize(const PianoGridState* self,
	psy_dsp_big_beat_t position)
{		
	assert(self);

	return pianogridstate_steps(self, position) *
		(1 / (psy_dsp_big_beat_t)self->lpb);
}

INLINE void pianogridstate_clip(PianoGridState* self,
	int clip_left_px, int clip_right_px,
	psy_dsp_big_beat_t* rv_left, psy_dsp_big_beat_t* rv_right)
{
	assert(self);
	assert(rv_left && rv_right);

	*rv_left = pianogridstate_quantize(self,
		pianogridstate_pxtobeat(self, clip_left_px));
	*rv_right = psy_min(
		psy_audio_pattern_length(pianogridstate_pattern(self)),
		pianogridstate_pxtobeat(self, clip_right_px));
}

// Header (Beatruler)
typedef struct {
	psy_ui_Component component;	
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
typedef struct {
	psy_ui_Component component;
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

typedef struct Pianogrid {
   psy_ui_Component component;
   PianoGridState* gridstate;
   PianoGridState defaultgridstate;
   KeyboardState* keyboardstate;
   KeyboardState defaultkeyboardstate;
   psy_audio_PatternEntry* hoverpatternentry;
   psy_Table lasttrackevent;
   Workspace* workspace;
   psy_dsp_big_beat_t sequenceentryoffset;
   psy_dsp_big_beat_t lastplayposition;
   psy_audio_PatternCursor oldcursor;
   bool cursoronnoterelease;
   PianoTrackDisplay trackdisplay;
   bool cursorchanging;
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

INLINE PianoTrackDisplay pianogrid_trackdisplay(const Pianogrid* self)
{
	assert(self);

	return self->trackdisplay;
}

INLINE psy_ui_Component* pianogrid_base(Pianogrid* self)
{
	assert(self);

	return &self->component;
}

typedef struct PianoBar {
	psy_ui_Component component;
	ZoomBox zoombox_keyheight;
	psy_ui_Label keys;
	psy_ui_ComboBox keytype;
	psy_ui_Label tracks;
	psy_ui_Button tracks_all;
	psy_ui_Button track_curr;
	psy_ui_Button tracks_active;
	Workspace* workspace;
} PianoBar;

void pianobar_init(PianoBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* pianobar_base(PianoBar* self)
{
	assert(self);

	return &self->component;
}

typedef struct Pianoroll {
   psy_ui_Component component;
   psy_ui_Component top;
   PianoRuler header;
   psy_ui_Component left;
   ZoomBox zoombox_beatwidth;   
   KeyboardState keyboardstate;
   PianoKeyboard keyboard;
   PianoGridState gridstate;
   Pianogrid grid;   
   uintptr_t opcount;
   bool syncpattern;
   psy_ui_Scroller scroller;
   PianoBar bar;
   Workspace* workspace;
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, PatternViewSkin*,
	Workspace*);
void pianoroll_setpattern(Pianoroll*, psy_audio_Pattern*);
void pianoroll_updatescroll(Pianoroll*);
void pianoroll_makecmds(psy_Property* parent);

INLINE psy_ui_Component* pianoroll_base(Pianoroll* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
