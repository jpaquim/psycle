// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRACKERCOLUMN_H)
#define TRACKERCOLUMN_H

// host
#include "trackergridstate.h"
#include "trackerlinestate.h"
#include "workspace.h"
// ui
#include <uicomponent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TrackerColumnFlags {
	int playbar;
	int cursor;
	int selection;
	int beat;
	int beat4;
	int mid;
	int focus;
} TrackerColumnFlags;

// TrackerColumn
typedef struct TrackerColumn {
	// inherits
	psy_ui_Component component;
	uintptr_t index;
	psy_ui_RealSize digitsize;	
	psy_ui_RealSize resizestartsize;	
	// internal			
	// references
	TrackerGridState* gridstate;
	TrackerLineState* linestate;	
	TrackDef* trackdef;
	Workspace* workspace;
} TrackerColumn;

void trackercolumn_init(TrackerColumn*, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerGridState*,
	TrackerLineState*, Workspace*);

TrackerColumn* trackercolumn_alloc(void);
TrackerColumn* trackercolumn_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerGridState* gridstate,
	TrackerLineState* linestate, Workspace* workspace);

INLINE psy_ui_Component* trackercolumn_base(TrackerColumn* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TRACKERCOLUMN_H */
