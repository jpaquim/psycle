// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songbar.h"

void songbar_init(SongBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	songtrackbar_init(&self->songtrackbar, &self->component, workspace);	
	timerbar_init(&self->timebar, &self->component, &workspace->player);	
	linesperbeatbar_init(&self->linesperbeatbar, &self->component, workspace);
	octavebar_init(&self->octavebar, &self->component, workspace);	
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(		
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
}
