// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UISWITCH_H)
#define UISWITCH_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component component;
	psy_Signal signal_clicked;
} psy_ui_Switch;

void ui_switch_init(psy_ui_Switch*, psy_ui_Component* parent);

#endif
