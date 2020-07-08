// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL)
#define PIANOROLL

#include "workspace.h"
#include "zoombox.h"

#include <pattern.h>

typedef struct {
	int keymin;
	int keymax;
	psy_dsp_NotesTabMode notemode;
	bool drawpianokeys;
	int keyheight;
	int defaultkeyheight;
	double zoom;
	psy_ui_Color keywhitecolour;
	psy_ui_Color keyblackcolour;
	psy_ui_Color keyseparatorcolour;	
} KeyboardState;

void keyboardstate_init(KeyboardState*);

INLINE int keyboardstate_numkeys(KeyboardState* self)
{
	return self->keymax - self->keymin;
}

INLINE void keyboardstate_setkeyheight(KeyboardState* self, int height)
{
	self->keyheight = height;
}

INLINE int keyboardstate_height(KeyboardState* self)
{
	return keyboardstate_numkeys(self) * self->keyheight;
}

typedef struct {
	int beatwidth;
	int defaultbeatwidth;
	double zoom;
	int lpb;
	psy_ui_Color eventcolour;
	psy_ui_Color eventhovercolour;
	psy_ui_Color eventcurrchannelcolour;
	psy_ui_Color grid12separatorcolour;
	psy_ui_Color gridseparatorcolour;
	psy_ui_Color row4beatcolour;
	psy_ui_Color row4beat2colour;
	psy_ui_Color rowbeatcolour;
	psy_ui_Color rowbeat2colour;
	psy_ui_Color rowcolour;
	psy_ui_Color row2colour;
	psy_ui_Color playbarcolour;
	psy_ui_Color rulerbaselinecolour;
	psy_ui_Color rulermarkcolour;
	psy_dsp_big_beat_t visibeats;
	int visisteps;	
	int visiwidth;	
	psy_dsp_big_beat_t stepwidth;
	psy_audio_PatternEditPosition cursor;
} GridState;

void gridstate_init(GridState*);

INLINE void gridstate_setbeatwidth(GridState* self, int width)
{
	//self->beatwidth = width;
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
   psy_audio_Pattern* pattern;
   Workspace* workspace;
   psy_dsp_big_beat_t sequenceentryoffset;
   psy_dsp_big_beat_t lastplayposition;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardState*,
	GridState*, Workspace*);
void pianogrid_setsharedgridstate(Pianogrid*, GridState*);
void pianogrid_setsharedkeyboardstate(Pianogrid*, KeyboardState*);
void pianogrid_setpattern(Pianogrid*, psy_audio_Pattern*);
void pianogrid_updategridstate(Pianogrid*);

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
   Workspace* workspace;  
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, Workspace*);
void pianoroll_setpattern(Pianoroll*, psy_audio_Pattern*);
void pianoroll_updatescroll(Pianoroll*);

#endif
