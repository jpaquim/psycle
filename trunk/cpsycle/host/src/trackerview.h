// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(TRACKERVIEW)
#define TRACKERVIEW

#include <uicomponent.h>
#include <pattern.h>
#include "noteinputs.h"
#include "player.h"
#include "skincoord.h"
#include "patternproperties.h"
#include "tabbar.h"
#include "workspace.h"

typedef struct {
	SkinCoord background;	
	SkinCoord record;
	SkinCoord mute;
	SkinCoord solo;
	SkinCoord digitx0;
	SkinCoord digit0x;	
} TrackerHeaderCoords;

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
	TrackerHeaderCoords headercoords;
	ui_bitmap skinbmp;	
} TrackerSkin;

typedef struct {
	int track;
	float offset;
	int line;
	int subline;
	int totallines;
	int col;	
} TrackerGridCursor;

typedef struct
{
	TrackerGridCursor topleft;
	TrackerGridCursor bottomright;
} TrackerGridBlock;

typedef struct {
	ui_component component;
	int dx;
	int trackwidth;
	int numtracks;
	int classic;
	TrackerSkin* skin;
} TrackerHeader;

typedef struct {
	ui_component component;
	struct TrackerView* view;
} TrackerLineNumbersLabel;

typedef struct {
	ui_component component;
	TrackerSkin* skin;
	int dy;
	int textheight;
	int lineheight;
	struct TrackerView* view;
} TrackerLineNumbers;

#define TRACKERGRID_NUMCOLS 10

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
   TrackerGridCursor cursor;
   float cursorstep;   
   Player* player;
   int textheight;
   int textwidth;
   int textleftedge;
   int colx[TRACKERGRID_NUMCOLS];
   TrackerHeader* header;
   TrackerLineNumbers* linenumbers;
   struct TrackerView* view;   
} TrackerGrid;

typedef struct TrackerView {
	ui_component component;
	TrackerHeader header;
	TrackerLineNumbersLabel linenumberslabel;
	TrackerLineNumbers linenumbers;
	TrackerGrid grid;
	PatternProperties properties;
	Pattern* pattern;
	TrackerSkin skin;
	ui_font font;
	int showlinenumbers;	
} TrackerView;

void InitTrackerGrid(TrackerGrid*, ui_component* parent, TrackerView* view, Player* player);
void TrackerViewApplyProperties(TrackerView* self, Properties* properties);
void InitTrackerHeader(TrackerHeader*, ui_component* parent);
void InitTrackerLineNumbersLabel(TrackerLineNumbersLabel* self, ui_component* parent, TrackerView*);
void InitTrackerLineNumbers(TrackerLineNumbers* self, ui_component* parent);
void InitTrackerView(TrackerView*, ui_component* parent, Workspace* workspace);
void TrackerViewSetPattern(TrackerView*, Pattern*);
void TrackerViewSongChanged(TrackerView*, Workspace*);

#endif