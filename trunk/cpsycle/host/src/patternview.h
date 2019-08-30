// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNVIEW)
#define PATTERNVIEW

#include <uicomponent.h>
#include <pattern.h>
#include "noteinputs.h"
#include "player.h"
#include "skincoord.h"
#include "patternproperties.h"


typedef struct {
	SkinCoord background;	
	SkinCoord record;
	SkinCoord mute;
	SkinCoord solo;
	SkinCoord digitx0;
	SkinCoord digit0x;	
} PatternHeaderCoords;

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
	PatternHeaderCoords headercoords;
	ui_bitmap skinbmp;
	HFONT hfont;
} PatternSkin;

typedef struct {
	int track;
	float offset;
	int line;
	int subline;
	int totallines;
	int col;	
} PatternGridCursor;

typedef struct
{
	PatternGridCursor topleft;
	PatternGridCursor bottomright;
} PatternGridBlock;

typedef struct {
	ui_component component;
	int dx;
	int trackwidth;
	PatternSkin* skin;
} PatternHeader;

typedef struct {
	ui_component component;
} PatternLineNumbersLabel;

typedef struct {
	ui_component component;
	PatternSkin* skin;
	int dy;
	int lineheight;
	struct PatternView* view;
} PatternLineNumbers;

#define PatternGrid_NUMCOLS 10

typedef struct {
   ui_component component;   
   int cx;
   int cy;
   int dx;
   int dy;
   int trackwidth;
   int lineheight;
   int numtracks;   
   int lpb;
   float bpl;
   float cbpl;
   PatternNode* curr_event;
   NoteInputs* noteinputs;
   char** notestab;
   PatternGridCursor cursor;
   float cursorstep;   
   Player* player;
   int textwidth;
   int colx[PatternGrid_NUMCOLS];   
   PatternHeader* header;
   PatternLineNumbers* linenumbers;
   struct PatternView* view;
} PatternGrid;

typedef struct PatternView {
	ui_component component;
	PatternHeader header;
	PatternLineNumbersLabel linenumberslabel;
	PatternLineNumbers linenumbers;
	PatternGrid grid;
	PatternProperties properties;
	Pattern* pattern;
	PatternSkin skin;
} PatternView;

void InitPatternGrid(PatternGrid*, ui_component* parent, PatternView* view, Player* player);
void PatternViewApplyProperties(PatternView* self, Properties* properties);
void InitPatternHeader(PatternHeader*, ui_component* parent);
void InitPatternLineNumbersLabel(PatternLineNumbersLabel* self, ui_component* parent);
void InitPatternLineNumbers(PatternLineNumbers* self, ui_component* parent);
void InitPatternView(PatternView*, ui_component* parent, Player* player);
void PatternViewSetPattern(PatternView*, Pattern*);

#endif