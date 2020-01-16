// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL)
#define PIANOROLL

#include <uicomponent.h>

#include <pattern.h>

#include "workspace.h"

typedef struct {
	int beatwidth;
	int keyheight;
	int keymin;
	int keymax;
	int lpb;
	psy_dsp_beat_t visibeats;
	int visisteps;	
	int visiwidth;	
	psy_dsp_beat_t stepwidth;
	int visikeys;
} PianoMetrics;

typedef struct {
	psy_ui_Component component;           	
	psy_dsp_beat_t bpl;
	int lpb;
	struct Pianoroll* view;   
	int scrollpos;
	PianoMetrics metrics;
} PianoHeader;

void pianoheader_init(PianoHeader*, psy_ui_Component* parent, struct Pianoroll*);

typedef struct {
	psy_ui_Component component;	
	int dy;
	int textheight;		
	struct TrackerView* view;
	PianoMetrics metrics;
} PianoKeyboard;

void pianokeyboard_init(PianoKeyboard*, psy_ui_Component* parent);

typedef struct {
   psy_ui_Component component;   
   int keyheight;   
   psy_dsp_beat_t bpl;
   int lpb;
   struct Pianoroll* view;
   int dy;
   int beatscrollpos;
   PianoMetrics metrics;
   psy_audio_PatternEntry* hover;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, struct Pianoroll*);

typedef struct Pianoroll {
   psy_ui_Component component;
   PianoHeader header;
   psy_ui_Component keyboardheader;
   PianoKeyboard keyboard;
   Pianogrid grid;
   int cx;
   int cy;
   psy_audio_Pattern* pattern;
   unsigned int opcount;
   int syncpattern;
   psy_dsp_beat_t sequenceentryoffset;
   psy_dsp_beat_t lastplayposition;
   Workspace* workspace;
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, Workspace* workspace);
void pianoroll_setpattern(Pianoroll*, psy_audio_Pattern*);

#endif
