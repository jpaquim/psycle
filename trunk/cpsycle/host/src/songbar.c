// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songbar.h"

void InitSongBar(SongBar* self, ui_component* parent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	InitSongTrackBar(&self->songtrackbar, &self->component, workspace);	
	InitTimeBar(&self->timebar, &self->component, &workspace->player);	
	InitLinesPerBeatBar(&self->linesperbeatbar, &self->component, &workspace->player);
	ui_component_resize(&self->linesperbeatbar.component, 130, 0);	
	InitOctaveBar(&self->octavebar, &self->component, workspace);	
	{		
		ui_margin margin = { 0, 10, 3, 3 };
		
		ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin);
	}
}