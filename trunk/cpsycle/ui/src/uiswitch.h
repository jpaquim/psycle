// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SWITCH_H
#define psy_ui_SWITCH_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_ui_Component component;
	psy_Signal signal_clicked;
} psy_ui_Switch;

void psy_ui_switch_init(psy_ui_Switch*, psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SWITCH_H */
