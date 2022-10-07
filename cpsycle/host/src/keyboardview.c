/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "keyboardview.h"


/* prototypes */

/* implementation */
void keyboardview_init(KeyboardView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	assert(self);
	assert(workspace);
	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_style_type(&self->component, STYLE_SIDE_VIEW);
	keyboardstate_init(&self->state, psy_ui_HORIZONTAL, TRUE);
	pianokeyboard_init(&self->keyboard, &self->component, &self->state,
		workspace_player(workspace), NULL);		
	psy_ui_component_set_align(&self->keyboard.component, psy_ui_ALIGN_CENTER);
	psy_ui_component_set_preferred_width(&self->keyboard.component,
		psy_ui_value_make_ew((double)psy_audio_NOTECOMMANDS_RELEASE));	
}
