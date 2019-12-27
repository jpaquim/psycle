// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEPBOX_H)
#define STEPBOX_H

#include "uicombobox.h"
#include "uilabel.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	ui_combobox combobox;	
	Workspace* workspace;
} StepBox;

void stepbox_init(StepBox*, psy_ui_Component* parent, Workspace*);

#endif
