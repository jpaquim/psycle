// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGBAR_H)
#define SONGBAR_H

#include "linesperbeatbar.h"
#include "octavebar.h"
#include "timebar.h"
#include "songtrackbar.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// SongBar
//
// Groups song settings for track, bpm, lpb and octave

typedef struct SongBar {
	// inherits
	psy_ui_Component component;
	// ui elements
	SongTrackBar songtrackbar;
	TimeBar timebar;
	LinesPerBeatBar linesperbeatbar;
	OctaveBar octavebar;
} SongBar;

void songbar_init(SongBar*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* songbar_base(SongBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
