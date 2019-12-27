// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGBAR_H)
#define SONGBAR_H

#include "uicomponent.h"
#include "workspace.h"
#include "timebar.h"
#include "linesperbeatbar.h"
#include "octavebar.h"
#include "songtrackbar.h"

typedef struct {
	psy_ui_Component component;
	SongTrackBar songtrackbar;
	TimeBar timebar;
	LinesPerBeatBar linesperbeatbar;
	OctaveBar octavebar;
} SongBar;

void songbar_init(SongBar*, psy_ui_Component* parent, Workspace*);

#endif
