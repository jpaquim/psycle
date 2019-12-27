// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUBAR_H)
#define VUBAR_H

#include <uicomponent.h>
#include "vumeter.h"
#include "volslider.h"
#include "clipbox.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	Vumeter vumeter;
	VolSlider volslider;
	ClipBox clipbox;	
} VuBar;

void vubar_init(VuBar*, psy_ui_Component* parent, Workspace*);

#endif
