// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SPLITBAR_H
#define psy_ui_SPLITBAR_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component component;
	int resize;
	int hover;
} psy_ui_SplitBar;

void ui_splitbar_init(psy_ui_SplitBar*, psy_ui_Component* parent);

#endif /* psy_ui_SPLITBAR_H */
