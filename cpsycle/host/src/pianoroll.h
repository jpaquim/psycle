// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL)
#define PIANOROLL

#include <uicomponent.h>

#include <pattern.h>

#include "workspace.h"

typedef struct {
	int keymin;
	int keymax;
	int keyheight;
} KeyboardConfig;

typedef struct {
	int beatwidth;	
	int lpb;
	psy_dsp_big_beat_t visibeats;
	int visisteps;	
	int visiwidth;	
	psy_dsp_big_beat_t stepwidth;
	int visikeys;
} PianoMetrics;

struct Pianogrid;

void pianometrics_update(PianoMetrics* rv, struct Pianogrid* grid,
	psy_audio_Pattern* pattern, int lpb);

typedef struct {
	psy_ui_Component component;           	
	psy_dsp_big_beat_t bpl;
	int lpb;
	struct Pianoroll* view;   
	int scrollpos;
	PianoMetrics metrics;
} PianoHeader;

void pianoheader_init(PianoHeader*, psy_ui_Component* parent, struct Pianoroll*);

typedef struct {
	psy_ui_Component component;
	KeyboardConfig* keyboardconfig;
} PianoKeyboard;

void pianokeyboard_init(PianoKeyboard*, psy_ui_Component* parent,
	KeyboardConfig*);
void pianokeyboard_setkeyboardconfig(PianoKeyboard*, KeyboardConfig*);

typedef struct Pianogrid {
   psy_ui_Component component;   
   psy_dsp_big_beat_t bpl;
   int lpb;   
   int beatscrollpos;
   PianoMetrics metrics;
   psy_audio_PatternEntry* hover;
   psy_Table channel;
   KeyboardConfig* keyboardconfig;
   psy_audio_Pattern* pattern;
   Workspace* workspace;
   psy_dsp_big_beat_t sequenceentryoffset;
   psy_dsp_big_beat_t lastplayposition;
} Pianogrid;

void pianogrid_init(Pianogrid*, psy_ui_Component* parent, KeyboardConfig*,
	Workspace*);
void pianogrid_setkeyboardconfig(Pianogrid*, KeyboardConfig*);
void pianogrid_setpattern(Pianogrid*, psy_audio_Pattern*);

typedef struct Pianoroll {
   psy_ui_Component component;
   psy_ui_Component top;   
   PianoHeader header;
   psy_ui_Component left;
   psy_ui_Component keyboardheader;
   PianoKeyboard keyboard;
   Pianogrid grid;
   int cx;
   int cy;
   psy_audio_Pattern* pattern;
   unsigned int opcount;
   int syncpattern;   
   Workspace* workspace;
   KeyboardConfig keyboardconfig;
} Pianoroll;

void pianoroll_init(Pianoroll*, psy_ui_Component* parent, Workspace*);
void pianoroll_setpattern(Pianoroll*, psy_audio_Pattern*);
void pianoroll_updatemetrics(Pianoroll*);

#endif
