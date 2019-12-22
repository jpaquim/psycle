// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNPROPERTIES)
#define PATTERNPROPERTIES

#include "uilabel.h"
#include "uiedit.h"
#include "uibutton.h"
#include "pattern.h"

typedef struct {
	ui_component component;
	ui_label namelabel;
	ui_label lengthlabel;
	ui_edit nameedit;
	ui_edit lengthedit;
	ui_button applybutton;
	ui_button closebutton;
	psy_audio_Pattern* pattern;
} PatternProperties;


void InitPatternProperties(PatternProperties*, ui_component* parent, psy_audio_Pattern*);
void PatternPropertiesSetPattern(PatternProperties*, psy_audio_Pattern*);

#endif
