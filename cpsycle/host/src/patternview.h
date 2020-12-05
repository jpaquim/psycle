// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEW_H)
#define PATTERNVIEW_H

// host
#include "interpolatecurveview.h"
#include "patternheader.h"
#include "patternproperties.h"
#include "pianoroll.h"
#include "stepbox.h"
#include "tabbar.h"
#include "trackerlinenumbers.h"
#include "trackerview.h"
#include "transformpatternview.h"
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uicheckbox.h>
#include <uilabel.h>
#include <uinotebook.h>

#ifdef __cplusplus
extern "C" {
#endif

// PatternViewBar
//
// The bar displayed in the mainframe status bar, if the patternview is active

typedef struct {
	psy_ui_Component component;	
	StepBox cursorstep;
	psy_ui_CheckBox movecursorwhenpaste;
	psy_ui_CheckBox defaultentries;
	psy_ui_Label status;	
	Workspace* workspace;
} PatternViewBar;

INLINE psy_ui_Component* patternviewbar_base(PatternViewBar* self)
{
	return &self->component;
}

void patternviewbar_init(PatternViewBar*, psy_ui_Component* parent,
	Workspace*);

// PatternBlockMenu

typedef struct PatternBlockMenu {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Button cut;
	psy_ui_Button copy;
	psy_ui_Button paste;
	psy_ui_Button mixpaste;
	psy_ui_Button del;
	psy_ui_Button transform;
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

INLINE psy_ui_Component* patternblockmenu_base(PatternBlockMenu* self)
{
	assert(self);

	return &self->component;
}

// PatternView
//
// Displays the tracker and/or pianoroll

typedef struct PatternView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component sectionbar;
	TabBar tabbar;
	psy_ui_Button contextbutton;
	TrackerLineNumberBar left;
	TrackerGrid griddefaults;
	TrackerHeader header;
	psy_ui_Notebook notebook;
	psy_ui_Notebook editnotebook;	
	TrackerView trackerview;
	Pianoroll pianoroll;	
	PatternProperties properties;
	PatternBlockMenu blockmenu;
	TransformPatternView transformpattern;
	InterpolateCurveView interpolatecurveview;
	// internal data
	TrackerLineState linestate;
	TrackConfig trackconfig;
	TrackerGridState gridstate;
	PatternViewSkin skin;
	int showlinenumbers;
	// references
	Workspace* workspace;	
} PatternView;

void patternview_init(PatternView*, psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,	Workspace*);
void patternview_setpattern(PatternView*, psy_audio_Pattern*);
void patternview_selectdisplay(PatternView*, PatternDisplayType);
void patternview_showlinenumbers(PatternView*, int showstate);
void patternview_toggleblockmenu(PatternView*);
void patternview_toggleinterpolatecurve(PatternView*, psy_ui_Component* sender);
void patternview_toggletransformpattern(PatternView*, psy_ui_Component* sender);

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEW_H */
