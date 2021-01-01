// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNHEADER)
#define PATTERNHEADER

// host
#include "trackergridstate.h"
#include "workspace.h"

// TrackerHeader

// It displays the a header for each track (Tracker Numb

typedef struct TrackerHeaderTrackState {
	bool playon;
} TrackerHeaderTrackState;

typedef struct TrackerHeader {
	// inherits
	psy_ui_Component component;
	// internal data	
	TrackerGridState defaultgridstate;
	bool classic;			
	uintptr_t currtrack;
	bool playing;
	psy_Table trackstates;
	// references
	TrackerGridState* gridstate;
	Workspace* workspace;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent, TrackConfig*,
	TrackerGridState*, Workspace*);
void trackerheader_setsharedgridstate(TrackerHeader*, TrackerGridState*,
	TrackConfig*);

INLINE psy_ui_Component* trackerheader_base(TrackerHeader* self)
{
	return &self->component;
}

#endif /* PATTERNHEADER */
