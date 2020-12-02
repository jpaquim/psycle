// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEW)
#define PATTERNVIEW

// host
#include "patternheader.h"
#include "patternproperties.h"
#include "pianoroll.h"
#include "stepbox.h"
#include "tabbar.h"
#include "trackerlinenumbers.h"
#include "trackerview.h"
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

// PatternView
//
// Displays the tracker and/or pianoroll

typedef struct PatternView {
	psy_ui_Component component;
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
	TrackerLineState linestate;
	TrackConfig trackconfig;
	TrackerGridState gridstate;
	PatternProperties properties;
	PatternViewSkin skin;
	Workspace* workspace;
	int showlinenumbers;
} PatternView;

void patternview_init(PatternView*, psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,	Workspace*);
void patternview_setpattern(PatternView*, psy_audio_Pattern*);
void patternview_selectdisplay(PatternView*, PatternDisplayType);
void patternview_showlinenumbers(PatternView*, int showstate);

#ifdef __cplusplus
}
#endif

#endif
