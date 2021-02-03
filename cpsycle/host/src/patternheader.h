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

typedef struct TrackDraw {
	uintptr_t track;
	TrackerGridState* gridstate;
	double height;
	bool playon;
} TrackDraw;

void trackdraw_init(TrackDraw*, TrackerGridState*, uintptr_t index,
	double height, bool playon);
void trackdraw_draw(TrackDraw*, psy_ui_Graphics*, double cpx,
	psy_ui_RealPoint dest);

typedef struct TrackPlainDraw {
	uintptr_t track;
	TrackerGridState* gridstate;
	double height;
	bool playon;
	psy_ui_RealPoint zoom;
	const TrackerHeaderCoords* coords;
} TrackPlainDraw;

void trackplaindraw_init(TrackPlainDraw*, TrackerGridState*,
	psy_ui_RealPoint zoom, uintptr_t index,
	double height, bool playon);
void trackplaindraw_draw(TrackPlainDraw*, psy_ui_Graphics*, double cpx,
	psy_ui_RealPoint dest);


typedef struct TrackerHeader {
	// inherits
	psy_ui_Component component;
	// internal data	
	TrackerGridState defaultgridstate;
	bool classic;			
	uintptr_t currtrack;
	bool playing;
	bool usebitmapskin;
	psy_Table trackstates;
	// references
	TrackerGridState* gridstate;
	Workspace* workspace;
	const TrackerHeaderCoords* coords;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent, TrackConfig*,
	TrackerGridState*, Workspace*);
void trackerheader_setsharedgridstate(TrackerHeader*, TrackerGridState*,
	TrackConfig*);
void trackerheader_updatecoords(TrackerHeader*);

INLINE psy_ui_Component* trackerheader_base(TrackerHeader* self)
{
	return &self->component;
}

#endif /* PATTERNHEADER */
