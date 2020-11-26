// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL)
#define PIANOROLL

#include "patternviewskin.h"
#include "workspace.h"
#include "zoombox.h"

#include <uiscroller.h>

#include <pattern.h>

typedef struct {
	int keymin;
	int keymax;
	psy_dsp_NotesTabMode notemode;
	bool drawpianokeys;
	psy_ui_Value keyheight;
	psy_ui_Value defaultkeyheight;
	PatternViewSkin* skin;
} KeyboardState;

void keyboardstate_init(KeyboardState*, PatternViewSkin* skin);

INLINE int keyboardstate_numkeys(KeyboardState* self)
{
	return self->keymax - self->keymin;
}

INLINE int keyboardstate_height(KeyboardState* self,
	psy_ui_TextMetric* tm)
{
	return keyboardstate_numkeys(self) * psy_ui_value_px(&self->keyheight, tm);
}

typedef struct {
	psy_audio_Pattern* pattern;
	psy_audio_PatternCursor cursor;		
	PatternViewSkin* skin;
	psy_ui_Colour eventcolour;
	psy_ui_Colour eventhovercolour;
	psy_ui_Colour eventcurrchannelcolour;		
	int pxperbeat;
	int defaultbeatwidth;
	int lpb;
	bool cursorchanging;
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

// Keyboard
typedef struct {
	psy_ui_Component component;
	KeyboardState* keyboardstate;
	KeyboardState defaultkeyboardstate;
} PianoKeyboard;

void pianokeyboard_init(PianoKeyboard*, psy_ui_Component* parent,
	KeyboardState*);
void pianokeyboard_setsharedkeyboardstate(PianoKeyboard*, KeyboardState*);

typedef struct Pianogrid {
   psy_ui_Component component;
   GridState* gridstate;
   GridState defaultgridstate;
   KeyboardState* keyboardstate;
   KeyboardState defaultkeyboardstate;
   psy_audio_PatternEntry* hover;
   psy_Table channel;
   Workspace* workspace;
   psy_dsp_big_beat_t sequenceentryoffset;
   psy_dsp_big_beat_t lastplayposition;
   psy_audio_PatternCursor oldcursor;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardState*,
	GridState*, Workspace*);
void pianogrid_setsharedgridstate(Pianogrid*, GridState*);
void pianogrid_setsharedkeyboardstate(Pianogrid*, KeyboardState*);
void pianogrid_setpattern(Pianogrid*, psy_audio_Pattern*);
void pianogrid_invalidateline(Pianogrid*, psy_dsp_big_beat_t offset);
void pianogrid_invalidatecursor(Pianogrid*);
void pianogrid_storecursor(Pianogrid*);

typedef struct Pianoroll {
   psy_ui_Component component;
   psy_ui_Component top;   
   PianoHeader header;
   psy_ui_Component left;
   ZoomBox zoombox_beatwidth;
   ZoomBox zoombox_keyheight;
   PianoKeyboard keyboard;
   Pianogrid grid;
   KeyboardState keyboardstate;
   GridState gridstate;
   int cx;
   int cy;
   psy_audio_Pattern* pattern;
   unsigned int opcount;
   bool syncpattern;
   psy_ui_Scroller scroller;
   Workspace* workspace;
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, PatternViewSkin*,
	Workspace*);
void pianoroll_setpattern(Pianoroll*, psy_audio_Pattern*);
void pianoroll_updatescroll(Pianoroll*);
void pianoroll_makecmds(psy_Property* parent);

#endif
