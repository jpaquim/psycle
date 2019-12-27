// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKSCOPEVIEW_H)
#define TRACKSCOPEVIEW_H

#include "workspace.h"
#include <uicomponent.h>

typedef struct {
	psy_ui_Component component;
	Workspace* workspace;
	int trackheight;
} TrackScopeView;

void trackscopeview_init(TrackScopeView*, psy_ui_Component* parent, Workspace*);
void trackscopeview_start(TrackScopeView*);
void trackscopeview_stop(TrackScopeView*);

#endif



