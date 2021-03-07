// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songbar.h"

void songbar_init(SongBar* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(songbar_base(self), parent, NULL);
	psy_ui_component_setdefaultalign(songbar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	songtrackbar_init(&self->songtrackbar, songbar_base(self), workspace);
	timebar_init(&self->timebar, songbar_base(self), workspace);
	linesperbeatbar_init(&self->linesperbeatbar, songbar_base(self),
		workspace);
	octavebar_init(&self->octavebar, songbar_base(self), workspace);
}
