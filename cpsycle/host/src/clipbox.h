// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(CLIPBOX_H)
#define CLIPBOX_H

#include <uicomponent.h>
#include "workspace.h"

typedef struct {
	ui_component component;	
	int clip;
} ClipBox;

void clipbox_init(ClipBox*, ui_component* parent, Workspace* workspace);

#endif