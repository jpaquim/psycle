// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "navigation.h"
// host
#include "resources/resource.h"

static void navigation_onprev(Navigation*, psy_ui_Component* sender);
static void navigation_onnext(Navigation*, psy_ui_Component* sender);

void navigation_init(Navigation* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_Margin margin;	
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;	
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->prev, &self->component, NULL);
	psy_ui_bitmap_loadresource(&self->prev.bitmapicon, IDB_ARROW_BACK_DARK);
	psy_ui_bitmap_settransparency(&self->prev.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_component_setalign(&self->prev.component, psy_ui_ALIGN_LEFT);
	psy_ui_margin_init_all_em(&margin, 0.0, 1.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->prev.component, &margin);
	//psy_ui_button_settext(&self->prev, "<");
	psy_signal_connect(&self->prev.signal_clicked, self, navigation_onprev);
	psy_ui_button_init(&self->next, &self->component, NULL);
	//psy_ui_button_settext(&self->next, ">");
	psy_ui_bitmap_loadresource(&self->next.bitmapicon, IDB_ARROW_FORWARD_DARK);
	psy_ui_bitmap_settransparency(&self->next.bitmapicon, psy_ui_colour_make(0x00FFFFFF));
	psy_ui_component_setalign(&self->next.component, psy_ui_ALIGN_LEFT);
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
