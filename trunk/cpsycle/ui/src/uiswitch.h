// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UISWITCH_H)
#define UISWITCH_H

#include "uicomponent.h"

typedef struct {
	ui_component component;
	Signal signal_clicked;    
} ui_switch;

void ui_switch_init(ui_switch*, ui_component* parent);

#endif
