// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEW_H)
#define PATTERNVIEW_H

// host
#include "interpolatecurveview.h"
#include "patternheader.h"
#include "patternproperties.h"
#include "patternviewmenu.h"
#include "patternviewbar.h"
#include "pianoroll.h"
#include "stepbox.h"
#include <uitabbar.h>
#include "trackerlinenumbers.h"
#include "trackerview.h"
#include "transformpatternview.h"
#include "swingfillview.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// PatternView
//
// Displays the tracker and/or pianoroll

// PatternView
//
// Editor/Viewer for a single pattern or the whole sequence.
// Composite of TrackerView and Pianoroll.

// TrackerView:      displays patternevents in a sheet
// Pianoroll:        displays patternevents as a roll


typedef struct PatternView {
	// inherits
	psy_ui_Component component;
	// internal
	// ui elements
	psy_ui_Component sectionbar;
	psy_ui_TabBar tabbar;
	psy_ui_Button contextbutton;
	TrackerLineNumberBar left;
	psy_ui_Component griddefaultspane;
	TrackerGrid griddefaults;
	psy_ui_Component headerpane;
	TrackerHeader header;
	psy_ui_Notebook notebook;
	psy_ui_Notebook editnotebook;
	psy_ui_Scroller trackerscroller;
	TrackerGrid tracker;
	Pianoroll pianoroll;	
	PatternProperties properties;
	PatternBlockMenu blockmenu;
	TransformPatternView transformpattern;
	InterpolateCurveView interpolatecurveview;
	SwingFillView swingfillview;
	// internal data
	TrackerLineState linestate;
	TrackConfig trackconfig;
	TrackerGridState gridstate;
	PatternViewSkin skin;	
	bool showdefaultline;
	PatternCursorStepMode pgupdownstepmode;
	intptr_t pgupdownstep;
	bool trackmodeswingfill;	
	int baselfheight;
	uintptr_t display;
	bool aligndisplay;
	// references
	Workspace* workspace;
} PatternView;

void patternview_init(PatternView*, psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,	Workspace*);
void patternview_setpattern(PatternView*, psy_audio_Pattern*);
void patternview_selectdisplay(PatternView*, PatternDisplayMode);
void patternview_showlinenumbers(PatternView*);
void patternview_hidelinenumbers(PatternView*);
void patternview_showbeatnumbers(PatternView*);
void patternview_hidebeatnumbers(PatternView*);
void patternview_toggleblockmenu(PatternView*);
void patternview_toggleinterpolatecurve(PatternView*, psy_ui_Component* sender);
void patternview_toggletransformpattern(PatternView*, psy_ui_Component* sender);
void patternview_toggleswingfill(PatternView*, psy_ui_Component* sender);
void patternview_oninterpolatelinear(PatternView*);
void patternview_onpatternimport(PatternView*);
void patternview_onpatternexport(PatternView*);

INLINE psy_ui_Component* patternview_base(PatternView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEW_H */
