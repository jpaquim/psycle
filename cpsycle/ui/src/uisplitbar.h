// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UISPLITBAR_H)
#define UISPLITBAR_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component component;
	int resize;
} ui_splitbar;

void ui_splitbar_init(ui_splitbar*, psy_ui_Component* parent);

#endif
