// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERVIEW)
#define TRACKERVIEW

#include "notestab.h"
#include "skincoord.h"
#include "tabbar.h"
#include "workspace.h"
#include <uibutton.h>
#include <inputmap.h>

#include <pattern.h>

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
	unsigned int midline;
	unsigned int midline2;
	TrackerHeaderCoords headercoords;
	ui_bitmap bitmap;
} TrackerSkin;

typedef PatternEditPosition TrackerCursor;

typedef struct {
	PatternEditPosition topleft;
	PatternEditPosition bottomright;
} TrackerGridBlock;

typedef TrackerGridBlock PatternSelection;

typedef struct {
	ui_component component;
	int dx;
	int trackwidth;
	unsigned int numtracks;
	int classic;
	TrackerSkin* skin;
	struct TrackerView* view;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, ui_component* parent,
	struct TrackerView* view);

typedef struct {
	ui_component component;
	struct TrackerView* view;
} TrackerLineNumbersLabel;

void InitTrackerLineNumbersLabel(TrackerLineNumbersLabel*, ui_component* parent, struct TrackerView*);

typedef struct {
	ui_component component;
	TrackerSkin* skin;
	int dy;
	int textheight;
	int lineheight;
	struct TrackerView* view;
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, ui_component* parent,
	struct TrackerView*);

#define TRACKERGRID_numparametercols 10

typedef struct {
	ui_component component;
	ui_button cut;
	ui_button copy;
	ui_button paste;
	ui_button mixpaste;
	ui_button del;
	ui_button changegenerator;
	ui_button changeinstrument;
	ui_button blocktransposeup;
	ui_button blocktransposedown;
	ui_button blocktransposeup12;	
	ui_button blocktransposedown12;
} PatternBlockMenu;

void patternblockmenu_init(PatternBlockMenu*, ui_component*);

typedef struct {
   ui_component component;   
   int cx;
   int cy;
   int dx;
   int dy;
   int trackwidth;
   int lineheight;
   unsigned int numtracks;   
   int lpb;
   double bpl;
   double cbpl;   
   NotesTabMode notestabmode;
   TrackerCursor cursor;
   beat_t cursorstep;   
   Player* player;
   int textheight;
   int textwidth;
   int textleftedge;
   int colx[TRACKERGRID_numparametercols];
   TrackerHeader* header;
   TrackerLineNumbers* linenumbers;
   struct TrackerView* view;
   PatternSelection selection;
   int hasselection;
   int midline;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, ui_component* parent, struct TrackerView*,
	Player*);

typedef Inputs TrackerInputs;

typedef struct {
	int line;
	int sublines;
	int accsublines;
} SublineCache;

typedef struct TrackerView {
	ui_component component;
	TrackerHeader header;
	TrackerLineNumbersLabel linenumberslabel;
	TrackerLineNumbers linenumbers;
	TrackerGrid grid;
	PatternBlockMenu blockmenu;
	Pattern* pattern;
	TrackerSkin skin;
	ui_font font;
	int showlinenumbers;
	int showlinenumbercursor;
	int showlinenumbersinhex;
	int wraparound;
	int showemptydata;
	Workspace* workspace;
	unsigned int opcount;
	TrackerInputs inputs;
	unsigned int cursorstep;
	int syncpattern;
	beat_t lastplayposition;
	beat_t sequenceentryoffset;
	List* sublines;
	Table screenlines;	
} TrackerView;

void trackerview_init(TrackerView*, ui_component* parent, Workspace*);
void TrackerViewSongChanged(TrackerView*, Workspace*);
void trackerview_setpattern(TrackerView*, Pattern*);
void TrackerViewApplyProperties(TrackerView*, Properties*);

#endif
