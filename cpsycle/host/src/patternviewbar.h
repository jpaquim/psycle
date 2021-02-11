// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNVIEWBAR_H)
#define PATTERNVIEWBAR_H

// host
#include "stepbox.h"
#include "workspace.h"
// ui
#include <uicheckbox.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// PatternViewBar
//
// The bar displayed in the mainframe status bar, if the patternview is active

typedef struct PatternViewBar {
	// inherits
	psy_ui_Component component;
	// internal
	// ui elements
	PatternCursorStepBox cursorstep;
	psy_ui_CheckBox movecursorwhenpaste;
	psy_ui_CheckBox defaultentries;
	psy_ui_CheckBox displaysinglepattern;
	psy_ui_Label status;
	// references
	Workspace* workspace;
} PatternViewBar;

void patternviewbar_init(PatternViewBar*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* patternviewbar_base(PatternViewBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PATTERNVIEWBAR_H */
