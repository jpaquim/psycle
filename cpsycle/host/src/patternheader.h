// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PATTERNHEADER)
#define PATTERNHEADER

// host
#include "trackergridstate.h"
#include "workspace.h"

// The TrackerView is where you enter notes. It displays a Pattern selected by
// the SequenceView as a tracker grid.

typedef struct TrackerHeaderTrackState {
	bool playon;
} TrackerHeaderTrackState;

typedef struct {
	psy_ui_Component component;
	TrackerGridState* gridstate;
	TrackerGridState defaultgridstate;
	int classic;		
	Workspace* workspace;
	uintptr_t currtrack;
	bool playing;
	psy_Table trackstates;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent, TrackConfig*,
	TrackerGridState*, Workspace*);
void trackerheader_setsharedgridstate(TrackerHeader*, TrackerGridState*,
	TrackConfig*);


#endif /* PATTERNHEADER */
