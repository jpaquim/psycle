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
	return self->keymax - self->keymin;
}

INLINE int keyboardstate_height(KeyboardState* self,
	psy_ui_TextMetric* tm)
{
	return keyboardstate_numkeys(self) * psy_ui_value_px(&self->keyheight, tm);
}

INLINE int keyboardstate_keytopx(KeyboardState* self, intptr_t key)
{
	return (int)(self->keyboardheightpx - (key + 1) * self->keyheightpx);
}

INLINE intptr_t keyboardstate_pxtokey(KeyboardState* self, int px)
{
	return self->keymax - 1 - px / self->keyheightpx;
}

typedef enum {
	PIANOROLL_TRACK_DISPLAY_ALL,
	PIANOROLL_TRACK_DISPLAY_CURRENT,
	PIANOROLL_TRACK_DISPLAY_ACTIVE
} PianoTrackDisplay;

typedef struct {
	psy_audio_Pattern* pattern;
	psy_audio_PatternCursor cursor;		
	PatternViewSkin* skin;		
	int pxperbeat;
	int defaultbeatwidth;
	uintptr_t lpb;
	bool cursorchanging;
	PianoTrackDisplay trackdisplay;
} GridState;

void gridstate_init(GridState*, PatternViewSkin* skin);

INLINE void gridstate_setpattern(GridState* self, psy_audio_Pattern* pattern)
{
	self->pattern = pattern;
}

INLINE  psy_audio_Pattern* gridstate_pattern(GridState* self)
{
	return self->pattern;
}

INLINE psy_audio_PatternCursor gridstate_cursor(const GridState* self)
{
	return self->cursor;
}

INLINE void gridstate_setlpb(GridState* self, uintptr_t lpb)
{
	self->lpb = lpb;
}

INLINE psy_dsp_big_beat_t gridstate_pxtobeat(GridState* self, int px)
{
	return px / (psy_dsp_big_beat_t)self->pxperbeat;
}

INLINE  int gridstate_beattopx(GridState* self, psy_dsp_big_beat_t position)
{
	return (int)(position * self->pxperbeat);
}

INLINE  psy_dsp_big_beat_t gridstate_step(GridState* self)
{
	return 1 / (psy_dsp_big_beat_t)self->lpb;
}

INLINE  int gridstate_steppx(GridState* self)
{
	return gridstate_beattopx(self, gridstate_step(self));
}

INLINE intptr_t gridstate_steps(GridState* self,
	psy_dsp_big_beat_t position)
{
	return (intptr_t)(position * self->lpb);
}

INLINE psy_dsp_big_beat_t gridstate_stepstobeat(GridState* self,
	intptr_t steps)
{
	return steps * gridstate_step(self);
}

INLINE psy_dsp_big_beat_t gridstate_quantize(GridState* self,
	psy_dsp_big_beat_t position)
{		
	return gridstate_steps(self, position) *
		(1 / (psy_dsp_big_beat_t)self->lpb);
}

// Header (Beatruler)
typedef struct {
	psy_ui_Component component;	
	GridState* gridstate;
	GridState defaultgridstate;
} PianoHeader;

void pianoheader_init(PianoHeader*, psy_ui_Component* parent, GridState*);
void pianoheader_setsharedgridstate(PianoHeader*, GridState*);

INLINE psy_ui_Component* pianoheader_base(PianoHeader* self)
{
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
   GridState* gridstate;
   GridState defaultgridstate;
   KeyboardState* keyboardstate;
   KeyboardState defaultkeyboardstate;
   psy_audio_PatternEntry* hoverpatternentry;
   psy_Table lasttrackevent;
   Workspace* workspace;
   psy_dsp_big_beat_t sequenceentryoffset;
   psy_dsp_big_beat_t lastplayposition;
   psy_audio_PatternCursor oldcursor;
   bool cursoronnoterelease;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardState*,
	GridState*, Workspace*);
void pianogrid_setsharedgridstate(Pianogrid*, GridState*);
void pianogrid_setsharedkeyboardstate(Pianogrid*, KeyboardState*);
void pianogrid_invalidateline(Pianogrid*, psy_dsp_big_beat_t offset);
void pianogrid_invalidatecursor(Pianogrid*);
void pianogrid_storecursor(Pianogrid*);
void pianogrid_onpatternchange(Pianogrid*, psy_audio_Pattern*);

INLINE psy_ui_Component* pianogrid_base(Pianogrid* self)
{
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
	return &self->component;
}

typedef struct Pianoroll {
   psy_ui_Component component;
   psy_ui_Component top;
   PianoHeader header;
   psy_ui_Component left;
   ZoomBox zoombox_beatwidth;   
   KeyboardState keyboardstate;
   PianoKeyboard keyboard;
   GridState gridstate;
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
	return &self->component;
}

#endif
