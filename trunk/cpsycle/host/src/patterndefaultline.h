/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PATTERNDEFAULTLINE_H)
#define PATTERNDEFAULTLINE_H

/* host */
#include "patternviewstate.h"
#include "trackerview.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** PatternDefaultLine
*/
typedef struct PatternDefaultLine {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label desc;
	psy_ui_Component pane;
	TrackerGrid grid;
	PatternViewState pvstate_default_line;
	TrackerState state;
	/* references */
	Workspace* workspace;
	PatternViewConfig* config;
} PatternDefaultLine;

void patterndefaultline_init(PatternDefaultLine*, psy_ui_Component* parent,
	TrackConfig*, PatternViewConfig*, Workspace*);

void patterndefaultline_update_song_tracks(PatternDefaultLine*);

INLINE psy_ui_Component* patterndefaultline_base(PatternDefaultLine* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PATTERNDEFAULTLINE_H */
