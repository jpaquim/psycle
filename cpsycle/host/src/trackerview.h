// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERVIEW)
#define TRACKERVIEW

#include "notestab.h"
#include "skincoord.h"
#include "tabbar.h"
#include "workspace.h"
#include <uibutton.h>
#include <inputmap.h>
#include <uibutton.h>
#include <uilabel.h>
#include "zoombox.h"

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
	psy_ui_Bitmap bitmap;
} TrackerSkin;

typedef struct {
	PatternEditPosition topleft;
	PatternEditPosition bottomright;
} TrackerGridBlock;

typedef TrackerGridBlock PatternSelection;

typedef struct {
	psy_ui_TextMetric tm;
	int textwidth;
	int textleftedge;
	int lineheight;	
	int patterntrackident;
	int headertrackident;
	int visilines;	
} TrackerMetrics;

typedef struct {
	psy_ui_Component component;
	int dx;	
	unsigned int numtracks;
	int classic;
	TrackerSkin* skin;
	struct TrackerView* view;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent,
	struct TrackerView* view);

typedef struct {
	psy_ui_Component component;
	struct TrackerView* view;
} TrackerLineNumbersLabel;

void InitTrackerLineNumbersLabel(TrackerLineNumbersLabel*,
	psy_ui_Component* parent, struct TrackerView*);

typedef struct {
	psy_ui_Component component;
	TrackerSkin* skin;
	int dy;
	struct TrackerView* view;
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, psy_ui_Component* parent,
	struct TrackerView*);

#define TRACKERGRID_numparametercols 10

typedef struct {
	psy_ui_Component component;
	psy_ui_Button cut;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button mixpaste;
	psy_ui_Button del;
	psy_ui_Button interpolatelinear;
	psy_ui_Button interpolatecurve;
	psy_ui_Button changegenerator;
	psy_ui_Button changeinstrument;
	psy_ui_Button blocktransposeup;
	psy_ui_Button blocktransposedown;
	psy_ui_Button blocktransposeup12;	
	psy_ui_Button blocktransposedown12;	
	psy_ui_Button import;
	psy_ui_Button export;
} PatternBlockMenu;

void patternblockmenu_init(PatternBlockMenu*, psy_ui_Component*);

enum {
	TRACKER_COLUMN_NONE  = -1,
	TRACKER_COLUMN_NOTE	 = 0,
	TRACKER_COLUMN_INST	 = 1,
	TRACKER_COLUMN_MACH	 = 2,
	TRACKER_COLUMN_VOL	 = 3,
	TRACKER_COLUMN_CMD	 = 4,
	TRACKER_COLUMN_PARAM = 5,
	TRACKER_COLUMN_END	 = 6
};

typedef struct {
	uintptr_t numdigits;
	uintptr_t numchars;	
	uintptr_t marginright;
	int wrapeditcolumn;
	int wrapclearcolumn;
	uintptr_t emptyvalue;
} TrackColumnDef;

typedef struct {
	TrackColumnDef note;
	TrackColumnDef inst;
	TrackColumnDef mach;
	TrackColumnDef vol;
	TrackColumnDef cmd;
	TrackColumnDef param;	
	int numfx;
} TrackDef;

void trackdef_init(TrackDef*);

void trackercolumndef_init(TrackColumnDef*, int numdigits, int numchars,
	int marginright, int wrapeditcol, int wrapclearcol, int emptyvalue);

typedef struct {
	int playbar;
	int cursor;
	int selection;
	int beat;
	int beat4;
	int mid;
} TrackerColumnFlags;

typedef enum {
	TRACKERGRID_EDITMODE_LOCAL,
	TRACKERGRID_EDITMODE_SONG
} TrackerGridEditMode;

typedef struct {
   psy_ui_Component component;   
   int dx;
   int dy;      
   unsigned int numtracks;   
   int lpb;
   double bpl;
   double cbpl;   
   psy_dsp_NotesTabMode notestabmode;
   PatternEditPosition cursor;
   psy_dsp_beat_t cursorstep;   
   psy_audio_Player* player;   
   struct TrackerView* view;
   PatternSelection selection;
   int hasselection;
   int midline;
   int doublemidline;
   int chordbegin;
   int columnresize;
   int dragcolumn;
   int dragcolumnbase;
   psy_audio_Pattern* pattern;
   TrackerGridEditMode editmode;
   int drawcursor;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, psy_ui_Component* parent,
	struct TrackerView*, psy_audio_Player*);

typedef Inputs TrackerInputs;

typedef struct TrackerView {
	psy_ui_Component component;	
	TrackerHeader header;
	TrackerLineNumbersLabel linenumberslabel;
	TrackerLineNumbers linenumbers;
	TrackerGrid griddefaults;
	TrackerGrid grid;
	PatternBlockMenu blockmenu;	
	TrackerSkin skin;	
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
	psy_dsp_beat_t lastplayposition;
	psy_dsp_beat_t sequenceentryoffset;
	psy_List* sublines;
	psy_Table screenlines;
	TrackerMetrics metrics;
	psy_Table trackconfigs;	
	TrackDef defaulttrackdef;
	ZoomBox zoombox;
	int doseqtick;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent, Workspace*);
void TrackerViewSongChanged(TrackerView*, Workspace*);
void trackerview_setpattern(TrackerView*, psy_audio_Pattern*);
void TrackerViewApplyProperties(TrackerView*, psy_Properties*);

#endif
