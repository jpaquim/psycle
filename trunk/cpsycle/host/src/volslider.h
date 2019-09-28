// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(VOLSLIDER_H)
#define VOLSLIDER_H

#include <uilabel.h>
#include <uislider.h>
#include <machines.h>
#include "workspace.h"

typedef struct {
	ui_component component;	
	ui_slider slider;
	Machines* machines;
	unsigned int slot;	
} VolSlider;

void InitVolSlider(VolSlider*, ui_component* parent, Workspace*);

#endif