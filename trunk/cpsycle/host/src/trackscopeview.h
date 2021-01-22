// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKSCOPEVIEW_H)
#define TRACKSCOPEVIEW_H

// host
#include "workspace.h"
// ui
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

// TrackScopeView
//
// Displays scopes for each pattern track with the current machine buffer

typedef struct TrackScopeView {
	// inherits
	psy_ui_Component component;
	// internal data
	uintptr_t maxcolumns;
	double trackwidth;
	double trackheight;
	double textheight;	
	// references
	Workspace* workspace;	
} TrackScopeView;

void trackscopeview_init(TrackScopeView*, psy_ui_Component* parent,
	Workspace*);
void trackscopeview_start(TrackScopeView*);
void trackscopeview_stop(TrackScopeView*);

INLINE psy_ui_Component* trackscopeview_base(TrackScopeView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKSCOPEVIEW_H */
