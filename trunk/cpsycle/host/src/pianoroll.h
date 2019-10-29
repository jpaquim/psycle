// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

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
	beat_t visibeats;
	int visisteps;	
	int visiwidth;	
	beat_t stepwidth;
	int visikeys;
} PianoMetrics;

typedef struct {
	ui_component component;           	
	beat_t bpl;
	int lpb;
	struct Pianoroll* view;   
	int scrollpos;
	PianoMetrics metrics;
} PianoHeader;

void pianoheader_init(PianoHeader*, ui_component* parent, struct Pianoroll*);

typedef struct {
	ui_component component;	
	int dy;
	int textheight;		
	struct TrackerView* view;
	PianoMetrics metrics;
} PianoKeyboard;

void pianokeyboard_init(PianoKeyboard*, ui_component* parent);

typedef struct {
   ui_component component;      
   int cx;
   int cy;   
   int keyheight;   
   beat_t bpl;
   int lpb;
   struct Pianoroll* view;
   int dy;
   int beatscrollpos;
   PianoMetrics metrics;
} Pianogrid;

void pianogrid_init(Pianogrid*, ui_component* parent, struct Pianoroll*);

typedef struct Pianoroll {
   ui_component component;
   PianoHeader header;
   ui_component keyboardheader;
   PianoKeyboard keyboard;
   Pianogrid grid;
   int cx;
   int cy;
   Pattern* pattern;
   unsigned int opcount;
   int syncpattern;
   Workspace* workspace;
} Pianoroll;

void pianoroll_init(Pianoroll*, ui_component* parent, Workspace* workspace);
void pianoroll_setpattern(Pianoroll*, Pattern*);

#endif