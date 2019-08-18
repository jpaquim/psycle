// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNVIEW)
#define PATTERNVIEW

#include <uicomponent.h>
#include <pattern.h>
#include "noteinputs.h"
#include "player.h"

typedef struct {
	unsigned int separator;
	unsigned int separator2;
	unsigned int background;
	unsigned int background2;
	unsigned int row4beat;
	unsigned int row4beat2;
	unsigned int rowbeat;
	unsigned int rowbeat2;
	unsigned int row;
	unsigned int row2;
	unsigned int font;
	unsigned int font2;
	unsigned int fontPlay;
	unsigned int fontPlay2;
	unsigned int fontCur;
	unsigned int fontCur2;
	unsigned int fontSel;
	unsigned int fontSel2;
	unsigned int selection;
	unsigned int selection2;
	unsigned int playbar;
	unsigned int playbar2;
	unsigned int cursor;
	unsigned int cursor2;
	HFONT hfont;
} PatternSkin;

typedef struct {
	int track;
	float offset;
	int col;
} Cursor;

typedef struct {
   ui_component component;   
   int cx;
   int cy;
   int dy;
   int trackwidth;
   int lineheight;
   int numtracks;
   int numlines;
   int lpb;
   float bpl;
   float cbpl;
   PatternNode* curr_event;
   NoteInputs* noteinputs;
   char** notestab;
   Cursor cursor;
   float cursorstep;
   Pattern* pattern;
   Player* player;
   int textwidth;
   int colx[10];
   PatternSkin skin;
} PatternView;

void InitPatternView(PatternView* patternview, ui_component* parent, Player* player);
void PatternViewApplyProperties(PatternView* self, Properties* properties);

#endif