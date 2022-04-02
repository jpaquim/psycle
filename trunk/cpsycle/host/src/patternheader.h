/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNHEADER_H)
#define PATTERNHEADER_H

/* host */
#include "trackergridstate.h"
#include "workspace.h"
/* ui */
#include <uilabel.h>

/* PatternTrackBox */
typedef struct PatternTrackBox {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component solo;
	psy_ui_Component mute;
	psy_ui_Component record;
	psy_ui_Component play;
	uintptr_t index;
	/* references */
	TrackerState* state;
} PatternTrackBox;

void patterntrackbox_init(PatternTrackBox*, psy_ui_Component* parent,
	uintptr_t index, TrackerState*);

PatternTrackBox* patterntrackbox_alloc(void);
PatternTrackBox* patterntrackbox_allocinit(psy_ui_Component* parent,
	uintptr_t index, TrackerState*);

void patterntrackbox_playon(PatternTrackBox*);
void patterntrackbox_playoff(PatternTrackBox*);
void patterntrackbox_update(PatternTrackBox*);

INLINE psy_ui_Component* patterntrackbox_base(PatternTrackBox* self)
{
	return &self->component;
}

/* PatternTrack */
typedef struct PatternTrack {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	PatternTrackBox trackbox;
} PatternTrack;

void patterntrack_init(PatternTrack*, psy_ui_Component* parent,
	uintptr_t index, TrackerState*);

PatternTrack* patterntrack_alloc(void);
PatternTrack* patterntrack_allocinit(psy_ui_Component* parent,
	uintptr_t index, TrackerState*);

INLINE psy_ui_Component* patterntrack_base(PatternTrack* self)
{
	return &self->component;
}
/*
** TrackerHeader
*/
typedef struct TrackerHeader {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_Table boxes;
	/* references */
	TrackerState* state;
	Workspace* workspace;
} TrackerHeader;

void trackerheader_init(TrackerHeader*, psy_ui_Component* parent, TrackConfig*,
	TrackerState*, Workspace*);
void trackerheader_build(TrackerHeader*);

INLINE psy_ui_Component* trackerheader_base(TrackerHeader* self)
{
	return &self->component;
}

typedef struct TrackerHeaderView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component pane;
	psy_ui_Label desc;
	TrackerHeader header;
} TrackerHeaderView;

void trackerheaderview_init(TrackerHeaderView*, psy_ui_Component* parent,
	TrackConfig*, TrackerState*, Workspace*);


#endif /* PATTERNHEADER_H */
