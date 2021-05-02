// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNHEADER_H)
#define PATTERNHEADER_H

// host
#include "trackergridstate.h"
#include "workspace.h"


typedef struct PatternTrackBox {
	// inherits
	psy_ui_Component component;
	uintptr_t index;
	bool playon;
	// references
	TrackerGridState* state;
} PatternTrackBox;

void patterntrackbox_init(PatternTrackBox*, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerGridState*);

PatternTrackBox* patterntrackbox_alloc(void);
PatternTrackBox* patterntrackbox_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerGridState*);

void patterntrackbox_playon(PatternTrackBox*);
void patterntrackbox_playoff(PatternTrackBox*);

INLINE psy_ui_Component* patterntrackbox_base(PatternTrackBox* self)
{
	return &self->component;
}

// TrackerHeader
//
typedef struct TrackerHeader {
	// inherits
	psy_ui_Component component;
	// internal
	uintptr_t currtrack;		
	psy_Table boxes;
	// references
	TrackerGridState* state;
	Workspace* workspace;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent, TrackConfig*,
	TrackerGridState*, Workspace*);
void trackerheader_build(TrackerHeader*);

INLINE psy_ui_Component* trackerheader_base(TrackerHeader* self)
{
	return &self->component;
}

#endif /* PATTERNHEADER_H */
