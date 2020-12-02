// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERLINENUMBERS)
#define TRACKERLINENUMBERS

// host
#include "trackergridstate.h"
#include "trackerlinestate.h"
#include "workspace.h"
#include "zoombox.h"

#ifdef __cplusplus
extern "C" {
#endif

// TrackerLineNumbers
// 
// Draws the tracker linenumbers

struct TrackerView;

typedef struct {
	// inherits
	psy_ui_Component component;
	TrackerLineState* linestate;
	TrackerLineState defaultlinestate;
	struct TrackerView* view;
	char_dyn_t* linestr;
	char_dyn_t* defaultstr;
	Workspace* workspace;
} TrackerLineNumbersLabel;

void trackerlinenumberslabel_init(TrackerLineNumbersLabel*,
	psy_ui_Component* parent, TrackerLineState*, struct TrackerView*,
	Workspace*);

typedef struct {
	psy_ui_Component component;
	TrackerLineState* linestate;
	TrackerLineState defaultlinestate;
	Workspace* workspace;
	psy_audio_PatternCursor lastcursor;	
	int showlinenumbercursor;
	int showlinenumbersinhex;
} TrackerLineNumbers;

void trackerlinenumbers_init(TrackerLineNumbers*, psy_ui_Component* parent,
	TrackerLineState*, Workspace* workspace);
void trackerlinenumbers_setsharedlinestate(TrackerLineNumbers*, TrackerLineState*);
void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const psy_audio_PatternCursor*);
void trackerlinenumbers_invalidateline(TrackerLineNumbers*,
	psy_dsp_big_beat_t offset);
void trackerlinenumbers_showlinenumbercursor(TrackerLineNumbers*, int showstate);
void trackerlinenumbers_showlinenumbersinhex(TrackerLineNumbers*, int showstate);

// TrackerLineNumberBar
typedef struct TrackerLineNumberBar {
	psy_ui_Component component;
	TrackerLineNumbersLabel linenumberslabel;
	TrackerLineNumbers linenumbers;
	ZoomBox zoombox;
	int zoomheightbase;
} TrackerLineNumberBar;

void trackerlinenumberbar_init(TrackerLineNumberBar* self, psy_ui_Component*
	parent, TrackerLineState* linestate, struct TrackerView* view, Workspace* workspace);
void trackerlinenumberbar_computefontheight(TrackerLineNumberBar*);

#ifdef __cplusplus
}
#endif

#endif /* TRACKERLINENUMBERS */
