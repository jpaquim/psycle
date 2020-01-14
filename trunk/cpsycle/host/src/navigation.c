// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "navigation.h"

static void navigation_onprev(Navigation*, psy_ui_Component* sender);
static void navigation_onnext(Navigation*, psy_ui_Component* sender);

void navigation_init(Navigation* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;	
	ui_component_init(&self->component, parent);	
	self->workspace = workspace;
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->prev, &self->component);	
	psy_ui_button_settextalignment(&self->prev,
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL | psy_ui_ALIGNMENT_TOP);
	ui_component_setalign(&self->prev.component, psy_ui_ALIGN_LEFT);
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	ui_component_setmargin(&self->prev.component, &margin);
	psy_ui_button_settext(&self->prev, "<");
	psy_signal_connect(&self->prev.signal_clicked, self, navigation_onprev);
	psy_ui_button_init(&self->next, &self->component);
	psy_ui_button_settext(&self->next, ">");
	psy_ui_button_settextalignment(&self->next,
		psy_ui_ALIGNMENT_CENTER_HORIZONTAL | psy_ui_ALIGNMENT_TOP);
	ui_component_setalign(&self->next.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->next.signal_clicked, self, navigation_onnext);
}

void navigation_onprev(Navigation* self, psy_ui_Component* sender)
{
	workspace_back(self->workspace);
}

void navigation_onnext(Navigation* self, psy_ui_Component* sender)
{
	workspace_forward(self->workspace);
}
