// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNVIEW)
#define PATTERNVIEW

#include <uilabel.h>
#include <uicheckbox.h>
#include <uinotebook.h>
#include <uibutton.h>
#include "patternproperties.h"
#include "trackerview.h"
#include "pianoroll.h"
#include "stepbox.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;	
	Workspace* workspace;
} PatternViewStatus;

void patternviewstatus_init(PatternViewStatus*, psy_ui_Component* parent,
	Workspace*);

typedef struct {
	psy_ui_Component component;	
	StepBox step;
	psy_ui_CheckBox movecursorwhenpaste;
	psy_ui_CheckBox defaultentries;
	PatternViewStatus status;
	Workspace* workspace;
} PatternViewBar;

INLINE psy_ui_Component* patternviewbar_base(PatternViewBar* self)
{
	return &self->component;
}

void patternviewbar_init(PatternViewBar*, psy_ui_Component* parent,
	Workspace*);

typedef struct PatternView {
	psy_ui_Component component;
	psy_ui_Component sectionbar;
	TabBar tabbar;
	psy_ui_Button contextbutton;
	psy_ui_Notebook notebook;
	psy_ui_Notebook editnotebook;
	TrackerView trackerview;
	Pianoroll pianoroll;	
	PatternProperties properties;
	PatternViewSkin skin;
	Workspace* workspace;	
} PatternView;

void patternview_init(PatternView*, psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,	Workspace*);
void patternview_setpattern(PatternView*, psy_audio_Pattern*);
void patternview_selectdisplay(PatternView*, PatternDisplayType);

#endif
