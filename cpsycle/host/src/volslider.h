// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(VOLSLIDER_H)
#define VOLSLIDER_H

#include <uislider.h>
#include <machines.h>
#include "workspace.h"

typedef struct {	
	psy_ui_Component component;
	psy_audio_Machines* machines;
	float value;
	int dragx;
} VolSlider;

void volslider_init(VolSlider*, psy_ui_Component* parent, Workspace*);

#endif
