/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "navigation.h"
/* host */
#include "resources/resource.h"
#include "styles.h"

/* protoypes*/
static void navigation_onprev(Navigation*, psy_ui_Component* sender);
static void navigation_onnext(Navigation*, psy_ui_Component* sender);

/* implementation */
void navigation_init(Navigation* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);	
	self->workspace = workspace;
	psy_ui_component_set_style_type(&self->component, STYLE_NAVBAR);
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 0.5, 0.0, 0.0));	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HEXPAND);
	psy_ui_button_init_connect(&self->prev, &self->component,
		self, navigation_onprev);		
	psy_ui_button_loadresource(&self->prev, IDB_ARROW_BACK_LIGHT,
		IDB_ARROW_BACK_DARK, psy_ui_colour_white());
	psy_ui_button_init_connect(&self->next, &self->component,
		self, navigation_onnext);	
	psy_ui_button_loadresource(&self->next, IDB_ARROW_FORWARD_LIGHT,
		IDB_ARROW_FORWARD_DARK, psy_ui_colour_white());
}

void navigation_onprev(Navigation* self, psy_ui_Component* sender)
{
	workspace_back(self->workspace);
}

void navigation_onnext(Navigation* self, psy_ui_Component* sender)
{
	workspace_forward(self->workspace);
}
