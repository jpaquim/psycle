// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERVIEW)
#define TRACKERVIEW

// host
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
	// inherits
	psy_ui_Component component;
	// internal data	
	TrackerGridState defaultgridstate;	
	TrackerLineState defaultlinestate;
	int lpb;
	psy_dsp_big_beat_t bpl;
	psy_dsp_big_beat_t cbpl;
	psy_dsp_NotesTabMode notestabmode;   
	psy_audio_PatternCursor oldcursor;
	psy_audio_PatternSelection selection;
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
	bool ft2home;
	bool ft2delete;
	bool effcursoralwaysdown;
	bool movecursoronestep;
	// signals
	psy_Signal signal_colresize;
	// references
	TrackerGridState* gridstate;
	TrackerLineState* linestate;
	Workspace* workspace;
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
// block menu
void trackergrid_oninterpolatelinear(TrackerGrid*);
void trackergrid_onchangegenerator(TrackerGrid*);
void trackergrid_onchangeinstrument(TrackerGrid*);
void trackergrid_blockstart(TrackerGrid*);
void trackergrid_blockend(TrackerGrid*);
void trackergrid_blockunmark(TrackerGrid*);
void trackergrid_onblockcut(TrackerGrid*);
void trackergrid_onblockcopy(TrackerGrid*);
void trackergrid_onblockpaste(TrackerGrid*);
void trackergrid_onblockmixpaste(TrackerGrid*);
void trackergrid_onblockdelete(TrackerGrid*);
void trackergrid_onblocktransposeup(TrackerGrid*);
void trackergrid_onblocktransposedown(TrackerGrid*);
void trackergrid_onblocktransposeup12(TrackerGrid*);
void trackergrid_onblocktransposedown12(TrackerGrid*);

INLINE bool trackergrid_midline(TrackerGrid* self)
{
	return self->midline;
}

INLINE psy_ui_Component* trackergrid_base(TrackerGrid* self)
{
	assert(self);

	return &self->component;
}

// todo move the rest of the class to patternview and
// trackergrid
typedef struct TrackerView {
	// inherits
	psy_ui_Component component;	
	// ui elements
	TrackerGrid grid;
	psy_ui_Scroller scroller;	
	// internal data
	int showdefaultline;	
	int pgupdownstep;
	bool pgupdownbeat;
	bool pgupdown4beat;
	// references
	Workspace* workspace;
	struct PatternView* view;
} TrackerView;

void trackerview_init(TrackerView*, psy_ui_Component* parent,
	TrackerGridState*, TrackerLineState*, TrackConfig*,
	struct PatternView* view,
	Workspace*);
void trackerview_setpattern(TrackerView*, psy_audio_Pattern*);
void trackerview_updatescrollstep(TrackerView*);
void trackerview_makecmds(psy_Property* parent);
void trackerview_onpatternimport(TrackerView*);
void trackerview_onpatternexport(TrackerView*);

INLINE psy_ui_Component* trackerview_base(TrackerView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERVIEW */
