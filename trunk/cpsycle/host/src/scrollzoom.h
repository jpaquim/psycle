// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SCROLLZOOM_H)
#define SCROLLZOOM_H

#include "workspace.h"

#include <uicomponent.h>

typedef struct {
	psy_ui_Component component;
	float zoomleft;
	float zoomright;	
	int dragmode;
	int dragoffset;
	psy_Signal signal_zoom;
	psy_Signal signal_customdraw;
} ScrollZoom;

void scrollzoom_init(ScrollZoom*, psy_ui_Component* parent);

#endif
