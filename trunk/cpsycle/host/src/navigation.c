// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "navigation.h"

static void navigation_initalign(Navigation*);
static void navigation_onprev(Navigation*, ui_component* sender);
static void navigation_onnext(Navigation*, ui_component* sender);

void navigation_init(Navigation* self, ui_component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_button_init(&self->prev, &self->component);
	ui_button_settextalignment(&self->prev, UI_ALIGNMENT_CENTER_HORIZONTAL |
		UI_ALIGNMENT_TOP);
	ui_button_settext(&self->prev, "<");
	psy_signal_connect(&self->prev.signal_clicked, self, navigation_onprev);
	ui_button_init(&self->next, &self->component);
	ui_button_settext(&self->next, ">");
	ui_button_settextalignment(&self->next, UI_ALIGNMENT_CENTER_HORIZONTAL |
		UI_ALIGNMENT_TOP);
	psy_signal_connect(&self->next.signal_clicked, self, navigation_onnext);	
	navigation_initalign(self);	
}

void navigation_initalign(Navigation* self)
{		
	ui_margin margin = { 0, 5, 0, 0 };
	
	psy_list_free(ui_components_setalign(		
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));
}

void navigation_onprev(Navigation* self, ui_component* sender)
{
	workspace_back(self->workspace);
}

void navigation_onnext(Navigation* self, ui_component* sender)
{
	workspace_forward(self->workspace);
}
