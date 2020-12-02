// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEPBOX_H)
#define STEPBOX_H

#include "uicombobox.h"
#include "uilabel.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// aim: Select how many lines the cursor moves after a note is entered (1-16). 

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	psy_ui_ComboBox combobox;	
	Workspace* workspace;
} StepBox;

void stepbox_init(StepBox*, psy_ui_Component* parent, Workspace*);

#ifdef __cplusplus
}
#endif

#endif
