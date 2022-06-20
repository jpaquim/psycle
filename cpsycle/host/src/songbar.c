/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "songbar.h"

/* implementation */
void songbar_init(SongBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(songbar_base(self), parent, NULL);	
	psy_ui_component_set_default_align(songbar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	songtrackbar_init(&self->songtrackbar, songbar_base(self), workspace);
	timebar_init(&self->timebar, songbar_base(self),
		workspace_player(workspace));
	linesperbeatbar_init(&self->linesperbeatbar, songbar_base(self),
		workspace_player(workspace));
	octavebar_init(&self->octavebar, songbar_base(self), workspace);
}
