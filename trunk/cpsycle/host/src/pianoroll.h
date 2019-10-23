// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PIANOROLL)
#define PIANOROLL

#include <uicomponent.h>
#include <pattern.h>
#include <uidef.h>

typedef struct {
	ui_component component;	
	int dy;
	int textheight;
	int keyheight;
	struct TrackerView* view;
} PianoKeyboard;

typedef struct {
   ui_component component;      
   int cx;
   int cy;   
   int keyheight;
   int beatwidth;
   float bpl;
   struct Pianoroll* view;
   int dy;
   int dx;
} Pianogrid;

typedef struct Pianoroll {
   ui_component component;
   PianoKeyboard keyboard;
   Pianogrid grid;
   int cx;
   int cy;  
   Pattern* pattern;
} Pianoroll;

void InitPianoroll(Pianoroll* pianoroll, ui_component* parent);
void InitPianogrid(Pianogrid* self, ui_component* parent, Pianoroll* roll);

#endif