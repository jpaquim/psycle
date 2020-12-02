// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERVIEW)
#define TRACKERVIEW

// host
#include "interpolatecurveview.h"
#include "trackerlinestate.h"
#include "trackergridstate.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uiscroller.h>

#ifdef __cplusplus
extern "C" {
#endif

// The TrackerView is where you enter notes. It displays a Pattern selected by
// the SequenceView as a tracker grid.

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

void patternblockmenu_init(PatternBlockMenu*, psy_ui_Component*, Workspace*);

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
   psy_audio_PatternCursor oldcursor;
   PatternSelection selection;
   psy_audio_PatternCursor dragselectionbase;
   psy_audio_PatternCursor lastdragcursor;
   int hasselection;
   int midline;
   int chordmodestarting;
   bool chordmode;
   int chordbegin;
   int columnresize;
   uintptr_t dragcolumn;
   int dragcolumnbase;
   uintptr_t dragtrack;
   uintptr_t dragparamcol;
   unsigned int opcount;
   bool syncpattern;
   bool wraparound;
   bool showemptydata;
   TrackerGridEditMode editmode;
   Workspace* workspace;
   psy_Signal signal_colresize;
   bool ft2home;
   bool ft2delete;
   bool effcursoralwaysdown;
   bool movecursoronestep;
} TrackerGrid;

void trackergrid_init(TrackerGrid*, psy_ui_Component* parent, TrackConfig*,
	TrackerGridState*, TrackerLineState*, TrackerGridEditMode, Workspace*);
void trackergrid_setsharedgridstate(TrackerGrid*, TrackerGridState*,
	TrackConfig*);
void trackergrid_setsharedlinestate(TrackerGrid*, TrackerLineState*);
void trackergrid_setpattern(TrackerGrid*, psy_audio_Pattern*);
void trackergrid_enablesync(TrackerGrid*);
void trackergrid_preventsync(TrackerGrid*);
void trackergrid_showemptydata(TrackerGrid*, int showstate);
void trackergrid_inputnote(TrackerGrid*, psy_dsp_note_t, bool chordmode);
void trackergrid_invalidateline(TrackerGrid*, psy_dsp_big_beat_t offset);
int trackergrid_scrollup(TrackerGrid*, psy_audio_PatternCursor);
int trackergrid_scrolldown(TrackerGrid*, psy_audio_PatternCursor);
int trackergrid_scrollleft(TrackerGrid*, psy_audio_PatternCursor);
int trackergrid_scrollright(TrackerGrid*, psy_audio_PatternCursor);
void trackergrid_storecursor(TrackerGrid*);
void trackergrid_centeroncursor(TrackerGrid*);
void trackergrid_setcentermode(TrackerGrid*, int mode);

INLINE bool trackergrid_midline(TrackerGrid* self)
{
	return self->midline;
}

// todo move the rest of the class to patternview and
// trackergrid
typedef struct TrackerView {
	psy_ui_Component component;	
	TrackerGrid grid;
	psy_ui_Scroller scroller;
	PatternBlockMenu blockmenu;
	InterpolateCurveView interpolatecurveview;			
	int showdefaultline;	
	int pgupdownstep;
	bool pgupdownbeat;
	bool pgupdown4beat;
	Workspace* workspace;
	struct PatternView* view;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent,
	TrackerGridState*, TrackerLineState*, TrackConfig*,
	struct PatternView* view,
	Workspace*);
void trackerview_setpattern(TrackerView*, psy_audio_Pattern*);
void trackerview_toggleblockmenu(TrackerView*);
void trackerview_updatescrollstep(TrackerView*);
void trackerview_makecmds(psy_Property* parent);

INLINE bool trackerview_blockmenuvisible(TrackerView* self)
{
	return psy_ui_component_visible(&self->blockmenu.component);
}

#ifdef __cplusplus
}
#endif

#endif
