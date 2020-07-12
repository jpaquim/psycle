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
#include "interpolatecurveview.h"
#include "patternviewskin.h"

#include <pattern.h>

// aim: The TrackerView is where you enter notes. It displays a Pattern
//      selected by the SequenceView as a tracker grid.

typedef struct {
	psy_ui_TextMetric tm;	
} TrackerMetrics;

enum {
	TRACKER_COLUMN_NONE = -1,
	TRACKER_COLUMN_NOTE = 0,
	TRACKER_COLUMN_INST = 1,
	TRACKER_COLUMN_MACH = 2,
	TRACKER_COLUMN_VOL = 3,
	TRACKER_COLUMN_CMD = 4,
	TRACKER_COLUMN_PARAM = 5,
	TRACKER_COLUMN_END = 6
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

typedef psy_audio_Inputs TrackerInputs;

typedef struct {
	PatternViewSkin* skin;
	psy_audio_Pattern* pattern;
	psy_Table trackconfigs;
	TrackDef defaulttrackdef;
	int textwidth;
	int textleftedge;
	int patterntrackident;
	int headertrackident;
	uintptr_t numtracks;
	double zoom;
} TrackerGridState;

void trackergridstate_init(TrackerGridState*);
void trackergridstate_dispose(TrackerGridState*);
int trackergridstate_trackwidth(TrackerGridState*, uintptr_t track);
TrackDef* trackergridstate_trackdef(TrackerGridState*, uintptr_t track);
int trackergridstate_track_x(TrackerGridState*, uintptr_t track);
uintptr_t trackergridstate_screentotrack(TrackerGridState*, int x,
	uintptr_t numsongtracks);

typedef struct {
	psy_ui_Component component;
	TrackerGridState* gridstate;
	TrackerGridState defaultgridstate;
	int classic;		
	Workspace* workspace;
	uintptr_t currtrack;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent,
	TrackerGridState*, Workspace*);
void trackerheader_setsharedgridstate(TrackerHeader*, TrackerGridState*);

typedef struct {
	int lineheight;
	int lpb;
	int drawcursor;
	psy_dsp_big_beat_t lastplayposition;
	psy_dsp_big_beat_t sequenceentryoffset;
	PatternViewSkin* skin;
	psy_audio_Pattern* pattern;
	// precomputed
	int visilines;
	bool cursorchanging;
} TrackerLineState;

void trackerlinestate_init(TrackerLineState*);
int trackerlinestate_offsettoscreenline(TrackerLineState*,
	psy_dsp_big_beat_t);

struct TrackerView;

typedef struct {
	psy_ui_Component component;
	TrackerLineState* linestate;
	TrackerLineState defaultlinestate;
	struct TrackerView* view;
} TrackerLineNumbersLabel;

void trackerlinenumberslabel_init(TrackerLineNumbersLabel*,
	psy_ui_Component* parent, TrackerLineState*, struct TrackerView*);

typedef struct {
	psy_ui_Component component;
	TrackerLineState* linestate;
	TrackerLineState defaultlinestate;
	Workspace* workspace;
	psy_audio_PatternEditPosition lastcursor;
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, psy_ui_Component* parent,
	TrackerLineState*, Workspace* workspace);
void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers*, TrackerLineState*);
void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const psy_audio_PatternEditPosition*);
void trackerlinenumbers_invalidateline(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset);

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
   TrackerGridState* gridstate;
   TrackerGridState defaultgridstate;
   TrackerLineState* linestate;
   TrackerLineState defaultlinestate;
   int lpb;
   psy_dsp_big_beat_t bpl;
   psy_dsp_big_beat_t cbpl;
   psy_dsp_NotesTabMode notestabmode;
   psy_audio_PatternEditPosition cursor;
   PatternSelection selection;
   psy_audio_PatternEditPosition dragselectionbase;
   psy_audio_PatternEditPosition lastdragcursor;
   int hasselection;
   int midline;
   bool chordmode;
   int chordbegin;
   int columnresize;
   int dragcolumn;
   int dragcolumnbase;
   unsigned int opcount;
   bool syncpattern;
   bool wraparound;
   bool showemptydata;
   TrackerGridEditMode editmode;
   psy_ui_TextMetric tm;
   Workspace* workspace;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, psy_ui_Component* parent,
	TrackerGridState*, TrackerLineState*, TrackerGridEditMode, Workspace*);
void trackergrid_setsharedgridstate(TrackerGrid*, TrackerGridState*);
void trackergrid_setsharedlinestate(TrackerGrid*, TrackerLineState*);
void trackergrid_setpattern(TrackerGrid*, psy_audio_Pattern*);
void trackergrid_enablesync(TrackerGrid*);
void trackergrid_preventsync(TrackerGrid*);
void trackergrid_showemptydata(TrackerGrid*, int showstate);

typedef struct TrackerView {
	psy_ui_Component component;	
	psy_ui_Component left;
	TrackerHeader header;
	TrackerLineNumbersLabel linenumberslabel;
	TrackerLineNumbers linenumbers;
	TrackerGrid griddefaults;
	TrackerGrid grid;
	PatternBlockMenu blockmenu;
	InterpolateCurveView interpolatecurveview;	
	TrackerLineState linestate;
	TrackerGridState gridstate;
	int showlinenumbers;
	int showlinenumbercursor;
	int showlinenumbersinhex;	
	int showdefaultline;
	Workspace* workspace;
	psy_List* sublines;
	psy_Table screenlines;
	TrackerMetrics metrics;	
	ZoomBox zoombox;
	bool hasnewline;
	int zoomheightbase;
	psy_ui_Component* patternview;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent,
	 psy_ui_Component* patternview, PatternViewSkin*, Workspace*);
void TrackerViewSongChanged(TrackerView*, Workspace*);
void trackerview_setpattern(TrackerView*, psy_audio_Pattern*);
void TrackerViewApplyProperties(TrackerView*, psy_Properties*);
void trackerview_showblockmenu(TrackerView*);
void trackerview_hideblockmenu(TrackerView*);
void trackerview_updatescrollstep(TrackerView*);
void trackerview_initcolumns(TrackerView*);
void trackerview_computemetrics(TrackerView*);
void trackerview_setfont(TrackerView*, psy_ui_Font*, bool iszoombase);
void trackerview_invalidateline(TrackerView*, psy_dsp_big_beat_t offset);
void trackerview_makecmds(psy_Properties* parent);

INLINE trackerview_blockmenuvisible(TrackerView* self)
{
	return psy_ui_component_visible(&self->blockmenu.component);
}

#endif
